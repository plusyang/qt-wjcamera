#ifndef RGAPROCESS_H
#define RGAPROCESS_H

#include "RockchipRga.h"

enum RGA_ZOOM {
    RGA_ZOOM_1x0,
    RGA_ZOOM_1x2,
    RGA_ZOOM_1x4,
    RGA_ZOOM_1x6,
    RGA_ZOOM_1x8,
    RGA_ZOOM_2x0,
};

void rga_init(int width, int height);
void rga_deinit();
void rga_zoom(int zoomVal);
int rga_process(unsigned char* psrc, unsigned char* pdst);

#endif