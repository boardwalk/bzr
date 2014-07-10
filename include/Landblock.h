#ifndef BZR_LANDBLOCK_H
#define BZR_LANDBLOCK_H

#include "Noncopyable.h"

class Landblock : Noncopyable
{
public:
   Landblock(const void* data, size_t length);

private:
   PACK(struct RawData
   {
      uint32_t fileid;
      uint32_t flags;
      uint16_t style[9][9];
      uint8_t height[9][9];
      uint8_t pad;
   });

   RawData _data;
};

#endif
