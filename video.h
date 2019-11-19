/* ------------------------------------------------------------------------- */
/*									    									 */
/* video.h - definitions for the video interface			   				  */
/*									    									 */
/* ------------------------------------------------------------------------- */

#ifndef VIDEO_H
#define VIDEO_H

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

#include <stdbool.h>    //for using bool in C file
#include <stdint.h>     //for uint32_t uint16_t ...
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
//#include <string.h>   //for memcpy
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/mman.h>   //For mmap
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>
#include <signal.h>
#include <mp4v2/mp4v2.h>

/***************************************************
* linux系统相关结构体定义
****************************************************/
typedef struct Buffer_Object
{
    unsigned int       physPtr;
    unsigned int       size;
    unsigned int       u32Seq;
    int                type;
    char               owner[16];
    int                len;         			//视频数据长度
  //  int       userPtr[1920*1080*3 + 128];     	//视频数据指针 max for bmp image
    uint8_t       userPtr[1920*1080*3 + 128];     	//视频数据指针 max for bmp image
} Buffer_Object;
typedef struct Buffer_Object *Buffer_Handle;

/***************************************************
* video相关写操作
****************************************************/
bool MP4_Write_Init(char *filePath, int fs);
bool MP4_Write_Main(uint8_t *ptr, int len);
void MP4_Write_Exit(void);
void MP4_Write_SPS_PPS(uint8_t* ptr, int len);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // VIDEO_H

