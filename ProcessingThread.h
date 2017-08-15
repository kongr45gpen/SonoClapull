#ifndef SONOCLAPULL_PROCESSINGTHREAD_H
#define SONOCLAPULL_PROCESSINGTHREAD_H

#include <mutex>
#include <thread>
#include <queue>
#include "Processing.h"

class ProcessingThread {
    std::shared_ptr<Processing> processing;

    std::queue<std::shared_ptr<Processing::File>> processQueue;
    std::list<std::thread> threads;

    std::mutex queue_mutex;

    class WorkerThread {
        std::shared_ptr<Processing::File> file;

    public:
        WorkerThread(const std::shared_ptr<Processing::File> &file);
        void operator()();
    };
public:
    ProcessingThread(const std::shared_ptr<Processing> &processing);
    void operator()();
};


#endif //SONOCLAPULL_PROCESSINGTHREAD_H
