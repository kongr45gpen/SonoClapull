#include <iostream>
#include <thread>
#include <mutex>
#include "ProcessingThread.h"

ProcessingThread::ProcessingThread(const std::shared_ptr<Processing> &processing) : processing(processing) {
    std::cout << "init proc thread" << std::endl;
}

void ProcessingThread::operator()() {
    std::cout << "Welcome to processing thread" << std::endl;

    processing->setFileProcessCallback([this] (const std::shared_ptr<Processing::File> & file) {
        std::cout << "Callback called, pushing to queue" << std::endl;
        std::lock_guard<std::mutex>(this->queue_mutex);
        processQueue.push(file);
    });

    std::cout << "Callback set" << std::endl;

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        {
            std::lock_guard<std::mutex>(this->queue_mutex);
            if (processQueue.empty()) {
                std::cout << "Queue empty; no files found\n";
            } else {
                std::cout << "File found, commencing PROCESSING" << std::endl;
                threads.emplace_back(WorkerThread(processQueue.front()));
                processQueue.pop();
            }
        }
    }
}

ProcessingThread::WorkerThread::WorkerThread(const std::shared_ptr<Processing::File> &file) : file(file) {}

void ProcessingThread::WorkerThread::operator()() {
    std::cout << " < processing of file\n";

    file->start();

    std::cout << " < processing of file done\n";
}
