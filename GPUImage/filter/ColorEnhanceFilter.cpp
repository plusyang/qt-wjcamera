#include "ColorEnhanceFilter.h"
#include <QDebug>


USING_NS_GI
ColorEnhanceFilter::~ColorEnhanceFilter()
{
    if (_hsbFilter) {
        _hsbFilter->release();
        _hsbFilter=0;
    }

    // if (_contrastFilter) {
    //     _contrastFilter->release();
    //     _contrastFilter = 0;
    // }
    if (_rgbFilter) {
        _rgbFilter->release();
        _rgbFilter = 0;
    }
}

ColorEnhanceFilter* ColorEnhanceFilter::create(int contexID)
{
    ColorEnhanceFilter* ret = new (std::nothrow) ColorEnhanceFilter();
    ret->_contexID = contexID;
    if (ret && !ret->init()) {
       delete ret;
       ret = 0;
    }
    return ret;
}

bool ColorEnhanceFilter::init()
{
    if (!FilterGroup::init()) {
       return false;
    }

    //_contrastFilter = ContrastFilter::create(_contexID);
    //_contrastFilter->setContrast(1.2,1.2,1.2);
    //addFilter(_contrastFilter);
    
    _rgbFilter = RGBFilter::create(_contexID);
    addFilter(_rgbFilter);
    
    _hsbFilter = HSBFilter::create();
    _hsbFilter->adjustBrightness(0.8);
    _hsbFilter->adjustSaturation(1.3);

    _rgbFilter->addTarget(_hsbFilter);
    setTerminalFilter(_hsbFilter);

    return true;
}

bool ColorEnhanceFilter::proceed(bool bUpdateTargets)
{
    return FilterGroup::proceed(bUpdateTargets);
}

void ColorEnhanceFilter::setRGBContrast(float contrast_r,float contrast_g,float contrast_b)
{
    _contrastFilter->setContrast(contrast_r,contrast_g,contrast_b);
}

void ColorEnhanceFilter::setRGBAdjust(float r,float g,float b)
{
    _rgbFilter->setRedAdjustment(r);
    _rgbFilter->setGreenAdjustment(g);
    _rgbFilter->setBlueAdjustment(b);
}

void ColorEnhanceFilter::setHsbAttrib(float brightness,float saturation)
{
    _hsbFilter->adjustBrightness(brightness);
    _hsbFilter->adjustSaturation(saturation);
}



/*
/*
 * GPUImage-x
 *
 * Copyright (C) 2017 Yijin Wang, Yiqian Wang
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.

#include "ContrastFilter.hpp"
#include "Context.hpp"

USING_NS_GI

REGISTER_FILTER_CLASS(ContrastFilter)

// ----------------------------------------//
//
//there has laplace filter conv , and rgb2hsv, ...
//
//--------------------------------------------//

const std::string kContrastFragmentShaderString = SHADER_STRING
(
    varying vec2 vTexCoord;
    uniform sampler2D colorMap;
    uniform float kernel[9];
    uniform float imageWidth;
    uniform float imageHeight;

    uniform  float contrast;

    vec3 rgb2hsv(vec3 c)
    {
        vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        float d = q.x - min(q.w, q.y);
        float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }

    vec3 hsv2rgb(vec3 c)
    {
        vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }



    void main()
    {
        float w = 1.0 / imageWidth;
        float h = 1.0 / imageHeight;
        vec2 texturecoord[9];
        texturecoord[4] = vTexCoord.st;  //0 0
        texturecoord[5] = texturecoord[4]+vec2(w,0.0); // 1 0
        texturecoord[2] = texturecoord[5]-vec2(0.0,h); // 1 -1
        texturecoord[1] = texturecoord[2]-vec2(w,0.0); // 0 -1
        texturecoord[0] = texturecoord[1]-vec2(w,0.0); // -1 -1
        texturecoord[3] = texturecoord[0]+vec2(0.0,h); // -1 0
        texturecoord[6] = texturecoord[3]+vec2(0.0,h); // -1 1
        texturecoord[7] = texturecoord[6]+vec2(w,0.0); // 0 1
        texturecoord[8] = texturecoord[7]+vec2(w,0.0); // 1 1

        int i;
        vec3 sum = vec3(0.0);
        for(i=0;i<9;i++){
            vec4 color = texture2D(colorMap,texturecoord[i]);
            vec3 hsv_color = rgb2hsv(color.rgb);

            hsv_color.rgb = hsv_color.rgb * kernel[i];
            hsv_color.b = (hsv_color.b-0.5)*contrast + 0.5;//...

            vec3 rgb_color = hsv2rgb(hsv_color);

            rgb_color.rgb = rgb_color.rgb * kernel[i];

            sum += rgb_color;


        }

      gl_FragColor = vec4(sum,1.0);
    }
);


ContrastFilter* ContrastFilter::create() {
    ContrastFilter* ret = new (std::nothrow) ContrastFilter();
    if (ret && !ret->init()) {
        delete ret;
        ret = 0;
    }
    return ret;
}

bool ContrastFilter::init() {
    if (!initWithFragmentShaderString(kContrastFragmentShaderString)) return false;

    Context::getInstance()->setActiveShaderProgram(_filterProgram);
    _filterTexCoordAttribute = _filterProgram->getAttribLocation("vTexCoord");
    CHECK_GL(glEnableVertexAttribArray(_filterTexCoordAttribute));

    return true;
}

void ContrastFilter::setContrast(float contrast) {
    _contrast = contrast;
    if (_contrast > 4.0) _contrast = 4.0;
    else if (_contrast < 0.0) _contrast = 0.0;
}

bool ContrastFilter::proceed(bool bUpdateTargets) {
    static const GLfloat imageVertices[] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f,  1.0f,
        1.0f,  1.0f,
    };

    float kernel[9] = {
        0.0, -1.0, 0.0,
        -1.0, 4.0, -1.0,
        0.0, -1.0, 0.0
    };

    _framebuffer->active();
    CHECK_GL(glClearColor(_backgroundColor.r, _backgroundColor.g, _backgroundColor.b, _backgroundColor.a));
    CHECK_GL(glClear(GL_COLOR_BUFFER_BIT));
    Context::getInstance()->setActiveShaderProgram(_filterProgram);
    std::map<int, InputFrameBufferInfo>::const_iterator it = _inputFramebuffers.begin();
    Framebuffer* fb = it->second.frameBuffer;

    GLuint kernelLocation = glGetUniformLocation(_filterProgram->getID(),"kernel");
    glUniform1fv(kernelLocation,9,kernel);

    _filterProgram->setUniformValue("contrast", _contrast);
    if((it->second.rotationMode)==RotateLeft || (it->second.rotationMode)==RotateRight ||
           (it->second.rotationMode)==RotateRightFlipVertical || (it->second.rotationMode)==RotateRightFlipHorizontal){
        _filterProgram->setUniformValue("imageWidth", (float)fb->getHeight());
        _filterProgram->setUniformValue("imageHeight", (float)fb->getWidth());
    }else{
        _filterProgram->setUniformValue("imageWidth", (float)fb->getWidth());
        _filterProgram->setUniformValue("imageHeight", (float)fb->getHeight());
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture ( GL_TEXTURE_2D, fb->getTexture());

    _filterProgram->setUniformValue("colorMap", 0);

    CHECK_GL(glVertexAttribPointer(_filterPositionAttribute, 2, GL_FLOAT, 0, 0, imageVertices));
    CHECK_GL(glVertexAttribPointer(_filterTexCoordAttribute, 2, GL_FLOAT, 0, 0, _getTexureCoordinate(it->second.rotationMode)));

    CHECK_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

    _framebuffer->inactive();

    return Source::proceed(bUpdateTargets);
}

*/

