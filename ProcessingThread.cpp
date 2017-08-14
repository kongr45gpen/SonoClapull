#include <iostream>
#include <thread>
#include "ProcessingThread.h"

ProcessingThread::ProcessingThread(const std::shared_ptr<Processing> &processing) : processing(processing) {}

void ProcessingThread::operator()() {

}
