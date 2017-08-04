#ifndef SONOCLAPULL_DEBUGWINDOW_H
#define SONOCLAPULL_DEBUGWINDOW_H

#include <string>
#define PATH_MAX 4097

class DebugWindow {
    char location[PATH_MAX] = "~/SonoClapull/sine.wav";

    std::string openError;

    bool dataExists = false;
    void analyse();

    float data[4096];
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
