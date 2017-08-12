#ifndef SONOCLAPULL_FILEENDEXCEPTION_H
#define SONOCLAPULL_FILEENDEXCEPTION_H

#include "FileProcessException.h"

class FileEndException : public FileProcessException {
public:
    FileEndException(const std::string &description);
};

#endif
