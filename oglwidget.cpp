/****************************************
* 类名：oglWidget :gl图像窗口
* 功能：
* gl画图
* 作者：冯鹏
* 日期：20181120
* 版本：2.1
*****************************************/

#include "oglwidget.h"
#include "common.h"
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QCoreApplication>
#include <QOpenGLTexture>
#include <QSemaphore>

#define VERTEXIN 0
#define TEXTUREIN 1
#define camVideoDev  "/dev/video0"
#define YUV420P2RGB

extern QSemaphore sem;

oglWidget::oglWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    program(0),loopCnt(0)
{
    //TODO remove
    closeBtn = new QPushButton("返回",this);
    closeBtn->setGeometry(0,0,40,20);
    closeBtn->hide();

    videoW=video_width = 1280;
    videoH=video_height = 720;

    p420_data=new unsigned char[(1280*720*3)>>1];

    qDebug()<<"oglWidget create done.";
}


oglWidget::~oglWidget()
{
    program->disableAttributeArray("vertexIn");
    program->disableAttributeArray("textureIn");
    program->release();
    qDebug()<<"program release success..";

    makeCurrent();
    vbo.destroy();
    doneCurrent();
    qDebug()<<"vbo.destroy success..";

    delete[] p420_data;
}

void oglWidget::setVD(VideoDevice *vd)
{
    this->vd=vd;
}

/*******yuv420sp(NV12) 转换为 yuv420p *******/
void oglWidget::yuv420sp_to_yuv420p(unsigned char* yuv420sp, unsigned char* yuv420p, int width, int height)
{
    int i, j;
    int y_size = width * height;

    unsigned char* y = yuv420sp;
    unsigned char* uv = yuv420sp + y_size;

    unsigned char* y_tmp = yuv420p;
    unsigned char* u_tmp = yuv420p + y_size;
    unsigned char* v_tmp = yuv420p + y_size * 5 / 4;

    // y
    memcpy(y_tmp, y, y_size);

    // u
    for (j = 0, i = 0; j < y_size/2; j+=2, i++)
    {
        u_tmp[i] = uv[j];
        v_tmp[i] = uv[j+1];
    }
}


void oglWidget::cleanup()
{
   //如果全部加上，本来析构的时候有问题
   // makeCurrent();
    //m_logoVbo.destroy();
   // delete program;
    //m_program = 0;
   // doneCurrent();
}

QSize oglWidget::sizeHint() const
{
    return QSize(1280, 720);
}

void oglWidget::initializeGL()
{
    initializeOpenGLFunctions();
    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex,this);
    const char *vsrc =
           "attribute vec4 vertexIn; \
            attribute vec4 textureIn; \
            varying vec4 textureOut;  \
            void main(void)           \
            {                         \
                gl_Position = vertexIn; \
                textureOut = textureIn; \
            }";
    vshader->compileSourceCode(vsrc);

    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment,this);

#ifdef YUV420P2RGB
    const char *fsrc =
            "precision mediump float;\n"
            "uniform sampler2D tex_y;					\n"
            "uniform sampler2D tex_u;					\n"
            "uniform sampler2D tex_v;					\n"
            "varying vec4 textureOut;							\n"
            "void main()                                  \n"
            "{                                            \n"
            "  vec4 c = vec4((texture2D(tex_y, textureOut.st).r - 16./255.) * 1.164);\n"
            "  vec4 U = vec4(texture2D(tex_u, textureOut.st).r - 128./255.);\n"
            "  vec4 V = vec4(texture2D(tex_v, textureOut.st).r - 128./255.);\n"
            "  c += V * vec4(1.596, -0.813, 0, 0);\n"
            "  c += U * vec4(0, -0.392, 2.017, 0);\n"
            "  c.a = 1.0;\n"
            "  gl_FragColor = c;\n"
            "}                                            \n";
#else
    //YUV420SP(NV12)转RGB
    const char *fsrc =
				"precision mediump float;\
                uniform sampler2D textureY;\
                uniform sampler2D textureUV;\
				varying vec4 textureOut;\
                void main(void)\
                { \
                    vec3 yuv; \
                    vec3 rgb; \
                    yuv.x = texture2D(textureY, textureOut.st).r - 0.0625; \
                    yuv.y = texture2D(textureUV, textureOut.st).r - 0.5; \
                    yuv.z = texture2D(textureUV, textureOut.st).g - 0.5; \
                    rgb = mat3(1.0,       1.0,       1.0, \
                                0.0,       -0.39465,  2.03211, \
                                1.13983,   -0.58060,  0.0) * yuv; \
                    gl_FragColor = vec4(rgb, 1.0); \
                }";
#endif
    fshader->compileSourceCode(fsrc);

    program = new QOpenGLShaderProgram(this);
    program->addShader(vshader);
    program->addShader(fshader);
    program->link();

    GLfloat points[]{
       -1.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, -1.0f,
       -1.0f, -1.0f,

       0.0f,0.0f,
       1.0f,0.0f,
       1.0f,1.0f,
       0.0f,1.0f
    };

    vbo.create();
    vbo.bind();
    vbo.allocate(points,sizeof(points));

    program->enableAttributeArray("vertexIn");
    program->enableAttributeArray("textureIn");
    program->setAttributeBuffer("vertexIn",GL_FLOAT, 0, 2, 2*sizeof(GLfloat));
    program->setAttributeBuffer("textureIn",GL_FLOAT, 2 * 4 * sizeof(GLfloat),2,2*sizeof(GLfloat));

    program->bind();

#ifdef YUV420P2RGB
    //读取着色器中的数据变量tex_y, tex_u, tex_v的位置,这些变量的声明可以在
             //片段着色器源码中可以看到
    textureUniformY = program->uniformLocation("tex_y");
    textureUniformU =  program->uniformLocation("tex_u");
    textureUniformV =  program->uniformLocation("tex_v");

    //分别创建y,u,v纹理对象
    m_pTextureY = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_pTextureU = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_pTextureV = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_pTextureY->create();
    m_pTextureU->create();
    m_pTextureV->create();
    //获取返回y分量的纹理索引值
    id_y = m_pTextureY->textureId();
    //获取返回u分量的纹理索引值
    id_u = m_pTextureU->textureId();
    //获取返回v分量的纹理索引值
    id_v = m_pTextureV->textureId();
    glClearColor(0.3,0.3,0.3,0.0);//设置背景色
#else
    textureUniformY = program->uniformLocation("textureY");
    textureUniformUV =  program->uniformLocation("textureUV");
    // //分别创建y,uv纹理对象
    // m_pTextureY = new QOpenGLTexture(QOpenGLTexture::Target2D);
    // m_pTextureUV = new QOpenGLTexture(QOpenGLTexture::Target2D);
    // m_pTextureY->create();
    // m_pTextureUV->create();

    // //获取返回y分量的纹理索引值
    // id_y = m_pTextureY->textureId();
    // //获取返回uv分量的纹理索引值
    // id_uv = m_pTextureUV->textureId();
    GLuint ids[2];
    glGenTextures(2, ids);
    id_y = ids[0];
    id_uv = ids[1];
#endif
}


void oglWidget::paintGL()
{
    glClearColor(0.5f, 0.5f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    //glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB );
    int width ;
    int	height ;

    width = videoW;
    height = videoH;
#ifdef YUV420P2RGB
    glActiveTexture(GL_TEXTURE0);
    glBindTexture ( GL_TEXTURE_2D, id_y );
    glTexImage2D ( GL_TEXTURE_2D,
                   0, GL_LUMINANCE,
                   width, height,
                   0, GL_LUMINANCE,
                   GL_UNSIGNED_BYTE, p420_data );

    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    glActiveTexture(GL_TEXTURE1);
    glBindTexture ( GL_TEXTURE_2D, id_u );
    glTexImage2D ( GL_TEXTURE_2D,
                   0, GL_LUMINANCE,
                   width / 2, height / 2,
                   0, GL_LUMINANCE,
                   GL_UNSIGNED_BYTE, p420_data + videoW * videoH);

    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    glActiveTexture(GL_TEXTURE2);
    glBindTexture ( GL_TEXTURE_2D, id_v );
    glTexImage2D ( GL_TEXTURE_2D,
                   0, GL_LUMINANCE,
                   width / 2, height / 2,
                   0, GL_LUMINANCE,
                   GL_UNSIGNED_BYTE, p420_data + videoW * videoH * 5 / 4 );

    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    glUniform1i(textureUniformY, 0);
    //指定u纹理要使用新值
    glUniform1i(textureUniformU, 1);
    //指定v纹理要使用新值
    glUniform1i(textureUniformV, 2);
#else
    glActiveTexture(GL_TEXTURE0);
    glBindTexture ( GL_TEXTURE_2D, id_y );
    glTexImage2D ( GL_TEXTURE_2D,
                   0, GL_RED,
                   width, height,
                   0, GL_RED,
                   GL_UNSIGNED_BYTE, readThread->yuv_buf);
					
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    glActiveTexture(GL_TEXTURE1);
    glBindTexture ( GL_TEXTURE_2D, id_uv );
    glTexImage2D ( GL_TEXTURE_2D,
                   0, GL_RG,
                   width >> 1, height >> 1,
                   0, GL_RG,
                   GL_UNSIGNED_BYTE, readThread->yuv_buf + videoW * videoH);

    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    glUniform1i(textureUniformY, 0);
    //指定uv纹理要使用新值
    glUniform1i(textureUniformUV, 1);
    // program->setUniformValue(textureUniformY, 0);
    // program->setUniformValue(textureUniformUV, 1);
#endif

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    QMetaObject::invokeMethod(this,"update_ss",Qt::QueuedConnection);

}

void oglWidget::update_ss()
{
    //qDebug()<<"oglWidget::update_ss update a frame.";
    
    if(vd == NULL){
        return;
    }

    #ifdef YUV420P2RGB
    //将NV12格式转化为YUV420P
    yuv420sp_to_yuv420p(vd->yuv_buffer,p420_data,1280,720);
    #else
    //p420_data = readThread->yuv_buf;
    #endif
    update();
    vd->osd_do();

    sem.release(); //modify by yzh, fixed thread synchronization, use QSemaphore
    

}

void oglWidget::resizeGL(int width, int height)
{
     glViewport(0, 0, width, height);
}
