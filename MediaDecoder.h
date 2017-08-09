#ifndef SONOCLAPULL_MEDIADECODER_H
#define SONOCLAPULL_MEDIADECODER_H

#include <string>
#include <vector>

extern "C" {
#include <libavformat/avformat.h>
}

class MediaDecoder {
    std::string filename;
    std::string format;
    int sampleRate;

    AVFormatContext *formatContext = nullptr;
    AVCodecContext *codecContext = nullptr;
    AVFrame *frame = nullptr;
    AVFrame *floatFrame = nullptr;
    AVPacket packet;

    /**
     * Get a libav error description based on its key
     * @param description A description of the error
     * @param value A value for the error
     */
    std::string avError(int key, std::string description = "", std::string value = "");
    bool readFrame();
    int decodePacket();
    void convertToFloat(int samples, AVSampleFormat format, int64_t channelLayout, int sampleRate);
public:
    MediaDecoder(const std::string &filename);
    virtual ~MediaDecoder();

    const std::string &getFormat() const;

    int getSampleRate() const;
};


#endif //SONOCLAPULL_MEDIADECODER_H
