#ifndef SONOCLAPULL_LOADWINDOW_H
#define SONOCLAPULL_LOADWINDOW_H

#include <climits>
#include "../Processing.h"

class LoadWindow {
    static constexpr int EXT_MAX = 128;

    std::shared_ptr<Processing> processing;

    /**
     * The amount of files loaded the last time the "Load" button
     * was pressed
     */
    int lastLoad = -1;

    char path[PATH_MAX] = "~/Music";
    char extension[EXT_MAX] = "";
    bool recurse = false;

    void performLoad();
public:
    explicit LoadWindow(const std::shared_ptr<Processing> &processing);

    void draw();
};


#endif //SONOCLAPULL_LOADWINDOW_H
