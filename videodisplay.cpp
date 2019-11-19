#include "videodisplay.h"
#include "ui_videodisplay.h"
#include <QSemaphore>

VideoDisplay::VideoDisplay(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoDisplay)
{
    ui->setupUi(this);
    vd = new VideoDevice();
    readThread = new VideoReadThread(vd); //video读取线程
    view = (GPUImage_Widget *)ui->videoDisplay;
    view2 = (GPUImage_Widget *)ui->videoDisplay_2;
    view->seteEnvironment(vd,1);
    view2->seteEnvironment(vd,2);
	
    connect(readThread, &VideoReadThread::frameReceived,view,&GPUImage_Widget::update_ss);
    connect(readThread, &VideoReadThread::frameReceived,view2,&GPUImage_Widget::update_ss);
    if (!readThread->isRunning())
        readThread->start();
}

VideoDisplay::~VideoDisplay()
{
    readThread->stop();
    if (readThread->isFinished())
        delete readThread;
    qDebug()<<"readThread stop success..";

    delete vd;
    vd=NULL;

    qDebug()<<"delete vd success..";
    delete view;
    delete ui;
}

void VideoDisplay::keyPressEvent(QKeyEvent *event)
{
   //处理特殊按键，ESC关闭
   if (event->key() == Qt::Key_Escape) {
       this->close();
       qDebug()<<"image close";
   }
   return;
}
