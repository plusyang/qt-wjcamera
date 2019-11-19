#ifndef GPUIMAGE_PARAMS_H
#define GPUIMAGE_PARAMS_H

#include "gpuimage_filter.h"

class GPUImage_Params
{
public:
    static GPUImage_Params* getInstance();

    void setBrightness(float brightness);
    void setSharpness(float shapness);
    void setBilateralParams(float multiplier, float normalization);
    void setColorEnhanceParams(float color_r, float color_g, float color_b);
    void setLastMask_enable(bool able);
    void setUndist_enable(bool able);
    void setRotation(ROTATION_MODE rotation);
    void setZoom(ZOOM_FACTOR zoomFactor);

    //get gpu filter paramters..
    float getBrightness(){ return _brightness; }
    float getSharpness(){return _shapness; }
    float getBilateral_multiplier(){return _bilateral_multiplier;}
    float getBilateral_normalization(){return _bilateral_normalization;}
    float getColor_r(){return _color_r;}
    float getColor_g(){return _color_g;}
    float getColor_b(){return _color_b;}
    bool getLastMask_enable(){return _lastMask_enable;}
    bool getUndist_enable(){return _undist_enable;}

    int getRotation(){return _rotation;}
    float getZoom(){return _zoom;}

private:

    GPUImage_Params();
    ~GPUImage_Params();
    static GPUImage_Params* _instance;

    float _brightness;
    float _shapness;
    float _bilateral_multiplier;
    float _bilateral_normalization;

    float _color_r;
    float _color_g;
    float _color_b;
    
    bool _lastMask_enable;
    bool _undist_enable;

    //rotation and zoom
    int _rotation;
    float _zoom;
};

#endif // GPUIMAGE_PARAMS_H
