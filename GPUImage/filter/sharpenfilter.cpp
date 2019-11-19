#include "sharpenfilter.h"
#include "Context.hpp"

USING_NS_GI


static const std::string sharpenVertexShaderString = SHADER_STRING
(
    attribute vec4 position;
    attribute vec4 inputTextureCoordinate;

    uniform float imageWidthFactor;
    uniform float imageHeightFactor;
    uniform float sharpness;

    varying vec2 textureCoordinate;
    varying vec2 leftTextureCoordinate;
    varying vec2 rightTextureCoordinate;
    varying vec2 topTextureCoordinate;
    varying vec2 bottomTextureCoordinate;

    varying float centerMultiplier;
    varying float edgeMultiplier;

    void main()
    {
         gl_Position = position;

         vec2 widthStep = vec2(imageWidthFactor, 0.0);
         vec2 heightStep = vec2(0.0, imageHeightFactor);

         textureCoordinate = inputTextureCoordinate.xy;
         leftTextureCoordinate = inputTextureCoordinate.xy - widthStep;
         rightTextureCoordinate = inputTextureCoordinate.xy + widthStep;
         topTextureCoordinate = inputTextureCoordinate.xy + heightStep;
         bottomTextureCoordinate = inputTextureCoordinate.xy - heightStep;

         centerMultiplier = 1.0 + 4.0 * sharpness;
         edgeMultiplier = sharpness;
    }
);

static const std::string sharpenFragmentShaderString = SHADER_STRING
(
	precision highp float;
 
	varying highp vec2 textureCoordinate;
	varying highp vec2 leftTextureCoordinate;
	varying highp vec2 rightTextureCoordinate; 
	varying highp vec2 topTextureCoordinate;
	varying highp vec2 bottomTextureCoordinate;
 
	varying highp float centerMultiplier;
	varying highp float edgeMultiplier;

	uniform sampler2D inputImageTexture;
 
 void main()
 {
	mediump vec3 textureColor = texture2D(inputImageTexture, textureCoordinate).rgb;
	mediump vec3 leftTextureColor = texture2D(inputImageTexture, leftTextureCoordinate).rgb;
	mediump vec3 rightTextureColor = texture2D(inputImageTexture, rightTextureCoordinate).rgb;
	mediump vec3 topTextureColor = texture2D(inputImageTexture, topTextureCoordinate).rgb;
	mediump vec3 bottomTextureColor = texture2D(inputImageTexture, bottomTextureCoordinate).rgb;

    gl_FragColor = vec4((textureColor * centerMultiplier - (leftTextureColor * edgeMultiplier + rightTextureColor * edgeMultiplier + topTextureColor * edgeMultiplier + bottomTextureColor * edgeMultiplier)), texture2D(inputImageTexture, bottomTextureCoordinate).w);
 }
);

SharpenFilter* SharpenFilter::create(int contexID, float sharpness)
{
    SharpenFilter* ret = new (std::nothrow) SharpenFilter();
    ret->_contexID = contexID;
    if (ret && !ret->init(sharpness)) {
        delete ret;
        ret = 0;
    }
    return ret;
}

bool SharpenFilter::init(float sharpness)
{
    if(!initWithShaderString(sharpenVertexShaderString,sharpenFragmentShaderString)) return false;
    _sharpness = sharpness;

    Context::getInstance()->setActiveShaderProgram(_filterProgram);
    _filterTexCoordAttribute = _filterProgram->getAttribLocation("inputTextureCoordinate");
    CHECK_GL(glEnableVertexAttribArray(_filterTexCoordAttribute));

    return true;
}

void SharpenFilter::setParamters(float sharpness)
{
    _sharpness = sharpness;
//    if (_sharpness > 1.0) _sharpness = 1.0;
//    else if (_sharpness < -1.0) _sharpness = -1.0;
}

bool SharpenFilter::proceed(bool bUpdateTargets)
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

    _filterProgram->setUniformValue("sharpness", _sharpness);
    if((it->second.rotationMode)==RotateLeft || (it->second.rotationMode)==RotateRight ||
           (it->second.rotationMode)==RotateRightFlipVertical || (it->second.rotationMode)==RotateRightFlipHorizontal){
        _filterProgram->setUniformValue("imageWidthFactor", (float)(1.0 / fb->getHeight()));
        _filterProgram->setUniformValue("imageHeightFactor", (float)(1.0 / fb->getWidth()));
    }else{
        _filterProgram->setUniformValue("imageWidthFactor", (float)(1.0 / fb->getWidth()));
        _filterProgram->setUniformValue("imageHeightFactor", (float)(1.0 / fb->getHeight()));
    }
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture ( GL_TEXTURE_2D, fb->getTexture());

    _filterProgram->setUniformValue("inputImageTexture", 0);

    CHECK_GL(glVertexAttribPointer(_filterPositionAttribute, 2, GL_FLOAT, 0, 0, imageVertices));
    CHECK_GL(glVertexAttribPointer(_filterTexCoordAttribute, 2, GL_FLOAT, 0, 0, _getTexureCoordinate(it->second.rotationMode)));

    CHECK_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

    _framebuffer->inactive();

    return Source::proceed(bUpdateTargets);
}
