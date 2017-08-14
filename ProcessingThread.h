#ifndef SONOCLAPULL_PROCESSINGTHREAD_H
#define SONOCLAPULL_PROCESSINGTHREAD_H

#include <mutex>
#include "Processing.h"

class ProcessingThread {
    std::shared_ptr<Processing> processing;
public:
    ProcessingThread(const std::shared_ptr<Processing> &processing);
    void operator()();
};


#endif //SONOCLAPULL_PROCESSINGTHREAD_H
