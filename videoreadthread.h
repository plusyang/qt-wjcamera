#ifndef VIDEOREADTHREAD_H
#define VIDEOREADTHREAD_H

#include <QSemaphore>
#include <QThread>
#include <QMutex>
#include <QDebug>
#include "videodevice.h"


class VideoReadThread : public QThread
{
    Q_OBJECT
public:
    VideoReadThread(VideoDevice* vd);
    ~VideoReadThread();
    void stop();
    bool getIsStop();
    void setVideoDevice(VideoDevice *vd);
    void deleteLaterMY();
    
signals:
    void frameReceived();

protected:
    void run();

private:
    
    VideoDevice *vd;
    bool isStop;  //结束标识 
    QMutex lock;

};

#endif // VIDEOREADTHREAD_H
