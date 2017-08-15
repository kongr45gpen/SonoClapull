#include "Processing.h"

Processing::File::File(const boost::filesystem::directory_entry &directoryEntry) :
        directoryEntry(directoryEntry) {
//        toneLocator(getPath()) {
    name = directoryEntry.path().leaf().string();
    status = STATUS_QUEUED;
}

const float Processing::File::getProgress() {
    if (toneLocator.is_initialized() && (status == STATUS_SEARCHING_CLOCK || status == STATUS_DECODING_DATA)) {
        // Query & update progress
        progress = toneLocator->getMediaDecoder().getProgress();
    }

    return progress;
}

void Processing::File::start() {
    status = STATUS_DEMUXING;
    toneLocator.emplace(getPath());

    status = STATUS_SEARCHING_CLOCK;
    toneLocator->locateClock();

    status = STATUS_DECODING_DATA;
    toneLocator->getData();

    status = STATUS_DONE;
    progress = 1.0f;
}


void Processing::addFile(boost::filesystem::directory_entry &entry) {
    std::lock_guard<std::mutex> lock(file_mutex);

    // Find if the entry exists already
    // TODO: See if this needs to be faster
    for (std::shared_ptr<File> &file : files) {
        if (file->getRawPath() == entry.path()) {
            // TODO: Show a warning
            return;
        }
    }

    files.push_back(std::make_unique<File>(entry));
}

void Processing::removeFile(int index) {
    std::lock_guard<std::mutex> lock(file_mutex);

    files.erase(files.begin() + index);
}

std::vector<std::shared_ptr<Processing::File>> &Processing::getFiles() {
    std::lock_guard<std::mutex> lock(file_mutex);

    return files;
}

void Processing::setFileProcessCallback(const decltype(fileProcessCallback) &callback) {
    fileProcessCallback = callback;
}

void Processing::processFile(int index) {
    if (fileProcessCallback) {
        fileProcessCallback(files[index]);
    }
}
