#ifndef GPUIMAGE_WIDGET_H
#define GPUIMAGE_WIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include "GPUImage-x.h"
#include "videodevice.h"
#include <QKeyEvent>

class GPUImage_Widget : public QOpenGLWidget, public QOpenGLFunctions
{
    Q_OBJECT
public:
    GPUImage_Widget(QWidget *parent = 0);
    ~GPUImage_Widget();
    void setFillMode(int fillMode);
    void seteEnvironment(VideoDevice *vd, int glContexID);
    void savePicture();
public slots:
    void update_ss();

protected:
    virtual void initializeGL();
    virtual void resizeGL(int w,int h);
    virtual void paintGL();

private:
    GPUImage::TargetView *targetView;
    GPUImage::Filter *filter1,*filter2,*filter3,*filter4,*filter5;
    GPUImage::LastMaskFilter *lastFilter;
    GPUImage::ZoomFilter *zoomFilter;
    GPUImage::ColorEnhanceFilter *colorEnhance;
    GPUImage::ContrastFilter *contrast;
    GPUImage::UndistortFilter *undistFilter;
    GPUImage::SourceCameraYUV *sourceCamera;

    int videoW,videoH;

    // 单帧图像数据指针
    unsigned char* rgb_data; //rgb data after gl process

    VideoDevice *vd;

    //OpenGL framebuffer ContexID 
    int glContexID;
    bool blCatchImgData;

};

#endif // GPUIMAGE_WIDGET_H
