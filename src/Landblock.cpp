#include "Landblock.h"

Landblock::Landblock(const void* data, size_t length)
{
   if(length != sizeof(RawData))
   {
      throw runtime_error("Bad landblock data length");
   }

   memcpy(&_data, data, sizeof(_data));
}

