#ifndef BZR_BLOBREADER_H
#define BZR_BLOBREADER_H

class BlobReader
{
public:
   BlobReader(const void* data, size_t size) : _data(data), _size(size), _position(0)
   {}

   template<class T>
   const T* readPointer(size_t count = 1)
   {
      if(_position + sizeof(T) * count > _size)
      {
         throw runtime_error("Read overrun in blob");
      }

      auto result = (const T*)((const uint8_t*)_data + _position);

      _position += sizeof(T) * count;

      return result;
   }

   void assertEnd()
   {
       if(_position < _size)
       {
           throw runtime_error("Expected end of blob");
       }
   }

private:
   const void* _data;
   size_t _size;
   size_t _position;
};

#endif
