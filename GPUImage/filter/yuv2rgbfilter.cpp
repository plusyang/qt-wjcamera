#include "yuv2rgbfilter.h"
#include "../Context.hpp"

#include <QDebug>

USING_NS_GI



static const std::string yuv2rgbFragmentShaderString = SHADER_STRING
(
	precision mediump float;
    uniform sampler2D tex_y;
    uniform sampler2D tex_uv;
    varying vec2 vTexCoord;
    void main()
    {
      vec3 yuv;
      vec3 rgb; 
      //convert formula come from http://www.fourcc.org/fccyvrgb.php
      yuv.x = (texture2D(tex_y, vTexCoord.st).r - 16./255.) * 1.164;
      yuv.y = texture2D(tex_uv, vTexCoord.st).r - 128./255.;
      yuv.z = texture2D(tex_uv, vTexCoord.st).g - 128./255.;
      rgb = mat3(1.0,      1.0,     1.0,
                 0.0,      -0.391,   2.018,
                 1.596,    -0.813 ,   0.0)*yuv;
      gl_FragColor = vec4(rgb,1.0);
    }
);

Yuv2RgbFilter* Yuv2RgbFilter::create(int contexID) {
    Yuv2RgbFilter* ret = new (std::nothrow) Yuv2RgbFilter();
    ret->_contexID = contexID;
    if (ret && !ret->init()) {
        delete ret;
        ret = 0;
    }
    return ret;
}

Yuv2RgbFilter::~Yuv2RgbFilter()
{
    glDeleteTextures(2, yuv_tex);
}

bool Yuv2RgbFilter::init()
{
    if (!initWithFragmentShaderString(yuv2rgbFragmentShaderString)) return false;
//    qDebug()<<"init program ID:"<<_filterProgram->getID();

    glGenTextures(2, yuv_tex);
    for (int i=0;i<2;i++) {
        glActiveTexture(GL_TEXTURE0+i);
        glBindTexture ( GL_TEXTURE_2D, yuv_tex[i] );
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    }
    glBindTexture ( GL_TEXTURE_2D, 0 );

    glUseProgram(_filterProgram->getID());
    glUniform1i(glGetUniformLocation(_filterProgram->getID(), "tex_y"), 0);
    glUniform1i(glGetUniformLocation(_filterProgram->getID(), "tex_uv"), 1);
    glUseProgram(0);

    return true;
}

bool Yuv2RgbFilter::proceed(bool bUpdateTargets/* = true*/ )
{
    static const GLfloat imageVertices[] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f,  1.0f,
        1.0f,  1.0f,
    };

    _framebuffer->active();

    CHECK_GL(glClearColor(_backgroundColor.r, _backgroundColor.g, _backgroundColor.b, _backgroundColor.a));
    CHECK_GL(glClear(GL_COLOR_BUFFER_BIT));
    Context::getInstance()->setActiveShaderProgram(_filterProgram);

//    qDebug()<<"proceed framebuffer: "<<_framebuffer->getFramebuffer();
//    qDebug()<<"proceed program ID:"<<_filterProgram->getID();

    std::map<int, InputFrameBufferInfo>::const_iterator it = _inputFramebuffers.begin();
    Framebuffer* fb = it->second.frameBuffer;

    unsigned char *yuv_data = fb->getYuvData();
    int width = fb->getWidth();
    int height = fb->getHeight();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture ( GL_TEXTURE_2D, yuv_tex[0] );
    glTexImage2D ( GL_TEXTURE_2D,0, GL_RED,width,height,0, GL_RED,GL_UNSIGNED_BYTE, yuv_data);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture ( GL_TEXTURE_2D, yuv_tex[1] );
    glTexImage2D ( GL_TEXTURE_2D,0, GL_RG,width/2,height/2,0, GL_RG,GL_UNSIGNED_BYTE, yuv_data+width*height );

    // texcoord attribute
    GLuint filterTexCoordAttribute = _filterProgram->getAttribLocation("texCoord");
    CHECK_GL(glEnableVertexAttribArray(filterTexCoordAttribute));
    CHECK_GL(glVertexAttribPointer(filterTexCoordAttribute, 2, GL_FLOAT, 0, 0, _getTexureCoordinate(it->second.rotationMode)));

    CHECK_GL(glVertexAttribPointer(_filterPositionAttribute, 2, GL_FLOAT, 0, 0, imageVertices));
    CHECK_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

    _framebuffer->inactive();
    return Source::proceed(bUpdateTargets);


}
