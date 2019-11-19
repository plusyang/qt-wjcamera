#include "LastMaskFilter.h"
#include "Context.hpp"


USING_NS_GI

const static std::string kLastMaskFragmentShader = SHADER_STRING
(
 varying highp vec2 vTexCoord;
 uniform sampler2D colorMap;
 uniform sampler2D maskMap;
 uniform int enable;
 void main(){
     if(enable == 1){
         gl_FragColor = texture2D(colorMap, vTexCoord)*texture2D(maskMap, vTexCoord).a;
     }else{
         gl_FragColor = texture2D(colorMap, vTexCoord);
     }  
 }
);

LastMaskFilter* LastMaskFilter::create(int contexID) {
    LastMaskFilter* ret = new (std::nothrow) LastMaskFilter();
    ret->_contexID = contexID;
    if (ret && !ret->init()) {
        delete ret;
        ret = 0;
    }
    return ret;
}

LastMaskFilter::~LastMaskFilter()
{
    glDeleteTextures(2, &_maskMap_textureID);
}

bool LastMaskFilter::init() {
    if (!initWithFragmentShaderString(kLastMaskFragmentShader)) return false;
    Context::getInstance()->setActiveShaderProgram(_filterProgram);
    _colorMap_location = _filterProgram->getUniformLocation("colorMap");
    _maskMap_location = _filterProgram->getUniformLocation("maskMap");
    _enable = 1;
    return true;
}

void LastMaskFilter::setEnable(bool able)
{
    if(able){
        _enable = 1;
    }else{
        _enable = 0;
    }
}

bool LastMaskFilter::proceed(bool bUpdateTargets/* = true*/) {
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
    std::map<int, InputFrameBufferInfo>::const_iterator it = _inputFramebuffers.begin();
    Framebuffer* fb = it->second.frameBuffer;
    _filterProgram->setUniformValue("enable", _enable);

    CHECK_GL(glActiveTexture(GL_TEXTURE0));
    CHECK_GL(glBindTexture ( GL_TEXTURE_2D, fb->getTexture()));
    CHECK_GL(glUniform1i(_colorMap_location, 0));

    CHECK_GL(glActiveTexture(GL_TEXTURE1));
    CHECK_GL(glBindTexture(GL_TEXTURE_2D, _maskMap_textureID));
    CHECK_GL(glUniform1i(_maskMap_location, 1));

    CHECK_GL(glVertexAttribPointer(_filterPositionAttribute, 2, GL_FLOAT, 0, 0, imageVertices));
    // texcoord attribute
    GLuint filterTexCoordAttribute = _filterProgram->getAttribLocation("texCoord");
    CHECK_GL(glEnableVertexAttribArray(filterTexCoordAttribute));
    CHECK_GL(glVertexAttribPointer(filterTexCoordAttribute, 2, GL_FLOAT, 0, 0, _getTexureCoordinate(it->second.rotationMode)));

    CHECK_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

    _framebuffer->inactive();

    return Source::proceed(bUpdateTargets);
}


void LastMaskFilter::setMaskPixel(int width,int height, unsigned char* pixel)
{
    CHECK_GL(glGenTextures(1, &_maskMap_textureID));
    CHECK_GL(glBindTexture(GL_TEXTURE_2D, _maskMap_textureID));
    CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    CHECK_GL(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,pixel));

    CHECK_GL(glBindTexture(GL_TEXTURE_2D, 0));
}
