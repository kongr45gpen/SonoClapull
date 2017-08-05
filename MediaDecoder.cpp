#include <sstream>
#include "MediaDecoder.h"
#include "FileProcessException.h"

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
}

MediaDecoder::MediaDecoder(const std::string &filename) : filename(filename) {
    int err;

    err = avformat_open_input(&formatContext, filename.c_str(), nullptr, nullptr);
    if (err < 0) {
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
        throw FileProcessException(avError(err, "Failed to open codec ", decoder->long_name));
    }

    format = decoder->long_name;
    sampleRate = stream->codec->sample_rate;
}

MediaDecoder::~MediaDecoder() {
    // See if it's OK to close this earlier
    avformat_close_input(&formatContext);
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
