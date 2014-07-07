#include "DatFile.hpp"
#include <cstdlib>

int main()
{
    DatFile df("data/client_portal.dat");

    auto worldInfo = df.read(0x13000000);

    printf("read %lu bytes\n", worldInfo.size());

    return EXIT_SUCCESS;
}

