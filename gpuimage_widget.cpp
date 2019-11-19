#include "gpuimage_widget.h"
#include <QImage>
#include <QDebug>
#include "common.h"
#include "record_picture.h"
#include "gpuimage_params.h"
#include "gpuimage_filter.h"
#include <sys/time.h>
#include <opencv2/opencv.hpp>
#include "GPUImage/filter/ColorEnhanceFilter.h"

GLuint pboIdes[2] = {0,0};
GPUImage_Widget::GPUImage_Widget(QWidget *parent):
    QOpenGLWidget(parent){
}

GPUImage_Widget::~GPUImage_Widget()
{
    makeCurrent();
    blCatchImgData = false;
    targetView->release();
    filter1->release();
    filter2->release();
    filter3->release();
    lastFilter->release();
    colorEnhance->release();
    undistFilter->release();
    zoomFilter->release();
    sourceCamera->release();
    doneCurrent();
}

void GPUImage_Widget::initializeGL()
{
    this->initializeOpenGLFunctions();
    glGenBuffers(2, pboIdes);
    qDebug() << "pboids[0] = " << pboIdes[0];
    qDebug() << "pboids[1] = " << pboIdes[1];

    targetView = new GPUImage::TargetView();
    sourceCamera = GPUImage::SourceCameraYUV::create(glContexID);
    //filters ..
    filter1 = GPUImage::Yuv2RgbFilter::create(glContexID);
    filter2 = GPUImage::BilateralFilter::create(glContexID);
    filter3 = GPUImage::SharpenFilter::create(glContexID);
    zoomFilter = GPUImage::ZoomFilter::create(glContexID);
    lastFilter = GPUImage::LastMaskFilter::create(glContexID);
    colorEnhance = GPUImage::ColorEnhanceFilter::create(glContexID);
    undistFilter = GPUImage::UndistortFilter::create(glContexID);

    //八角形掩模
    QImage im;
    im.load(":/XSresource/mask1280x720.png");
    im= im.convertToFormat(QImage::Format_RGBA8888);
    unsigned char *imdata = im.bits();
    qDebug()<<im.width()<<im.height();
    lastFilter->setMaskPixel(im.width(),im.height(),imdata);

    sourceCamera->addTarget(filter1)->addTarget(filter2)->addTarget(filter3)->addTarget(colorEnhance)->addTarget(undistFilter)
        ->addTarget(zoomFilter)->addTarget(lastFilter)->addTarget(targetView);

    GLint imp_fmt, imp_type;
    glGetIntegerv (GL_IMPLEMENTATION_COLOR_READ_FORMAT, &imp_fmt);
    glGetIntegerv (GL_IMPLEMENTATION_COLOR_READ_TYPE,   &imp_type);
    printf ("glReadPixel Supported Color Format/Type: %x/%x\n", imp_fmt, imp_type);

}
void GPUImage_Widget::savePicture()
{
    blCatchImgData = true;
    savePictureQt(vd->gl_rgb_buffer,videoW,videoH,"/mnt/1111.jpg");
}

void GPUImage_Widget::resizeGL(int w,int h)
{
    targetView->onSizeChanged(w,h);
    glViewport(0,0,w,h);
}

void GPUImage_Widget::paintGL()
{
    //Rotation and Zoom
    ROTATION_MODE rotation = (ROTATION_MODE)(GPUImage_Params::getInstance()->getRotation());
    float zoom = GPUImage_Params::getInstance()->getZoom();    
    zoomFilter->setZoom(zoom);
    sourceCamera->setFrameData(videoW,videoH,vd->yuv_buffer,(GPUImage::RotationMode)rotation);
    
    //Smooth
    GPUImage::BilateralFilter *f2 = (GPUImage::BilateralFilter *)filter2;
    f2->setTexelSpacingMultiplier(GPUImage_Params::getInstance()->getBilateral_multiplier());
    f2->setDistanceNormalizationFactor(GPUImage_Params::getInstance()->getBilateral_normalization());

    //Sharpen
    GPUImage::SharpenFilter *f3 = (GPUImage::SharpenFilter*)filter3;
    f3->setParamters(GPUImage_Params::getInstance()->getSharpness());

    //Color Enhance
   colorEnhance->setRGBAdjust(GPUImage_Params::getInstance()->getColor_r(),
               GPUImage_Params::getInstance()->getColor_g(),
               GPUImage_Params::getInstance()->getColor_b());

    //lastMask and undist enable
    lastFilter->setEnable(GPUImage_Params::getInstance()->getLastMask_enable());
    undistFilter->setEnable(GPUImage_Params::getInstance()->getUndist_enable());
    
    struct timeval tpend1, tpend2;
    long usec1 = 0;
    gettimeofday(&tpend1, NULL);   
    sourceCamera->proceed(true);
    gettimeofday(&tpend2, NULL);
    usec1 = 1000 * (tpend2.tv_sec - tpend1.tv_sec) + (tpend2.tv_usec - tpend1.tv_usec) / 1000;
    // printf("--------------------sourceCamera->proceed=%ld ms\n", usec1);
    if(blCatchImgData)
    {
        rgb_data = sourceCamera->captureAProcessedFrameData(lastFilter,videoW,videoH);//lzx lastFilter
        memcpy(vd->gl_rgb_buffer,rgb_data,videoW*videoH*sizeof(char)*4); //TODO..
    }
    //rgb_data = sourceCamera->captureAProcessedFrameData(filter1,videoW,videoH);
    //memcpy(vd->gl_rgb_buffer,rgb_data,videoW*videoH*sizeof(char)*4); //TODO..
    //delete rgb_data;
    //rgb_data = 0;

    // gettimeofday(&tpend2, NULL);
    // usec1 = 1000 * (tpend2.tv_sec - tpend1.tv_sec) + (tpend2.tv_usec - tpend1.tv_usec) / 1000;
    // printf("captureAProcessedFrameData :cost_time=%ld ms\n", usec1);

    QMetaObject::invokeMethod(this,"update_ss",Qt::QueuedConnection);

}

void GPUImage_Widget::setFillMode(int fillMode)
{
    targetView->setFillMode((GPUImage::TargetView::FillMode)fillMode);
}


void GPUImage_Widget::update_ss()
{
    if(vd == NULL){
        return;
    }
    update();
    
}


void GPUImage_Widget::seteEnvironment(VideoDevice *vd, int glContexID)
{
    this->vd=vd;
    this->videoW = vd->get_width();
    this->videoH = vd->get_height();
    this->glContexID = glContexID;

}

