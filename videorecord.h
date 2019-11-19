#ifndef VIDEORECORD_H
#define VIDEORECORD_H

#include <QString>
#include "common.h"

#define VIDEO_FILE_LIMIT_SIZE			((int)(1024 * 1024 * 1024)) //(1073741824) //Gbyte)
#define VIDEO_HARDDISK_WRONG 		(1 * 1024) //1G
#define IMAGE_HARDDISK_WRONG 		(10)  //10M
#define VIDEO_USBDISK_WRONG 		(1 * 1024) //1G
#define IMAGE_USBDISK_WRONG 		(10) //10M

#define ACCESS access  
#define MKDIR(a) mkdir((a),0755)  
#define KB 1024.0                                           // 2^10
#define MB 1048576.0                                        // 2^20
#define GB 1073741824.0                                     // 2^30

//----------ROCKCHIP MPP struct define -----------------
#include "rk_mpi.h"
#include "rk_mpi_cmd.h"
#include "rk_type.h"
#include "mpp_buffer.h"
#include "mpp_frame.h"
#include "mpp_packet.h"
#include "mpp_err.h"

#define MPP_ALIGN(x, a)         (((x)+(a)-1)&~((a)-1))
struct MPP_ENC_DATA
{
    // global flow control flag
    uint32_t frm_eos;
    uint32_t pkt_eos;
    uint32_t frame_count;
    uint64_t stream_size;

    // base flow context
    MppCtx ctx;
    MppApi *mpi;
    MppEncPrepCfg prep_cfg;
    MppEncRcCfg rc_cfg;
    MppEncCodecCfg codec_cfg;

    // input / output
    MppBuffer frm_buf;
    MppEncSeiMode sei_mode;

    uint32_t width;
    uint32_t height;
    uint32_t hor_stride;
    uint32_t ver_stride;
    MppFrameFormat fmt;
    MppCodingType type;
    uint32_t num_frames;

    // resources
    size_t frame_size;

    int32_t gop;
    int32_t fps;
    int32_t bps;

    FILE *fp_output;
};
//struct MPP_ENC_DATA mpp_enc_data;
//---------------------------    

//------------------BMP struct define-----------------
typedef unsigned char  BYTE;
typedef unsigned short	WORD;
typedef unsigned int  DWORD;

typedef struct tagBITMAPFILEHEADER{
     DWORD    bfSize;                // size BMP file ,unit is bytes
     DWORD    bfReserved;            // 0
     DWORD    bfOffBits;             // must be 54

}BITMAPFILEHEADER;


typedef struct tagBITMAPINFOHEADER{
     DWORD    biSize;                // must be 0x28
     DWORD    biWidth;               //
     DWORD    biHeight;              //
     WORD     biPlanes;              // must be 1
     WORD     biBitCount;            //
     DWORD    biCompression;         //
     DWORD    biSizeImage;           //
     DWORD    biXPelsPerMeter;       //
     DWORD    biYPelsPerMeter;       //
     DWORD    biClrUsed;             //
     DWORD    biClrImportant;        //
}BITMAPINFOHEADER;


typedef struct tagRGBQUAD{
     BYTE	rgbBlue;
     BYTE	rgbGreen;
     BYTE	rgbRed;
     BYTE	rgbReserved;
}RGBQUAD;

typedef enum
{
	eStrorageMode_HardDisk = 0x00,
	eStrorageMode_UsbDisk
}eStrorage_Mode;

typedef enum
{
	eStrorageFormat_VIDEO= 0x00,
	eStrorageFormat_IMAGE
}eStrorage_Format;

typedef enum
{
	eStrorageState_Normal = 0x00,
	eStrorageState_Waring, //will delete..
	eStrorageState_Wrong,
	eStrorageState_PathError,
	eStrorageState_OtherError,
}eStrorage_State;


namespace VideoRecord
{
    int init_mpp(int width, int height);
    int destroy_mpp();
    int video_record();
    int video_record_start();
    int video_record_stop();
    int video_record_write(void *addr,int bufSize);
    int take_picture();
    int take_picture_write(unsigned char *p, int width, int height);
    int savePicBmp(unsigned char *p, int width, int height, char *outfile );
    int savePicJpg(unsigned char *p, int width, int height, char *outfile);
    void NV12_T_RGB(unsigned char *nv12 , unsigned char *rgb, int width , int height);
    
    eStrorage_State check_disk_state(eStrorage_Format strorageformat);
    int CreatDir(char *pszDir); 
    int copy_file(char* src, char* des);
}

#endif // VIDEORECORD_H
