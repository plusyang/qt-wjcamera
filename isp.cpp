
#include <stdio.h>
#include <unistd.h>
#include "isp.h"
#include "common.h"
#include "rkisp_interface.h"

void* _rkisp_engine;

static unsigned char AE_metering_coff[3][25]={
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0, 1, 3, 1, 0, 
 2, 4, 6, 4, 2, 
 3, 6, 9, 6, 3, 
 2, 4, 6, 4, 2, 
 0, 2, 4, 2, 0}
 };


int isp_start_func(int fd, const char* _isp_iq_file)
{
    int ret=0;

    if(_rkisp_engine != NULL){
        QLog(LOG_INFO, "isp start failed, rkisp_engine already exit.");
        ret = -2;
    }
        
    rkisp_start(_rkisp_engine, fd, ISP_DEV_NAME, _isp_iq_file);
    if (_rkisp_engine == NULL){
        QLog(LOG_INFO, "isp start failed");
        ret = -1;
    }else
        QLog(LOG_INFO, "isp start success");
    //close AE FLICKER
    rkisp_setAntiBandMode(_rkisp_engine,HAL_AE_FLK_OFF);
    return ret;
}

int isp_stop_func()
{   int ret=0;
    ret = rkisp_stop(_rkisp_engine);
    if(ret == 0)
        QLog(LOG_INFO, "isp stop success");
    else
        QLog(LOG_INFO, "isp stop failed");
    return ret;
}


void isp_do_awb()
{
    rkisp_set3ALocks(_rkisp_engine,HAL_3A_LOCKS_NONE);
    usleep(1000*200);
    //rkisp_saveAwbCfgToFile(_rkisp_engine);
    rkisp_set3ALocks(_rkisp_engine,HAL_3A_LOCKS_WB);           
    QLog(LOG_INFO, "do awb done");

}

void isp_set_denoise_shape(int level_denoise,int level_shape)
{
    level_denoise = level_denoise < ISP_DENOISE_SHAPE_MIN ? ISP_DENOISE_SHAPE_MIN : level_denoise;
    level_denoise = level_denoise > ISP_DENOISE_SHAPE_MAX ? ISP_DENOISE_SHAPE_MAX : level_denoise;
    level_shape = level_shape < ISP_DENOISE_SHAPE_MIN ? ISP_DENOISE_SHAPE_MIN : level_shape;
    level_shape = level_shape > ISP_DENOISE_SHAPE_MAX ? ISP_DENOISE_SHAPE_MAX : level_shape;

    enum HAL_MODE_e modee=HAL_MODE_MANUAL;
    enum HAL_FLT_DENOISE_LEVEL_e denoiseLevel=HAL_FLT_DENOISE_LEVEL_e(level_denoise);
    enum HAL_FLT_SHARPENING_LEVEL_e shapLevel=HAL_FLT_SHARPENING_LEVEL_e(level_shape);

    rkisp_setFilterLevel(_rkisp_engine,modee,denoiseLevel,shapLevel);
    QLog(LOG_INFO, "denoise_shape: (denoiseLevel shapLevel) = %d  %d",denoiseLevel,shapLevel);

}

void isp_set_brightness(int level)
{
    int setp = 1.0 * ISP_BRIGHTNESS_MAX / 8;
    
    int brightVal = level*setp;
    if(brightVal < ISP_BRIGHTNESS_MIN)
        brightVal = ISP_BRIGHTNESS_MIN;
    if(brightVal > ISP_BRIGHTNESS_MAX)
        brightVal = ISP_BRIGHTNESS_MAX;
    
    rkisp_setBrightness(_rkisp_engine, brightVal);
    QLog(LOG_INFO, "brightness: level = %d, brightVal = %d",level,brightVal);
}

void isp_set_contrast(int level)
{
    int setp = 1.0 * ISP_CONTRAST_MAX / 8;
    
    int contrast = level*setp;
    if(contrast < ISP_CONTRAST_MIN)
        contrast = ISP_CONTRAST_MIN;
    if(contrast > ISP_CONTRAST_MAX)
        contrast = ISP_CONTRAST_MAX;
    
    rkisp_setContrast(_rkisp_engine, contrast);
    QLog(LOG_INFO, "contrast: level = %d, contrast = %d",level,contrast);
}

void isp_set_saturation(int level)
{
    int setp = 1.0 * ISP_SATURATION_MAX / 8;
    
    int sat = level*setp;
    if(sat < ISP_SATURATION_MIN)
        sat = ISP_SATURATION_MIN;
    if(sat > ISP_SATURATION_MAX)
        sat = ISP_SATURATION_MAX;
    
    rkisp_setSaturation(_rkisp_engine, sat);
    QLog(LOG_INFO, "Saturation: level = %d, sat = %d",level,sat);
}

void isp_set_hue(int level)
{
    int setp = 1.0 * ISP_HUE_MAX / 8;
    
    int hue = level*setp;
    if(hue < ISP_HUE_MIN)
        hue = ISP_HUE_MIN;
    if(hue > ISP_HUE_MAX)
        hue = ISP_HUE_MAX;
    
    rkisp_setHue(_rkisp_engine, hue);
    QLog(LOG_INFO, "Hue: level = %d, hue = %d",level,hue);
}

void isp_set_agc(int type)
{
    if(type ==ISP_AGC_ON ){
        rkisp_setAeMaxExposureGain(_rkisp_engine,5);
    }else{
        rkisp_setAeMaxExposureGain(_rkisp_engine,0);
    }
    float gain;
    rkisp_getAeMaxExposureGain(_rkisp_engine,gain);
    QLog(LOG_INFO, "AGC: = %f\n",gain);
}

void isp_set_aepoint(float val)
{
    if(val >255) val = 255;
    else if(val <0) val =0;
    rkisp_setAeSetPoint(_rkisp_engine, val);
    float v;
    rkisp_getAeSetPoint(_rkisp_engine, v);
    QLog(LOG_INFO, "AE point : = %f\n",v);
}

void isp_set_autoAE()
{
    rkisp_setAeMode(_rkisp_engine,HAL_AE_OPERATION_MODE_AUTO);
    QLog(LOG_INFO, "AE auto ON \n");
}


extern unsigned char ExpMeans[25];
void isp_change_AE_metering(int type)
{
    rkisp_setExposureMeterMode(_rkisp_engine,HAL_AE_METERING_MODE_USER);

    if(type==ISP_AE_METERING_AVERAGE){
        rkisp_setExposureMeterCoeff(_rkisp_engine,AE_metering_coff[0]);
    }else if(type==ISP_AE_METERING_PEAK){
        rkisp_setExposureMeterCoeff(_rkisp_engine,AE_metering_coff[1]);
    }else{
        rkisp_setExposureMeterCoeff(_rkisp_engine,AE_metering_coff[2]);
    }


/*  //TODO..
    //wait ae stable
    usleep(1000*200);
    if(type==ISP_AE_METERING_PEAK){
        int maxIdx=0;
        int maxVal=ExpMeans[0];
        for(int i=0; i<25;i++){
            if(maxVal<ExpMeans[i]) {
                maxIdx = i;
                maxVal = ExpMeans[i];
            }
        }
        int ae_point = 0;
        rkisp_getAeSetPoint(_rkisp_engine,ae_point);
        AE_metering_coff[1][maxIdx] = 1.0*maxVal/ae_point*AE_metering_coff[1][maxIdx];
        rkisp_setExposureMeterCoeff(_rkisp_engine,AE_metering_coff[1]);
    }
*/

}

void isp_sample_rkisp_test()
{
    int aeModeTest=0;
    float time=0.0f;
    float gain=0.0f;
    float maxtime,maxgain,meanLuma;

    enum HAL_AE_STATE aestate;
    int locks = HAL_3A_LOCKS_EXPOSURE;
    float aepoint = 30;
    HAL_Window_t expWindow = {800,400,400,200};
    enum HAL_AE_METERING_MODE aemetmod=HAL_AE_METERING_MODE_AVERAGE;
    enum HAL_AE_OPERATION_MODE setAeMode=HAL_AE_OPERATION_MODE(aeModeTest);

    enum HAL_MODE_e modee=HAL_MODE_MANUAL;
    enum HAL_FLT_DENOISE_LEVEL_e denoiseLevel=HAL_FLT_DENOISE_LEVEL_10;
    enum HAL_FLT_SHARPENING_LEVEL_e shapLevel=HAL_FLT_SHARPENING_LEVEL_10;

    HAL_FPS_INFO_t fpsInfo={1,60};
    rkisp_setFps(_rkisp_engine,fpsInfo);

    rkisp_getAeTime(_rkisp_engine,time);
    rkisp_getAeGain(_rkisp_engine,gain);
    rkisp_getAeMaxExposureTime(_rkisp_engine,maxtime);
    rkisp_getAeMaxExposureGain(_rkisp_engine,maxgain);
    rkisp_getAeMeanLuma(_rkisp_engine,meanLuma);
    rkisp_getAeSetPoint(_rkisp_engine,aepoint);
    rkisp_getAeState(_rkisp_engine,aestate);
    rkisp_get3ALocks(_rkisp_engine,locks);
    rkisp_getAeWindow(_rkisp_engine,expWindow.left_hoff,expWindow.top_voff,expWindow.right_width,expWindow.bottom_height);
    rkisp_getExposureMeterMode(_rkisp_engine,aemetmod);

    rkisp_getFilterLevel(_rkisp_engine,modee,denoiseLevel,shapLevel);
    rkisp_getFps(_rkisp_engine,fpsInfo);
    printf("@@@@@@@@@@@@ AE time gain := %f  %f \n",time,gain);
    printf("@@@@@@@@@@@@ AE max_time max_gain := %f  %f \n",maxtime,maxgain);
    printf("@@@@@@@@@@@@ AE meanLuma aepoint := %f  %f\n",meanLuma, aepoint);
    printf("@@@@@@@@@@@@ AE Window := %d  %d  %d  %d\n",expWindow.left_hoff,expWindow.top_voff,expWindow.right_width,expWindow.bottom_height);
    printf("@@@@@@@@@@@@ AE aestate locks := %d  %d\n",aestate,locks);
    printf("@@@@@@@@@@@@ AE Meter setAeMode := %d %d\n",aemetmod,setAeMode);
    printf("@@@@@@@@@@@@  denoiseLevel shapLevel := %d  %d \n",denoiseLevel,shapLevel);
    printf("@@@@@@@@@@@@  fpsInfo := %d  %d \n",fpsInfo.denominator,fpsInfo.numerator);

}

void* isp_get_rkisp_engine()
{
    return _rkisp_engine;
}

