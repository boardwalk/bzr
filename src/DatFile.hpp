#pragma once
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

class DatFileError : public runtime_error
{
public:
    DatFileError(const std::string& msg) : runtime_error(msg) { }
};

class DatFile
{
public:
    DatFile(const string& path);
    ~DatFile();

    vector<uint8_t> read(uint32_t id) const;

    // noncopyable
    DatFile(const DatFile&) = delete;
    DatFile& operator=(const DatFile&) = delete;

private:
    vector<uint8_t> readBlocks(uint32_t position) const;

    mutable fstream _fs;
    uint32_t _blockSize;
    uint32_t _rootPosition;
};

