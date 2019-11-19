#ifndef VIDEORECORDTHREAD_H
#define VIDEORECORDTHREAD_H

#include <QThread>
#include <QDebug>
#include "videodevice.h"

class VideoRecordThread : public QThread
{
    Q_OBJECT
public:
    VideoRecordThread();
    ~VideoRecordThread();
    void stop();
    void setVD(VideoDevice *vd);
    
protected:
    void run();

private:
    VideoDevice *vd;
    bool isStop;

};

class PictureRecordThread : public QThread
{
    Q_OBJECT
public:
    PictureRecordThread();
    ~PictureRecordThread();
    void stop();
    void setVD(VideoDevice *vd);
 
protected:
    void run();

private:
    VideoDevice *vd;
    bool isStop;

};

#endif // VIDEORECORDTHREAD_H