#ifndef SONOCLAPULL_MEDIADECODER_H
#define SONOCLAPULL_MEDIADECODER_H

#include <string>

extern "C" {
#include <libavformat/avformat.h>
}

class MediaDecoder {
    std::string filename;
    std::string format;
    int sampleRate;

    AVFormatContext *formatContext = nullptr;

    /**
     * Get a libav error description based on its key
     * @param description A description of the error
     * @param value A value for the error
     */
    std::string avError(int key, std::string description = "", std::string value = "");
public:
    MediaDecoder(const std::string &filename);

    const std::string &getFormat() const;

    virtual ~MediaDecoder();

    int getSampleRate() const;
};


#endif //SONOCLAPULL_MEDIADECODER_H
