#ifndef VIDEODISPLAY_H
#define VIDEODISPLAY_H

#include <QWidget>
#include <QKeyEvent>
#include "videodevice.h"
#include "videoreadthread.h"
#include "oglwidget.h"
#include "gpuimage_widget.h"

namespace Ui {
class VideoDisplay;
}

class VideoDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit VideoDisplay(QWidget *parent = 0);
    ~VideoDisplay();
    void keyPressEvent(QKeyEvent *event);
    
private:
    Ui::VideoDisplay *ui;
    VideoDevice *vd;
    VideoReadThread *readThread;
    GPUImage_Widget *view, *view2;
//    oglWidget *view;
};

#endif // VIDEODISPLAY_H
