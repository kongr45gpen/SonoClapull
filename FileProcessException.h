#ifndef SONOCLAPULL_FILEPROCESSEXCEPTION_H
#define SONOCLAPULL_FILEPROCESSEXCEPTION_H

#include <string>

class FileProcessException {
protected:
    std::string description;
public:
    FileProcessException(const std::string &description);
    std::string what() { return description; }
};


#endif //SONOCLAPULL_FILEPROCESSEXCEPTION_H
