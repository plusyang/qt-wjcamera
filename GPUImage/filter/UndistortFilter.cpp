#include "UndistortFilter.h"
#include "Context.hpp"

USING_NS_GI

const static std::string kUndistortFragmentShader = SHADER_STRING
(
 precision mediump float;
 varying highp vec2 vTexCoord;
 uniform sampler2D colorMap;
 uniform int enable;
 void main(){
     highp vec2 distortion_center = vec2(0.5, 0.5);
     float distortion_k1 = -0.278667;
     float distortion_k2 = 0.067252;
     float distortion_x;
     float distortion_y;
     float rr;
     float r2;
     float theta;
     highp vec2 dest_uv;
     rr = sqrt((vTexCoord.x - distortion_center.x) * (vTexCoord.x - distortion_center.x) + (vTexCoord.y - distortion_center.y) * (vTexCoord.y - distortion_center.y));
     r2 = rr * (1.0 + distortion_k1*(rr*rr) + distortion_k2*(rr*rr*rr*rr));
     theta = atan(vTexCoord.x - distortion_center.x, vTexCoord.y - distortion_center.y);
     distortion_x = sin(theta) * r2 * 1.0;
     distortion_y = cos(theta) * r2 * 1.0;
     dest_uv.x = distortion_x + 0.5;
     dest_uv.y = distortion_y + 0.5;

     if(enable==1){
        gl_FragColor = vec4(texture2D(colorMap, dest_uv).r, texture2D(colorMap, dest_uv).g, texture2D(colorMap, dest_uv).b, 1.);
     }else{
        gl_FragColor = texture2D(colorMap,vTexCoord);
     }

    }
);


UndistortFilter* UndistortFilter::create(int contexID)
{
    UndistortFilter* ret = new (std::nothrow) UndistortFilter();
    ret->_contexID = contexID;
    if (ret && !ret->init()) {
        delete ret;
        ret = 0;
    }
    return ret;
}

bool UndistortFilter::init()
{
    if (!initWithFragmentShaderString(kUndistortFragmentShader)) return false;
    Context::getInstance()->setActiveShaderProgram(_filterProgram);
    _colorMap_location = _filterProgram->getUniformLocation("colorMap");
    _enable = 1;
    return true;
}

void UndistortFilter::setEnable(bool able)
{
    if(able){
        _enable = 1;
    }else{
        _enable = 0;
    }
}

bool UndistortFilter::proceed(bool bUpdateTargets)
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
    std::map<int, InputFrameBufferInfo>::const_iterator it = _inputFramebuffers.begin();
    Framebuffer* fb = it->second.frameBuffer;
    _filterProgram->setUniformValue("enable", _enable);

    CHECK_GL(glActiveTexture(GL_TEXTURE0));
    CHECK_GL(glBindTexture ( GL_TEXTURE_2D, fb->getTexture()));
    CHECK_GL(glUniform1i(_colorMap_location, 0));

    CHECK_GL(glVertexAttribPointer(_filterPositionAttribute, 2, GL_FLOAT, 0, 0, imageVertices));
    // texcoord attribute
    GLuint filterTexCoordAttribute = _filterProgram->getAttribLocation("texCoord");
    CHECK_GL(glEnableVertexAttribArray(filterTexCoordAttribute));
    CHECK_GL(glVertexAttribPointer(filterTexCoordAttribute, 2, GL_FLOAT, 0, 0, _getTexureCoordinate(it->second.rotationMode)));

    CHECK_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

    _framebuffer->inactive();

    return Source::proceed(bUpdateTargets);
}
