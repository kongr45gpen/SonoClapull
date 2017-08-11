#include <sstream>
#include <iostream>
#include "MediaDecoder.h"
#include "FileProcessException.h"

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

//    frame = av_frame_alloc();
//    floatFrame = av_frame_alloc();
//    if (frame == nullptr) {
//        throw FileProcessException(avError(ENOMEM, "Could not allocate frame"));
//    }

    // initialize packet, set data to NULL, let the demuxer fill it
    av_init_packet(&packet);
    packet.data = nullptr;
    packet.size = 0;
}

MediaDecoder::~MediaDecoder() {
    // See if it's OK to close this earlier
    avformat_close_input(&formatContext);
    av_frame_free(&frame);
//    av_frame_free(&floatFrame);
}

std::shared_ptr<std::vector<float> > MediaDecoder::getNextSamples() {
    // Initialise the return array
    auto returnSamples = std::make_shared<std::vector<float> >(samples);
    int start = 0;

    // A few samples from the previous frame were left unprocessed; return them now
    if (processedFrameEnd >= 0) {
        int end = std::min(packetSamples, processedFrameEnd + samples);
        std::cout << "processing leftover samples " << processedFrameEnd << "~" << end << std::endl;
        std::copy(packetData.begin() + processedFrameEnd, packetData.begin() + end, returnSamples->begin());

        start = processedFrameEnd - end;
        processedFrameEnd = -1;
    }

    while (start < samples) {
        if (!readFrame()) {
            return nullptr;
        };

        unsigned long gotSamples = packetData.size();

//        std::cout << "Read frame #" << framesProcessed;
//        std::cout << " , got " << gotSamples << " samples ";
//        std::cout << "  " << gotSamples << "/" << samples-start << " rem. [total=" << samples << "]";
//        std::cout << std::endl;
        if (gotSamples > samples - start) {
            processedFrameEnd = samples - start;
            gotSamples = (unsigned long) samples - start;

            std::cout << "Too many samples! Processing " << 0 << "~" << processedFrameEnd << std::endl;
        }

        // Fill the return array with the samples contained in this frame
        for (int i = 0; i < gotSamples; i++) {
            (*returnSamples)[i+start] = packetData[i];
        }
        start += gotSamples;
    }

    std::cout << "first samples: " << returnSamples->at(0) << ", " << returnSamples->at(1) << "\n";
    std::cout << "last  samples: " << returnSamples->at(samples-4) << ", " << returnSamples->at(samples-3) << ", " << returnSamples->at(samples-2) << ", " << returnSamples->at(samples-1) << "\n";

    return returnSamples;
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

    // TODO: Don't free the first frame
    av_frame_free(&frame);
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
//    int err;
//    err = av_frame_make_writable(floatFrame);
//    if (err < 0) {
//        throw FileProcessException(avError(err, "Failed to make frame writeable"));
//    }
//    av_frame_free(&floatFrame);
//    floatFrame = av_frame_alloc();
//    av_frame_set_channels(floatFrame, 1);
//    av_frame_set_channel_layout(floatFrame, AV_CH_LAYOUT_MONO);
//    av_frame_set_sample_rate(floatFrame, frame->sample_rate);
//    floatFrame->format = AV_SAMPLE_FMT_FLT; // is this right??

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
