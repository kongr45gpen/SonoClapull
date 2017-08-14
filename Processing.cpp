#include "Processing.h"

Processing::File::File(const boost::filesystem::directory_entry &directoryEntry) :
        directoryEntry(directoryEntry) {
//        toneLocator(getPath()) {
    name = directoryEntry.path().leaf().string();
}


void Processing::addFile(boost::filesystem::directory_entry &entry) {
    // Find if the entry exists already
    // TODO: See if this needs to be faster
    for (File &file : files) {
        if (file.getRawPath() == entry.path()) {
            // TODO: Show a warning
            return;
        }
    }

    files.emplace_back(entry);
}