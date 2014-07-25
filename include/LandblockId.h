#ifndef BZR_LANDBLOCKID_H
#define BZR_LANDBLOCKID_H

class LandblockId
{
public:
    LandblockId();
    LandblockId(uint8_t x, uint8_t y);

    int x() const;
    int y() const;
    uint32_t fileId() const;

    int calcSquareDistance(LandblockId other) const;

    bool operator==(LandblockId other) const;
    bool operator!=(LandblockId other) const;

private:
    uint8_t _x;
    uint8_t _y;
};

namespace std
{
    template<>
    struct hash<LandblockId>
    {
        size_t operator()(LandblockId id) const
        {
            return id.fileId();
        }
    };
}

#endif
