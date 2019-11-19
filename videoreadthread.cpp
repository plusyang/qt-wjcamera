/****************************************
* 类名：VideoReadThread
* 功能：读取图像帧线程，得到一帧数据后通知oglwidget显示
* 作者：yangzhenhao
* 日期：20190201
* 版本：1.0
*
* 日期：201902011
* 版本：1.5
* modify by yzh, fixed 0 copy, use QMutex
*****************************************/
#include "videoreadthread.h"
#include <string.h>

extern bool isSaveVideo;
extern bool isSavePic;
QSemaphore sem(0);
QSemaphore semPic(0);
QSemaphore semVideo(0);

VideoReadThread::VideoReadThread(VideoDevice* vd)
{
    this->vd = vd;
    isStop = false;
}


VideoReadThread::~VideoReadThread()
{
}

void VideoReadThread::run()
{
    while(1)
    {
        if(isStop == true){            
            lock.unlock();
            return;
        }
        
        if(vd == NULL){
            lock.unlock();
            continue;
        }

        lock.lock();
        //从缓存队列中抽出一帧图像
        if(-1 == vd->get_frame()){
           qDebug() << " get_frame fail";
           return;
        }
        lock.unlock();

        emit frameReceived();        
        //Record..
        if(isSavePic){
            semPic.release();
        }
        
        if(isSaveVideo) {            
            semVideo.release();
        }

    }

}

void VideoReadThread::setVideoDevice(VideoDevice *vd)
{
    this->vd = vd;
}

void VideoReadThread::stop()
{
    isStop=true;
    quit();
    wait();
}

bool VideoReadThread::getIsStop()
{
    return isStop;
}

void VideoReadThread::deleteLaterMY()
{
    qDebug()<<"VideoReadThread deleteLater..";
    QThread::deleteLater();
}
