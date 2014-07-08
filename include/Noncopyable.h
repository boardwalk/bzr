#ifndef BZR_NONCOPYABLE_H
#define BZR_NONCOPYABLE_H

class Noncopyable
{
public:
    Noncopyable() {}

private:
    Noncopyable(const Noncopyable&);
    Noncopyable& operator=(const Noncopyable&);
};

#endif
