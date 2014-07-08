#pragma once

class Noncopyable
{
public:
    Noncopyable() {}

private:
    Noncopyable(const Noncopyable&);
    Noncopyable& operator=(const Noncopyable&);
};

