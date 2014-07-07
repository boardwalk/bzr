#include "DatFile.hpp"

#ifdef __GNUC__
#define PACK(decl) decl __attribute__((__packed__))
#elif _MSC_VER
#define PACK(decl) __pragma(pack(push, 1)) decl __pragma(pack(pop))
#else
#error Implement PACK for this compiler.
#endif

static const size_t HEADER_OFFSET = 0x140;
static const uint32_t HEADER_MAGIC_NUMBER = 0x5442; // 'TB\0\0'
static const int MAX_NODE_COUNT = 62;

PACK(struct DatHeader
{
    uint32_t magicNumber;
    uint32_t blockSize;
    uint32_t fileSize;
    uint32_t fileVersion;
    uint32_t fileVersion2;
    uint32_t freeHead;
    uint32_t freeTail;
    uint32_t freeBlockCount;
    uint32_t rootPosition;
});

PACK(struct DatLeaf
{
    uint32_t flags;
    uint32_t id;
    uint32_t position;
    uint32_t size;
    uint32_t timestamp;
    uint32_t version;
});

PACK(struct DatNode
{
    uint32_t internalNodes[MAX_NODE_COUNT];
    uint32_t nodeCount;
    DatLeaf leafNodes[MAX_NODE_COUNT];
});

DatFile::DatFile(const string& path) :
    _fs(path.c_str(), ios_base::in|ios_base::binary)
{
    DatHeader header;

    _fs.seekg(HEADER_OFFSET);
    _fs.read((char*)&header, sizeof(header));

    if(!_fs.good())
    {
        throw DatFileError("Could not read dat file header");
    }

    if(header.magicNumber != HEADER_MAGIC_NUMBER)
    {
        throw DatFileError("Dat file header has bad magic number");
    }

    _blockSize = header.blockSize - sizeof(uint32_t); // exclude next sector position
    _rootPosition = header.rootPosition;
}

DatFile::~DatFile()
{}

vector<uint8_t> DatFile::read(uint32_t id) const
{
    auto nodePosition = _rootPosition;

    while(true)
    {
        auto nodeData = readBlocks(nodePosition);
        auto node = (DatNode*)nodeData.data();

        if(node->nodeCount > MAX_NODE_COUNT)
        {
            throw DatFileError("Node has bad node count");
        }

        for(auto i = 0u; i < node->nodeCount; i++)
        {
            auto& leafNode = node->leafNodes[i];

            if(leafNode.id > id)
            {
                if(node->internalNodes[0] == 0)
                {
                    return vector<uint8_t>();
                }

                nodePosition = node->internalNodes[i];
                break;
            }

            if(leafNode.id == id)
            {
                auto result = readBlocks(leafNode.position);
                result.resize(leafNode.size);
                return result;
            }
        }
    }
}

vector<uint8_t> DatFile::readBlocks(uint32_t position) const
{
    vector<uint8_t> result;

    while(position != 0)
    {
        result.resize(result.size() + _blockSize);

        _fs.seekg(position);
        _fs.read((char*)&position, sizeof(position));
        _fs.read((char*)result.data() + result.size() - _blockSize, _blockSize);

        if(!_fs.good())
        {
            throw DatFileError("Failed to read block");
        }
    }

    return result;
}

