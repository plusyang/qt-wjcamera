#ifndef __COMMON_H__
#define __COMMON_H__

//----------video info---------------
#define CAMERA_DEV_NAME "/dev/video0"
#define ISP_DEV_NAME    "/dev/video1"
#define ISP_IQ_FILE_OH01A10     "/etc/cam_iq-oh01a10.xml"
#define ISP_IQ_FILE_OV2741     "/etc/cam_iq-ov2741.xml"
#define ISP_IQ_FILE_OV13850     "/etc/cam_iq-ov13850.xml"

#define IMAGEWIDTH_HD 1280
#define IMAGEHEIGHT_HD 720
#define IMAGEWIDTH_FHD 1920
#define IMAGEHEIGHT_FHD 1080
#define  IMAGECHANEL   3

// ------ OSD struct define----------
#include <QString>
typedef struct OsdMsg{
    int shCnt;
    int initShCnt;
    QString msg;
    int type; // 0 -no osd,  1 -save image, 2 -save video
} OsdMsg;


//------------video and image save define-----------
#define FILE_ROOTPATH_HARDDISK "/mnt/media/hard-disk"
#define FILE_ROOTPATH_USBDISK "/mnt/media/usb-disk"
#define FILE_ROOTPATH_APP "WJDES_DCIM"



//---------------Log define---------------------
#include <QDebug>
#define LOG_INFO "INFO"
#define LOG_WARNING "WARNING"
#define LOG_DEBUG "DEBUG"
void QLog(const char* tag, const char * format, ...);


#endif //__COMMON_H__
