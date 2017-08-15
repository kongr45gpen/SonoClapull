#ifndef SONOCLAPULL_PROCESSING_H
#define SONOCLAPULL_PROCESSING_H

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <mutex>
#include <atomic>
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

        std::atomic<Status> status{STATUS_QUEUED};
        std::atomic<float> progress{0};
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

        const boost::filesystem::directory_entry &getDirectoryEntry() const {
            return directoryEntry;
        }

        const Status getStatus() const {
            return status;
        }

        struct MediaData {
            int sampleRate;
            std::string format;
        };

        boost::optional<MediaData> getMediaData() const {
            if (toneLocator.is_initialized()) {
                return boost::optional<MediaData>(
                        MediaData{
                                toneLocator->getMediaDecoder().getSampleRate(),
                                toneLocator->getMediaDecoder().getFormat()
                        });
            }

            return boost::none;
        }

        const float getProgress();

        void start();
    };

private:
    std::vector<std::shared_ptr<File> > files;

    /**
     * The function to call in this thread when a file is processed
     */
    std::function<void(std::shared_ptr<File> &)> fileProcessCallback;
    std::mutex file_mutex;
public:
    void addFile(boost::filesystem::directory_entry &entry);
    void removeFile(int index);
    void processFile(int index);
    std::vector<std::shared_ptr<File> > &getFiles();

    void setFileProcessCallback(const decltype(fileProcessCallback) & callback);
};


#endif //SONOCLAPULL_PROCESSING_H
