#include "gpuimage_params.h"

GPUImage_Params* GPUImage_Params::_instance = 0;

GPUImage_Params::GPUImage_Params()
{
    this->_brightness = 0;
    this->_shapness = 0;
    this->_bilateral_multiplier = 0;
    this->_bilateral_normalization = 8;
    this->_rotation = 0;
    this->_zoom = 1.0;

    this->_color_r = 1.0;
    this->_color_g = 1.0;
    this->_color_b = 1.0;
}

GPUImage_Params::~GPUImage_Params()
{

}

GPUImage_Params* GPUImage_Params::getInstance()
{
    if(!_instance){
        if(!_instance){
            _instance = new GPUImage_Params();
        }
    }
    return _instance;
}


void GPUImage_Params::setBrightness(float brightness)
{
    this->_brightness=brightness;
}

void GPUImage_Params::setSharpness(float shapness)
{
    this->_shapness = shapness;
}

void GPUImage_Params::setBilateralParams(float multiplier, float normalization)
{
    this->_bilateral_multiplier = multiplier;
    this->_bilateral_normalization = normalization;
}

void GPUImage_Params::setColorEnhanceParams(float color_r, float color_g, float color_b)
{
    this->_color_r = color_r;
    this->_color_g = color_g;
    this->_color_b = color_b;
}

void GPUImage_Params::setRotation(ROTATION_MODE rotation)
{
    this->_rotation = rotation;
}

void GPUImage_Params::setZoom(ZOOM_FACTOR zoomFactor)
{
    if(zoomFactor == ZOOM_FACTOR_1x0){
        this->_zoom = 1.0;
    }else if (zoomFactor == ZOOM_FACTOR_1x2) {
        this->_zoom = 1.2;
    }else if (zoomFactor == ZOOM_FACTOR_1x4) {
        this->_zoom = 1.4;
    }else if (zoomFactor == ZOOM_FACTOR_1x6) {
        this->_zoom = 1.6;
    }else if (zoomFactor == ZOOM_FACTOR_1x8) {
        this->_zoom = 1.8;
    }else if(zoomFactor == ZOOM_FACTOR_2x0) {
        this->_zoom = 2.0;
    }
}

void GPUImage_Params::setLastMask_enable(bool able)
{
    this->_lastMask_enable = able;
}

void GPUImage_Params::setUndist_enable(bool able)
{
    this->_undist_enable = able;
}