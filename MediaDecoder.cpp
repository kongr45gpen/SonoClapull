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

MediaDecoder::MediaDecoder(const std::string &filename) : filename(filename) {
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

    err = avcodec_open2(stream->codec, decoder, &options);
    if (err < 0) {
        throw FileProcessException(avError(err, "Failed to open codec", decoder->long_name));
    }

    codecContext = stream->codec;
    format = decoder->long_name;
    sampleRate = stream->codec->sample_rate;

    frame = av_frame_alloc();
    floatFrame = av_frame_alloc();
    if (frame == nullptr || floatFrame == nullptr) {
        throw FileProcessException(avError(ENOMEM, "Could not allocate frame"));
    }

    // initialize packet, set data to NULL, let the demuxer fill it
    av_init_packet(&packet);
    packet.data = nullptr;
    packet.size = 0;

    while(readFrame()) { };
//    readFrame();
}

MediaDecoder::~MediaDecoder() {
    // See if it's OK to close this earlier
    avformat_close_input(&formatContext);
    av_frame_free(&frame);
    av_frame_free(&floatFrame);
}

bool MediaDecoder::readFrame() {
    if (av_read_frame(formatContext, &packet) >= 0) {
        AVPacket originalPacket = packet;
        do {
            int err = decodePacket();
            if (err < 0) {
                break;
            }
            packet.data += err;
            packet.size -= err;
        } while (packet.size > 0);
        av_packet_unref(&originalPacket);

        return true;
    }

    return false;
}

int MediaDecoder::decodePacket() {
    int err = 0;
    int decoded = packet.size;
    int gotFrame = 0;

    err = avcodec_decode_audio4(codecContext, frame, &gotFrame, &packet);
    if (err < 0) {
        throw FileProcessException(avError(err, "Error decoding audio frame"));
    }

    decoded = FFMIN(err, packet.size);
    if (gotFrame) {
        long bytesPerSample = av_get_bytes_per_sample((AVSampleFormat) frame->format);
        long unpaddedLinesize = frame->nb_samples * bytesPerSample;

//        frame->buf;
        std::cout << av_get_sample_fmt_name((AVSampleFormat) frame->format);

        std::cout << "channels: " << av_get_default_channel_layout(frame->channels) << std::endl;

        convertToFloat(5, AV_SAMPLE_FMT_FLT, frame->channel_layout, frame->sample_rate);
        auto data = reinterpret_cast<float *>(floatFrame->extended_data[0]);

//        av_get_sample_fl

        for (int i = 0 ; i < floatFrame->nb_samples ; i += 1) {
            std::cout << data[i] << std::endl;
        }

        std::cout << " bytes per sample: " << bytesPerSample << "\t linesize: " << unpaddedLinesize
                                                                               << "\n";
    }

    return decoded;
}

void MediaDecoder::convertToFloat(int samples, AVSampleFormat format, int64_t channelLayout, int sampleRate)
{
//    int err;
//    err = av_frame_make_writable(floatFrame);
//    if (err < 0) {
//        throw FileProcessException(avError(err, "Failed to make frame writeable"));
//    }

    av_frame_set_channels(floatFrame, 1);
    av_frame_set_channel_layout(floatFrame, AV_CH_LAYOUT_MONO);
    av_frame_set_sample_rate(floatFrame, frame->sample_rate);
    floatFrame->format = AV_SAMPLE_FMT_FLT; // is this right??

    if (!frame->channel_layout) {
        frame->channel_layout = av_get_default_channel_layout(frame->channels);
    }

    SwrContext *swr = swr_alloc();
    swr_config_frame(swr, floatFrame, frame);
    swr_convert_frame(swr, floatFrame, frame);

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
