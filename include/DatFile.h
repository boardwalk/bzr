#ifndef BZR_DATFILE_H
#define BZR_DATFILE_H

#include "Noncopyable.h"
#include <fstream>
#include <string>
#include <vector>

class DatFile : Noncopyable
{
public:
    DatFile(const string& path);
    ~DatFile();

    vector<uint8_t> read(uint32_t id) const;

private:
    vector<uint8_t> readBlocks(uint32_t position) const;

    mutable fstream _fs;
    uint32_t _blockSize;
    uint32_t _rootPosition;
};

#endif
