#ifndef BZR_BLOBREADER_H
#define BZR_BLOBREADER_H

class BlobReader
{
public:
    BlobReader(const void* data, size_t size) : _data(data), _size(size), _position(0)
    {}

    template<class T>
    T read()
    {
        assertRemaining(sizeof(T));

        auto result = *(const T*)((const uint8_t*)_data + _position);

        _position += sizeof(T);

        return result;
    }

    uint16_t readVarInt()
    {
        uint16_t val = read<uint8_t>();

        if(val & 0x80)
        {
            val = (val & 0x7F) << 8 | read<uint8_t>();
        }

        return val;
    }

    template<class T>
    const T* readPointer(size_t count = 1)
    {
        assertRemaining(sizeof(T) * count);

        auto result = (const T*)((const uint8_t*)_data + _position);

        _position += sizeof(T) * count;

        return result;
    }

    void assertEnd() const
    {
        if(_position < _size)
        {
           throw runtime_error("Expected end of blob");
        }
    }

private:
    void assertRemaining(size_t numBytes) const
    {
        if(_position + numBytes > _size)
        {
            throw runtime_error("Read overrun in blob");
        }
    }

    const void* _data;
    size_t _size;
    size_t _position;
};

#endif
