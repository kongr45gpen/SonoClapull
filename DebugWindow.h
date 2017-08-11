#ifndef SONOCLAPULL_DEBUGWINDOW_H
#define SONOCLAPULL_DEBUGWINDOW_H

#include <string>
#include <memory>
#include "MediaDecoder.h"

#define PATH_MAX 4097

class DebugWindow {
    char location[PATH_MAX] = "~/Music/sine.wav";

    std::string openError;

    bool dataExists = false;
    std::shared_ptr<MediaDecoder> mediaDecoder;
    std::shared_ptr<std::vector<float> > data;
    void analyse();

    //float * data;
    float fftdata[1024];

    class Exception {
        std::string what;
    public:
        Exception(const std::string &what);

        const std::string &getWhat() const;
    };
public:
    DebugWindow();
    void draw();
};


#endif //SONOCLAPULL_DEBUGWINDOW_H
