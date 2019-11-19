
#include "videorecordthread.h"
#include "videorecord.h"
#include <unistd.h>
#include <QSemaphore>

extern bool isSaveVideo;
extern bool isSavePic;
extern QSemaphore semPic;
extern QSemaphore semVideo;

VideoRecordThread::VideoRecordThread()
{
    isStop=false;
}

VideoRecordThread::~VideoRecordThread(){}

void VideoRecordThread::setVD(VideoDevice *vd)
{
    this->vd=vd;
}

void VideoRecordThread::stop()
{
    isStop=true;
    quit();
    wait();
}

void VideoRecordThread::run()
{
    while(1)
    {
        usleep(1000*20);
        if(isStop == true)
            return;
        if(vd == NULL){
            continue;
        }

        if(isSaveVideo){
            semVideo.acquire();
            int ret = 0;           
            // ret = VideoRecord::video_record_write(vd->yuv_buffer, (int)vd->yuv_buffer_len);
            if(ret < 0) qDebug() << "video_record_write falied.";

        } 

    }
}


//-------------------------------------------------------------


PictureRecordThread::PictureRecordThread()
{
    isStop=false;
}

PictureRecordThread::~PictureRecordThread(){}

void PictureRecordThread::setVD(VideoDevice *vd)
{
    this->vd=vd;
}

void PictureRecordThread::stop()
{
    isStop=true;
    quit();
    wait();
}

void PictureRecordThread::run()
{
    while(1)
    {
        usleep(1000*30); 
        if(isStop == true)
            return;
        if(vd == NULL){
            continue;
        }
        if(isSavePic) {
            semPic.acquire();
            int ret = 0;
            ret = VideoRecord::take_picture_write(vd->gl_rgb_buffer,vd->get_width(),vd->get_height());
            if(ret < 0) qDebug() << "take_picture_write  falied.";
        }

    }
}
