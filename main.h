#ifndef MAIN_H
#define MAIN_H

#include "gba.h"

struct donut {
    int row;
    int col;
    int width;
    int height;
    int velR;
    int velC;
    int size;
};

struct broccoli {
    int row;
    int col;
    int width;
    int height;
    int velR;
    int velC;
    int size;
};

struct player {
    int row;
    int col;
    int width;
    int height;
};

struct sprite {
    int row;
    int col;
    int width;
    int height;
    int velR;
    int velC;
    int size;
};

#endif
