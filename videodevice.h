#ifndef VIDEO_DEVICE_H
#define VIDEO_DEVICE_H

#include "common.h"
#include <stdio.h>
#include <QObject>

#define V4L_BUFFERS_DEFAULT     4//16
#define V4L_BUFFERS_MAX         16//32

struct buffer {
        void *start;
        size_t length;
};

class VideoDevice : public QObject
{
    Q_OBJECT
public:
    explicit VideoDevice();
    ~VideoDevice();

    int video_open();
    int video_close();
    int video_init();
    int video_uninit();
    int video_cropcap_set();
    int video_format_set();
    int video_format_get();
    int video_framerate_set();
    int video_mmap_init();
    int video_start_capture();
    int video_stop_capture();

    void get_v4l2_fmt_info(int fd); 

    int get_frame();
    int get_frame(unsigned char **yuv_buffer_pointer, size_t *len);
    int get_width(){ return this->imageWidth; }
    int get_height(){ return this->imageHeight; }
    int unget_frame();
    void osd_do();

    void errno_exit(const char *s);
    int xioctl(int fh, int request, void *arg);
    
private:
    int imageWidth;
    int imageHeight;
    int pixelformat;
    int nbufs;
    int fpsRate;

    struct buffer *mem0;
    const char *dev_name;
    int fd;//video0 file
    int index;

    char isp_iq_file[50];

public:
    unsigned char *yuv_buffer, *rga_yuv_buffer;;
    unsigned char *gl_rgb_buffer;
    size_t yuv_buffer_len;

signals:
    void sigOSD(QString str);
    void signal_VideoDevice_msg(QString str);
public slots:

};

#endif // VIDEO_DEVICE_H
