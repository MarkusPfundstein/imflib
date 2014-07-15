#ifndef J2KFRAME_H_INCLUDED
#define J2KFRAME_H_INCLUDED

#include <vector>

struct J2kFrame
{
    J2kFrame() : data(0) { };

    std::vector<uint8_t> data;
};

#endif // J2KFRAME_H_INCLUDED
