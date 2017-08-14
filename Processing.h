#ifndef SONOCLAPULL_PROCESSING_H
#define SONOCLAPULL_PROCESSING_H

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include "ToneLocator.h"

class Processing {
public:
    class File {
    public:
        enum Status {
            STATUS_QUEUED,
            STATUS_WAITING,
            STATUS_DEMUXING,
            STATUS_SEARCHING_CLOCK,
            STATUS_DECODING_DATA,
            STATUS_DONE,
            STATUS_ERROR
        };
    private:
        boost::filesystem::directory_entry directoryEntry;
        std::string name;

        boost::optional<ToneLocator> toneLocator;

        Status status = STATUS_QUEUED;
        float progress = 0;
    public:
        explicit File(const boost::filesystem::directory_entry &directoryEntry);

        const std::string &getName() const {
            return name;
        }
        const boost::filesystem::path &getRawPath() const {
            return directoryEntry.path();
        }
        const std::string &getPath() const {
            return directoryEntry.path().string();
        }

        const Status getStatus() const {
            return status;
        }

        const float getProgress() const {
            if (toneLocator.is_initialized()) {
                // Query & update progress
            }

            return progress;
        }
    };

private:
    std::vector<File> files;

public:
    void addFile(boost::filesystem::directory_entry & entry);

    std::vector<File> &getFiles() {
        return files;
    }
};


#endif //SONOCLAPULL_PROCESSING_H
