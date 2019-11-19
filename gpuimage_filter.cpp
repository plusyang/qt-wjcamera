
#include "gpuimage_filter.h"
#include "gpuimage_params.h"
#include "common.h"

void gpu_set_shapness(int level)
{
    if(level>10)
        level=10;
    if(level<0)
        level=0;
    float val=(4.0/10.0)*level;
    GPUImage_Params::getInstance()->setSharpness(val);
    QLog(LOG_INFO, "gpu shapness: = %d ",level);
}


void gpu_set_denoise(int level)
{
    if(level>10)
        level=10;
    if(level<0)
        level=0;
    float normalization = 8.0;
    float multiplier = (4.0/10.0)*level;
    GPUImage_Params::getInstance()->setBilateralParams(multiplier,normalization);
    QLog(LOG_INFO, "gpu denoise: = %d ",level);
}

void gpu_set_rotation(ROTATION_MODE mode)
{
    GPUImage_Params::getInstance()->setRotation(mode);
    QLog(LOG_INFO, "gpu rotation: = %d ",mode);
}


void gpu_set_zoom(ZOOM_FACTOR zoomFactor)
{
    GPUImage_Params::getInstance()->setZoom(zoomFactor);
    QLog(LOG_INFO, "gpu zoom: = %d ",zoomFactor);
}


void gpu_color_enhance(float r, float g, float b)
{
    //TODO..
    //if(level>3)
    //    level=3;
    //if(level<0)
    //    level=0;
    //GPUImage_Params::getInstance()->setColorEnhanceParams(1.0,1.0,1.0);
    //QLog(LOG_INFO, "gpu Color Enhance: = %d ",level);
    
    GPUImage_Params::getInstance()->setColorEnhanceParams(r,g,b);
    QLog(LOG_INFO, "gpu Color Enhance: TODO");
}


void gpu_8mask_enable(bool able)
{
    GPUImage_Params::getInstance()->setLastMask_enable(able);
}
void gpu_undist_enable(bool able)
{
    GPUImage_Params::getInstance()->setUndist_enable(able);
}
