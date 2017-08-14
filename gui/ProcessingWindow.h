#ifndef SONOCLAPULL_PROCESSINGWINDOW_H
#define SONOCLAPULL_PROCESSINGWINDOW_H

#include "../Processing.h"

class ProcessingWindow {
    std::shared_ptr<Processing> processing;

    inline void showStatus(const Processing::File::Status & status);
    inline ImVec4 statusColour(const Processing::File::Status & status);
public:
    ProcessingWindow(const std::shared_ptr<Processing> &processing);

    void draw();
};


#endif //SONOCLAPULL_PROCESSINGWINDOW_H
