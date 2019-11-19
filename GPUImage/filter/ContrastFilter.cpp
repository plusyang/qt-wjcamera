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
 */

#include "ContrastFilter.hpp"

USING_NS_GI

REGISTER_FILTER_CLASS(ContrastFilter)

const std::string kContrastFragmentShaderString = SHADER_STRING
(
    uniform sampler2D colorMap;
    uniform lowp float contrast_r;
    uniform lowp float contrast_g;
    uniform lowp float contrast_b;
    varying highp vec2 vTexCoord;
    void main()
    {
        lowp vec4 color = texture2D(colorMap, vTexCoord);
        lowp vec3 cc = (vec3(color.rgb - vec3(0.5)) * vec3(contrast_r,contrast_g,contrast_b)) + vec3(0.5);
        gl_FragColor = vec4(contrast_r,contrast_g,contrast_b, color.a);
    }
);


ContrastFilter* ContrastFilter::create(int contexID) {
    ContrastFilter* ret = new (std::nothrow) ContrastFilter();
    ret->_contexID = contexID;
    if (ret && !ret->init()) {
        delete ret;
        ret = 0;
    }
    return ret;
}

bool ContrastFilter::init() {
    if (!initWithFragmentShaderString(kContrastFragmentShaderString)) return false;

    _contrast_r = 1.0;
    _contrast_g = 1.0;
    _contrast_b = 1.0;
    //setContrast(_contrast_r,_contrast_g,_contrast_b);
    // registerProperty("contrast", _contrast, "The contrast of the image. Contrast ranges from 0.0 to 4.0 (max contrast), with 1.0 as the normal level", [this](float& contrast){
    //     setContrast(_contrast_r,_contrast_g,_contrast_b);
    // });

    return true;
}

void ContrastFilter::setContrast(float contrast_r,float contrast_g,float contrast_b) {
    _contrast_r = contrast_r;
    _contrast_g = contrast_g;
    _contrast_b = contrast_b;
    if (_contrast_r > 4.0) _contrast_r = 4.0;
    else if (_contrast_r < 0.0) _contrast_r = 0.0;
    if (_contrast_g > 4.0) _contrast_g = 4.0;
    else if (_contrast_g < 0.0) _contrast_g = 0.0;
    if (_contrast_b > 4.0) _contrast_b = 4.0;
    else if (_contrast_b < 0.0) _contrast_b = 0.0;
//    printf("_contrast_r = %f,_contrast_g = %f,_contrast_b = %f\n",_contrast_r,_contrast_g,_contrast_b);
}

bool ContrastFilter::proceed(bool bUpdateTargets/* = true*/) {
    _filterProgram->setUniformValue("contrast_r", _contrast_r);
    _filterProgram->setUniformValue("contrast_g", _contrast_g);
    _filterProgram->setUniformValue("contrast_b", _contrast_b);
    return Filter::proceed(bUpdateTargets);
}

