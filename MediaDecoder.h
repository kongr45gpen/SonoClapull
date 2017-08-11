#ifndef SONOCLAPULL_MEDIADECODER_H
#define SONOCLAPULL_MEDIADECODER_H

#include <string>
#include <vector>
#include <memory>

extern "C" {
#include <libavformat/avformat.h>
}

class MediaDecoder {
    /**
     * The amount of samples to return every time the getNextSamples() function is called
     */
    int samples;
    std::vector<float> packetData;
    std::shared_ptr<std::vector<float> > data_p;
    int packetSamples;

    std::vector<float> oldData; // old data stored to account for overlapping windows
    std::string filename;
    std::string format;

    int sampleRate;
    AVFormatContext *formatContext = nullptr;
    AVCodecContext *codecContext = nullptr;
    AVFrame *frame = nullptr;
    std::vector<float> floatFrame;
    AVPacket packet;

    int framesProcessed = 0;
    /**
     * The last processed frame number
     */
    int processedFrameEnd = -1;

    /**
     * Get a libav error description based on its key
     * @param description A description of the error
     * @param value A value for the error
     */
    std::string avError(int key, std::string description = "", std::string value = "");
    bool readFrame();
    int decodePacket();
    void convertToFloat();
public:
    MediaDecoder(const std::string &filename, int samples = 1024);
    virtual ~MediaDecoder();

    const std::string &getFormat() const;
    int getSampleRate() const;

    std::shared_ptr<std::vector<float> > getNextSamples();
};


#endif //SONOCLAPULL_MEDIADECODER_H
