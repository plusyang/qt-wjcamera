#ifndef OGLWIDGET_H
#define OGLWIDGET_H

#include <QFile>
#include <QTimer>
#include <QPushButton>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include "videodevice.h"

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)
QT_FORWARD_DECLARE_CLASS(QOpenGLTexture)

namespace Ui {
class oglWidget;
}

class oglWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    oglWidget(QWidget *parent = 0);
    ~oglWidget();
    void yuv420sp_to_yuv420p(unsigned char* yuv420sp, unsigned char* yuv420p, int width, int height);
    //图像数据加载函数LoadData
//    void LoadData(unsigned char* imagedata,int width,int height);
//    void on_timeout();
    
//    void NV12ToYuv420P(unsigned char* nv12,unsigned char* yuv420p,int width,int height);

    void setVD(VideoDevice *vd);
protected:
    void paintGL() Q_DECL_OVERRIDE;
    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int width, int height) Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;
    void cleanup();
    //void createTexture(int width, int height, int format, int* textureId);
    void dealOSD(QString osd);

public slots:
    void update_ss();

private:
    Ui::oglWidget *ui;
    int video_width;
    int video_height;

    // 单帧图像数据指针
    unsigned char* yuv_buffer_pointer;
    unsigned char* p420_data;

    VideoDevice *vd;

    QTimer *timer;

    QOpenGLShaderProgram *program;
    QOpenGLBuffer vbo;
    GLuint textureUniformY,textureUniformU,textureUniformV; //opengl中y、u、v分量位置
    GLuint textureUniformUV;	//opengl中uv分量位置

    GLuint id_y; //y纹理对象ID
    GLuint id_u; //u纹理对象ID
    GLuint id_v; //v纹理对象ID
    GLuint id_uv; //uv纹理对象ID

    QOpenGLTexture* m_pTextureY;  //y纹理对象
    QOpenGLTexture* m_pTextureU;  //u纹理对象
    QOpenGLTexture* m_pTextureV;  //v纹理对象
	QOpenGLTexture* m_pTextureUV;  //uv纹理对象
    uint videoW,videoH;
	int time_interval;

    long loopCnt;

public:
    //
    QPushButton *closeBtn;
};


#endif // OGLWIDGET_H
