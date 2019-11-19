#include <QDebug>
#include <QByteArray>
#include <QFile>
#include <QtGui>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/time.h>
#include <linux/videodev2.h>
#include <linux/version.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <jpeglib.h>

#include "videodevice.h"
#include "videorecord.h"

#include "isp.h"
#include "rgaProcess.h"

//#define SAVE_PICTURE_TYPE_BMP
#define CLEAR(x) memset(&(x), 0, sizeof(x))
static int verbose = 1;
#define pr_debug(fmt, arg...) \
    if (verbose) qDebug(fmt, ##arg)

OsdMsg osd_msg;

extern bool isSaveVideo;
extern bool isSavePic;

VideoDevice::VideoDevice()
{
    this->dev_name = CAMERA_DEV_NAME;
    this->nbufs = 4;
    this->fd = -1;
    this->pixelformat = V4L2_PIX_FMT_NV12;
    this->fpsRate = 60;
    
    if(video_open()!=0){
        emit signal_VideoDevice_msg("video_open:error");
        return;
    }
    if(video_init()!=0){ //init ,and set imageWidth imageHeight,and set isp_iq_file
        emit signal_VideoDevice_msg("video_init:error");
        return;
    }

    yuv_buffer = (unsigned char *)malloc(imageWidth*imageHeight*3/2*sizeof(char));
    rga_yuv_buffer = (unsigned char *)malloc(imageWidth*imageHeight*3/2*sizeof(char));
    gl_rgb_buffer = (unsigned char *)malloc(imageWidth*imageHeight*4*sizeof(char));
    
    rga_init(imageWidth, imageHeight);

    if(isp_start_func(this->fd, this->isp_iq_file)!=0){
        emit signal_VideoDevice_msg("isp_start_func:error");
        return;
    }
    if(video_start_capture()!=0){
        emit signal_VideoDevice_msg("video_start_capture:error");
        return;
    }

}

VideoDevice::~VideoDevice()
{
    if(isp_stop_func()!=0){
        emit signal_VideoDevice_msg("rkisp_stop_func:error");
        return;
    }
    if(video_stop_capture()!=0){
        emit signal_VideoDevice_msg("video_stop_capture:error");
        return;
    }
    if(video_uninit()!=0){
        emit signal_VideoDevice_msg("video_uninit:error");
        return;
    }
    if(video_close()!=0){
        emit signal_VideoDevice_msg("video_close:error");
        return;
    }
    delete yuv_buffer;
    delete rga_yuv_buffer;;
    rga_deinit();
}

int VideoDevice::video_stop_capture()
{
    enum v4l2_buf_type type;

    QLog(LOG_INFO,"Call VIDIOC_STREAMOFF");
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type)){
        errno_exit("VIDIOC_STREAMOFF");
        return -1;
    }

    return 0;
}

int VideoDevice::video_start_capture()
{
    int i;
    enum v4l2_buf_type type;

    for (i = 0; i < nbufs; ++i) {
        struct v4l2_buffer buf;

        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf)){
            errno_exit("VIDIOC_QBUF");
            return -1;
        }
    }
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(fd, VIDIOC_STREAMON, &type)){
        errno_exit("VIDIOC_STREAMON");
        return -1;
    }

    return 0;
}


int VideoDevice::video_open()
{
    fd = open(dev_name, O_RDWR/*   | O_NONBLOCK*/,0);
    if (fd == -1)
    {
        errno_exit("video_open");
        return -1;
    }
    return 0;
}

int VideoDevice::video_close()
{
    int ret;
    ret = close(fd);
    if(ret == -1){
        errno_exit("CLOSE");
        return -1;
    }
    fd = -1;
    return 0;
}

int VideoDevice::video_init()
{
    struct v4l2_capability cap;
    int ret;

    CLEAR(cap);
    ret = xioctl(fd, VIDIOC_QUERYCAP, &cap);
    if (ret == -1) {
        if (EINVAL == errno) {
            QLog(LOG_INFO,"%s is no V4L2 device", dev_name);
            return -1;
        }
        else {
            errno_exit("VIDIOC_QUERYCAP");
            return -1;
        }
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        QLog(LOG_INFO,"%s is no video capture device", dev_name);
        return -1;
    }
    if (!(cap.capabilities & V4L2_CAP_STREAMING))
    {
        QLog(LOG_INFO,"%s does not support streaming i/o", dev_name);
        return -1;
    }


    pr_debug("\tdriver: %s\n" "\tcard: %s \n" "\tbus_info: %s\n", cap.driver, cap.card, cap.bus_info);
    pr_debug("\tversion: %u.%u.%u\n", (cap.version >> 16) & 0xFF, (cap.version >> 8) & 0xFF, cap.version & 0xFF);
    pr_debug("\tcapabilities: 0x%08x\n", cap.capabilities);

    //video_cropcap_set();   //cifisp not support
    ret = video_format_get();  //set imageWidth imageHeight,and set isp_iq_file
    if(ret == -1){
        return -1;
    }
    video_framerate_set(); //cifisp not support
    //get_v4l2_fmt_info(fd);
    ret = video_mmap_init();
    if(ret == -1){
        return -1;
    }
    return 0;
}

int VideoDevice::video_uninit()
{
    int i;

    for (i = 0; i < nbufs; ++i){
        if (-1 == munmap(mem0[i].start, mem0[i].length))
            errno_exit("munmap");
    }
    free(mem0);

    return 0;
}

int VideoDevice::video_cropcap_set()
{
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;

    /*  Select video input, video standard and tune here. */
    CLEAR(cropcap);
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
        pr_debug("\tcropcap.type: %d\n", cropcap.type);
        pr_debug("\tcropcap.bounds.left: %d\n", cropcap.bounds.left);
        pr_debug("\tcropcap.bounds.top: %d\n", cropcap.bounds.top);
        pr_debug("\tcropcap.bounds.width: %d\n", cropcap.bounds.width);
        pr_debug("\tcropcap.bounds.height: %d\n", cropcap.bounds.height);

        pr_debug("\tcropcap.defrect.left: %d\n", cropcap.defrect.left);
        pr_debug("\tcropcap.defrect.top: %d\n", cropcap.defrect.top);
        pr_debug("\tcropcap.defrect.width: %d\n", cropcap.defrect.width);
        pr_debug("\tcropcap.defrect.height: %d\n", cropcap.defrect.height);

        pr_debug("\tcropcap.pixelaspect.numerator: %d\n", cropcap.pixelaspect.numerator);
        pr_debug("\tcropcap.pixelaspect.denominator: %d\n", cropcap.pixelaspect.denominator);
        pr_debug("\n");

        CLEAR(crop);
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect;   //  reset to default

        if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop)) {
            switch (errno)
            {
                case EINVAL:
                    //  Cropping not supported.
                    break;
                default:
                    //  Errors ignored.
                    pr_debug("\tcropping not supported\n");
                    break;
            }
             errno_exit("VIDIOC_S_CROP");
             return -1;
        }
    }
    else {
        errno_exit("VIDIOC_CROPCAP");
        return -1;
    }

    return 0;
}

int VideoDevice::video_mmap_init()
{
    struct v4l2_requestbuffers req;

    CLEAR(req);
    req.count = nbufs;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req))
    {
        if (EINVAL == errno)
        {
            QLog(LOG_INFO,"%d does not support memory mapping", fd);
            return -1;
        } else
        {
            errno_exit("VIDIOC_REQBUFS");
            return -1;
        }
    }
    pr_debug("\treq.count: %d\n", req.count);
    pr_debug("\treq.type: %d\n", req.type);
    pr_debug("\treq.memory: %d\n", req.memory);
    pr_debug("\n");

    if (req.count < 2)
    {
        QLog(LOG_INFO,"Insufficient buffer memory on %d", fd);
        return -1;
    }

    unsigned int i;
    mem0 = (struct buffer*)calloc(req.count, sizeof(*mem0));
    if (!mem0) {
        errno_exit("OUT_OF_MEMORY");
        //exit(EXIT_FAILURE);
    }
    struct v4l2_buffer buf;
    for (i = 0; i < req.count; ++i)
    {
        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf)) {
            errno_exit("VIDIOC_QUERYBUF");
            return -1;
        }

        mem0[i].length = buf.length;
        mem0[i].start = mmap(NULL /*  start anywhere */,
               buf.length,
               PROT_READ | PROT_WRITE /*  required */,
               MAP_SHARED /*  recommended */,
               fd, buf.m.offset);

        if (mem0[i].start == MAP_FAILED) {
            errno_exit("mmap");
            return -1;
        }

        QLog(LOG_INFO,"Buffer %u mapped at address %p.", i, mem0[i].start);

    }

    return 0;
}

void VideoDevice::get_v4l2_fmt_info(int fd)
{
    struct v4l2_streamparm Stream_Parm;
    struct v4l2_format Format;
    printf("************************Get format info********************************\n");
    Format.type= V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(fd,VIDIOC_G_FMT,&Format)==-1)
    {
        perror("ioctl");
        //exit(EXIT_FAILURE);
    }
    printf(">:[width:%d]\t[pixelformat:%d]\n",Format.fmt.pix.width,Format.fmt.pix.height);
    printf(">:[format:%d]\t[field:%d]\n",Format.fmt.pix.pixelformat,Format.fmt.pix.field);
    printf(">:[bytesperline:%d]\t[sizeimage:%d]\n",Format.fmt.pix.bytesperline,Format.fmt.pix.sizeimage);
    printf(">:[colorspace:%d]\n",Format.fmt.pix.colorspace);
    printf("***********************************************************************\n");
    printf("\n");

    printf("************************Get Stream_Parm********************************\n");
    Stream_Parm.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(fd,VIDIOC_G_PARM,&Stream_Parm)==-1)
    {
        perror("ioctl");
        //exit(EXIT_FAILURE);
    }
    printf(">:[Frame rate:%u] [%u]\n",Stream_Parm.parm.capture.timeperframe.numerator,Stream_Parm.parm.capture.timeperframe.denominator);
    printf(">:[capability:%d] [capturemode:%d]\n",Stream_Parm.parm.capture.capability,Stream_Parm.parm.capture.capturemode);
    printf(">:[extendemode:%d] [readbuffers:%d]\n",Stream_Parm.parm.capture.extendedmode,Stream_Parm.parm.capture.readbuffers);
    printf("***********************************************************************\n");
    printf("\n");
}

int VideoDevice::video_format_get()
{
    struct v4l2_format fmt;
    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(fd, VIDIOC_G_FMT, &fmt))
    {
        errno_exit("VIDIOC_G_FMT");
        return -1;
    }

    pr_debug("\tVideo format get: width: %u height: %u buffer size: %u\n",
        fmt.fmt.pix.width, fmt.fmt.pix.height, fmt.fmt.pix.sizeimage);
    this->imageWidth = fmt.fmt.pix.width;
    this->imageHeight = fmt.fmt.pix.height;
    
    if(this->imageWidth == IMAGEWIDTH_HD && this->imageHeight == IMAGEHEIGHT_HD){
        strcpy(this->isp_iq_file, ISP_IQ_FILE_OH01A10);
    }else if (this->imageWidth == IMAGEWIDTH_FHD && this->imageHeight == IMAGEHEIGHT_FHD){
        strcpy(this->isp_iq_file, ISP_IQ_FILE_OV2741);
    }else{
        strcpy(this->isp_iq_file, ISP_IQ_FILE_OV13850);
        //pr_debug("\tthis Video format not allowed.");
        //return -1;
    }
    
    return 0;
}


int VideoDevice::video_format_set()
{
    /*
    struct v4l2_format fmt;
    unsigned int min;

    CLEAR(fmt);

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = this->imageWidth;
    fmt.fmt.pix.height = this->imageHeight;
    fmt.fmt.pix.pixelformat = pixelformat;
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

    pr_debug("\tfmt.fmt.pix.pixelformat: %c,%c,%c,%c\n",
            fmt.fmt.pix.pixelformat & 0xFF,
            (fmt.fmt.pix.pixelformat >> 8) & 0xFF,
            (fmt.fmt.pix.pixelformat >> 16) & 0xFF,
            (fmt.fmt.pix.pixelformat >> 24) & 0xFF);

    if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
    {
        errno_exit("VIDIOC_S_FMT");
        return -1;
    }

    pr_debug("\tVideo format set: width: %u height: %u buffer size: %u\n",
            fmt.fmt.pix.width, fmt.fmt.pix.height, fmt.fmt.pix.sizeimage);

    //  Buggy driver paranoia.

    min = fmt.fmt.pix.width * 2;
    if (fmt.fmt.pix.bytesperline < min)
        fmt.fmt.pix.bytesperline = min;
    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
    if (fmt.fmt.pix.sizeimage < min)
        fmt.fmt.pix.sizeimage = min;
     */
    return 0;
}

int VideoDevice::video_framerate_set()
{
    struct v4l2_streamparm parm;

    CLEAR(parm);
    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if(-1 ==  xioctl(fd, VIDIOC_G_PARM, &parm))
    {
        errno_exit("VIDIOC_G_PARM");
        return -1;
    }

    pr_debug("\tCurrent frame rate: %u/%u\n",
            parm.parm.capture.timeperframe.numerator,
            parm.parm.capture.timeperframe.denominator);

    parm.parm.capture.timeperframe.numerator = 1;
    parm.parm.capture.timeperframe.denominator = fpsRate;//设置帧率

    if(-1 == xioctl(fd, VIDIOC_S_PARM, &parm))
    {
        errno_exit("VIDIOC_S_PARM");
        return -1;
    }

    if(-1 == xioctl(fd, VIDIOC_G_PARM, &parm))
    {
        errno_exit("VIDIOC_G_PARM");
        return -1;
    }

    pr_debug("\tFrame rate set: %u/%u\n",
            parm.parm.capture.timeperframe.numerator,
            parm.parm.capture.timeperframe.denominator);

    return 0;
}

void VideoDevice::errno_exit(const char *s)
{
    QLog(LOG_INFO,"%s error %d, %s", s, errno, strerror(errno));
}

int VideoDevice::xioctl(int fh, int request, void *arg)
{
	int r;

	do
	{
		r = ioctl(fh, request, arg);
	} while (-1 == r && EINTR == errno);

	return r;
}



void VideoDevice::osd_do()
{
    if(osd_msg.type == -1){ //do not send signal
        return;
    }else if(osd_msg.type == 0){ //osd =""
        emit sigOSD(QString(""));
        osd_msg.type = -1; 
    }else {
        if(osd_msg.shCnt == osd_msg.initShCnt){  
            emit sigOSD(osd_msg.msg);
        }else if(osd_msg.shCnt == 0){
            osd_msg.type = 0;
        }
        osd_msg.shCnt--;
    }
}

#define VIDEO_FRAME_COUNT
int VideoDevice::get_frame()
{   
    struct v4l2_buffer buf0;
    CLEAR(buf0);

    buf0.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf0.memory = V4L2_MEMORY_MMAP;
    
    // Dequeue a buffe
    if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf0)){
        errno_exit("VIDIOC_DQBUF");
        return -1;
    }

    assert(buf0.index < nbufs);
    memcpy(yuv_buffer, (unsigned char *)mem0[buf0.index].start, buf0.bytesused);
    yuv_buffer_len = buf0.bytesused;
    //rga_process(yuv_buffer, rga_yuv_buffer);
    
    // Requeue the buffe
    if (-1 == xioctl(fd, VIDIOC_QBUF, &buf0)){
        errno_exit("VIDIOC_QBUF");
        return -1;
    }


    #ifdef VIDEO_FRAME_COUNT
    static unsigned long frame_count = 0;
    static unsigned long count=1;
    static timeval start,end;
    long dif_sec, dif_usec;

    if(frame_count%700 == 0){
        gettimeofday(&start,NULL);
        count=frame_count;
    }

    if(frame_count == count+600 ){

        gettimeofday(&end,NULL);
        dif_sec = end.tv_sec - start.tv_sec;
        dif_usec = end.tv_usec - start.tv_usec;

        printf("=========== start : %ld %ld\n",start.tv_sec,start.tv_usec);
        printf("=========== end : %ld %ld\n",end.tv_sec,end.tv_usec);
        printf(" total := %ld(us)\n",dif_sec*1000000+dif_usec);
    }

    frame_count++;
#endif

    return 0;
}


int VideoDevice::get_frame(unsigned char **yuv_buffer_pointer, size_t *len)
{
    struct v4l2_buffer buf0;
    // Dequeue a buffe
    CLEAR(buf0);
    buf0.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf0.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf0)){
        errno_exit("VIDIOC_DQBUF");
        return -1;
    }

    assert(buf0.index < nbufs);
    (*yuv_buffer_pointer) = (unsigned char *)mem0[buf0.index].start;

    *len = buf0.bytesused;
    index = buf0.index;

    return 0;
}


int VideoDevice::unget_frame()
{
    if(index != -1)
    {
        v4l2_buffer buf0;
        CLEAR(buf0);
        buf0.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf0.memory = V4L2_MEMORY_MMAP;
        buf0.index = index;

        // Requeue the buffe
        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf0))
        {
            errno_exit("VIDIOC_QBUF");
            close(fd);
            return -1;
        }
        return 0;
    }
    return -1;
}
