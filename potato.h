#ifndef POTATO_H
#define POTATO_H
#include <string.h>
#include <cstdlib>
class potato
{
public:
    int hops;
    int index;
    int track[512];
    potato() : hops(0), index(0) {memset(track, sizeof(track), 0);};
    potato(int hops_) : hops(hops_), index(0) {memset(track, sizeof(track), 0);};
};

#endif