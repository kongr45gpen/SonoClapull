#include <sstream>
#include <iostream>
#include "MediaDecoder.h"
#include "FileProcessException.h"
#include "FileEndException.h"

extern "C" {
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

MediaDecoder::MediaDecoder(const std::string &filename, int samples) : filename(filename), samples(samples) {
    int err;

    err = avformat_open_input(&formatContext, filename.c_str(), nullptr, nullptr);
    if (err < 0 || formatContext == nullptr) {
        throw FileProcessException(avError(err, "Could not open file", filename));
    }

    err = avformat_find_stream_info(formatContext, nullptr);
    if (err < 0) {
        throw FileProcessException(avError(err, "Could not find stream information"));
    }

    int streamIndex;
    //
    // Open Codec context
    //
    AVStream *stream;
    AVCodec *decoder = nullptr;
    AVDictionary *options = nullptr;

    err = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (err < 0) {
        throw FileProcessException(avError(err, "Could not find audio stream in file"));
    }

    streamIndex = err;
    stream = formatContext->streams[streamIndex];
    decoder = avcodec_find_decoder(stream->codec->codec_id);
    if (decoder == nullptr) {
        throw FileProcessException(avError(err, "This file format is unsupported"));
    }

    av_dict_set(&options, "refcounted_frames", "0", 0);
    err = avcodec_open2(stream->codec, decoder, &options);
    if (err < 0) {
        throw FileProcessException(avError(err, "Failed to open codec", decoder->long_name));
    }

    codecContext = stream->codec;
    format = decoder->long_name;
    sampleRate = stream->codec->sample_rate;

    // initialize packet, set data to NULL, let the demuxer fill it
    av_init_packet(&packet);
    packet.data = nullptr;
    packet.size = 0;
}

MediaDecoder::~MediaDecoder() {
    // See if it's OK to close this earlier
    avformat_close_input(&formatContext);
    av_frame_free(&frame);
}

std::shared_ptr<std::vector<float> > MediaDecoder::getNextSamples() {
    // Initialise the return array
    auto returnSamples = std::make_shared<std::vector<float> >(samples);

    int overlapSamples = 0;
    if (oldData) {
        overlapSamples = std::min((int) floor(overlap * samples), (int) oldData->size());

        std::cout << "Overlap: " << oldData->size() - overlapSamples << "~" << oldData->size() << std::endl;

        // Copy the last N samples from the old data
        // into the first N samples of the new data
        // (N = overlapSamples)
        std::copy(
                oldData->end() - overlapSamples,
                oldData->end(),
                returnSamples->begin()
        );
    }

    addNewSamples(returnSamples->begin() + overlapSamples, samples - overlapSamples);

    oldData = returnSamples;

    return returnSamples;
}

void MediaDecoder::addNewSamples(std::vector<float>::iterator begin, int remainingSamples) {
    int returnStart = 0;

    while (remainingSamples > 0) {
        // Get a new frame if needed
        if (processedFrameStart >= processedFrameEnd) {
            if (!readFrame()) {
                throw FileEndException("End of file reached");
            }

            processedFrameStart = 0;
            processedFrameEnd = packetData.size();
        }

        // Copy the data to the return array
        int end = std::min((int) packetData.size(), processedFrameStart + remainingSamples);
        std::cout << "Copying sample pool " << processedFrameStart << " ~ " << end << std::endl;
        std::copy(packetData.begin() + processedFrameStart,
                  packetData.begin() + end,
                  begin + returnStart);

        // Update the state variables
        returnStart += (end - processedFrameStart);
        remainingSamples -= (end - processedFrameStart);
        processedFrameStart = end;
    }
}

bool MediaDecoder::readFrame() {
    // Get one packet from the source file
    packetSamples = 0;
    if (av_read_frame(formatContext, &packet) >= 0) {
        AVPacket originalPacket = packet;
        unsigned long samplesFilled = 0;

        if (packet.dts != packet.pts) {
            throw FileProcessException("Different decoded and actual packet timestamp!");
        }

        do {
            // Decode every frame of the packet
            int err = decodePacket();
            if (err < 0) {
                break;
            }
            packet.data += err;
            packet.size -= err;

            // Store packet data
//            if (packetData.size() < samplesFilled + floatFrame.size()) {
                // Increase size of array if needed
                packetData.resize(samplesFilled + floatFrame.size());
//            }
            packetSamples += floatFrame.size();
            std::copy(
                    floatFrame.begin(),
                    floatFrame.end(),
                    packetData.begin() + samplesFilled
            );
            samplesFilled += floatFrame.size();
        } while (packet.size > 0);
        packet = originalPacket;

        packetData.shrink_to_fit(); // Reduce memory consumption if needed

        return true;
    }

    return false;
}

int MediaDecoder::decodePacket() {
    int err = 0;
    int decoded = packet.size;
    int gotFrame = 0;

    if (frame != nullptr) {
        av_frame_free(&frame);
    }
    frame = av_frame_alloc();

    err = avcodec_decode_audio4(codecContext, frame, &gotFrame, &packet);
    if (err < 0) {
        // TODO: Log error
//        throw FileProcessException(avError(err, "Error decoding audio frame"));
    }

    decoded = FFMIN(err, packet.size);
    if ((bool) gotFrame) {
        convertToFloat();

        av_frame_unref(frame);
    } else {
        floatFrame.clear();
    }

    framesProcessed++;
    return decoded;
}

void MediaDecoder::convertToFloat()
{
    if (!frame->channel_layout) {
        frame->channel_layout = av_get_default_channel_layout(frame->channels);
    }

    SwrContext * swr = swr_alloc();
    if (swr == nullptr) {
        throw FileProcessException(avError(ENOMEM, "Could not allocate resampler context"));
    }

    av_opt_set_int(swr, "in_channel_layout",    frame->channel_layout, 0);
    av_opt_set_int(swr, "in_sample_rate",       frame->sample_rate, 0);
    av_opt_set_sample_fmt(swr, "in_sample_fmt", (AVSampleFormat) frame->format, 0);

    av_opt_set_int(swr, "out_channel_layout",    AV_CH_LAYOUT_MONO, 0);
    av_opt_set_int(swr, "out_sample_rate",       frame->sample_rate, 0);
    av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_FLT, 0);

    swr_init(swr);

    floatFrame.resize(frame->nb_samples);
    uint8_t * storage[1] = {reinterpret_cast<uint8_t *>(floatFrame.data()) };
    swr_convert(swr, storage, frame->nb_samples, const_cast<const uint8_t * *>(frame->extended_data), frame->nb_samples);

    swr_free(&swr);
}

std::string MediaDecoder::avError(int key, std::string description, std::string value) {
    char errBuffer[512];
    int ret = av_strerror(key, errBuffer, 512);

    std::ostringstream ss;

    if (!description.empty()) {
        ss << description;
    }
    if (!value.empty()) {
        ss << " " << value;
    }
    if (!description.empty()) {
        ss << ": ";
    }

    if (ret >= 0) {
        ss << errBuffer;
    } else {
        ss << "Unknown error";
    }

    return ss.str();
}

const std::string &MediaDecoder::getFormat() const {
    return format;
}

int MediaDecoder::getSampleRate() const {
    return sampleRate;
}
