#ifndef BZR_BASIC_H
#define BZR_BASIC_H

#include <memory>

using namespace std;

template<typename T, typename... Args>
unique_ptr<T> make_unique(Args&&... args)
{
    return unique_ptr<T>(new T(forward<Args>(args)...));
}

#endif

