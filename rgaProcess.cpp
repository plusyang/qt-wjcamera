#include "rgaProcess.h"
#include "common.h"
#include <sys/time.h>

static RockchipRga *rkRga = new RockchipRga();
static int rga_width, rga_height;
static RGA_ZOOM zoom = RGA_ZOOM_1x0;
void rga_init(int width, int height)
{
    rkRga->RkRgaInit();
    rga_width = width;
    rga_height = height;
}

void rga_deinit()
{
    delete rkRga;
}

void rga_zoom(int zoomVal)
{
    zoom = (RGA_ZOOM)zoomVal;
}

int rga_process(unsigned char* psrc, unsigned char* pdst)
{
    int ret = 0;
    int srcWidth,srcHeight,srcFormat;
	int dstWidth,dstHeight,dstFormat;
	
    srcWidth = rga_width;
    srcHeight = rga_height;
	srcFormat = RK_FORMAT_YCrCb_420_SP;

    dstWidth = rga_width;
    dstHeight = rga_height;
	dstFormat = RK_FORMAT_YCrCb_420_SP;

int zoomWidth,zoomHeight;
    int zoomX,zoomY;
    float zoomFactor=1.0;

    if(zoom == RGA_ZOOM_1x0){
        zoomFactor=1.0;
    }else if(zoom == RGA_ZOOM_1x2){
        zoomFactor=1.2;
    }else if(zoom == RGA_ZOOM_1x4){
        zoomFactor=1.4;
    }else if(zoom == RGA_ZOOM_1x6){
        zoomFactor=1.6;
    }else if(zoom == RGA_ZOOM_1x8){
        zoomFactor=1.8;
    }else if(zoom == RGA_ZOOM_2x0){
        zoomFactor=2.0;
    }

    zoomWidth = rga_width/zoomFactor;
    zoomHeight = rga_height/zoomFactor;
    zoomX = (rga_width-rga_width/zoomFactor)/2.0;
    zoomY = (rga_height-rga_height/zoomFactor)/2.0;
    zoomWidth = rga_width - 2*zoomX;
    zoomHeight = rga_height - 2*zoomY;
    printf("%d,%d,%d,%d \n",zoomX,zoomY,zoomWidth,zoomHeight);

    //malloc tmp memery
    void* tmp_mem = malloc(zoomWidth * zoomHeight * 3 / 2);
    if (!tmp_mem) {
        free(tmp_mem);
        return -ENOMEM;
    }

    rga_info_t src;
    rga_info_t dst;
    rga_info_t tmp;

    memset(&src, 0, sizeof(rga_info_t));
    src.fd = -1;
    src.mmuFlag = 1;
    src.virAddr = psrc;
    
    memset(&dst, 0, sizeof(rga_info_t));
    dst.fd = -1;
    dst.mmuFlag = 1;
    dst.virAddr = pdst;

    memset(&tmp, 0, sizeof(rga_info_t));
    tmp.fd = -1;
    tmp.mmuFlag = 1;
    tmp.virAddr = tmp_mem;

    rga_set_rect(&src.rect, zoomX,zoomY,zoomWidth,zoomHeight,srcWidth/*stride*/,srcHeight,srcFormat);
    rga_set_rect(&tmp.rect, 0,0,zoomWidth,zoomHeight,zoomWidth/*stride*/,zoomHeight,srcFormat);
    ret = rkRga->RkRgaBlit(&src, &tmp, NULL);

    rga_set_rect(&tmp.rect, 0,0,zoomWidth,zoomHeight,zoomWidth/*stride*/,zoomHeight,srcFormat);
    rga_set_rect(&dst.rect, 0,0,dstWidth,dstHeight,dstWidth/*stride*/,dstHeight,dstFormat);
    ret = rkRga->RkRgaBlit(&tmp, &dst, NULL);


    // rga_set_rect(&src.rect, 0,0,srcWidth,srcHeight,srcWidth/*stride*/,srcHeight,srcFormat);
    // rga_set_rect(&dst.rect, 0,0,dstWidth,dstHeight,dstWidth/*stride*/,dstHeight,dstFormat);
    // ret = rkRga->RkRgaBlit(&src, &dst, NULL);

    // struct timeval tpend1, tpend2;
    // long usec1 = 0;
    // gettimeofday(&tpend1, NULL);   
    // ret = rkRga->RkRgaBlit(&src, &dst, NULL);
    // gettimeofday(&tpend2, NULL);
    // usec1 = 1000 * (tpend2.tv_sec - tpend1.tv_sec) + (tpend2.tv_usec - tpend1.tv_usec) / 1000;
    //printf("cost_time=%ld ms\n", usec1);
    if (ret) {
        printf("RkRgaBlit error : %s\n",strerror(errno));
    }

    return ret;
}
