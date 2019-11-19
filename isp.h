#ifndef ISP_H
#define ISP_H

#define ISP_BRIGHTNESS_MIN -100
#define ISP_BRIGHTNESS_MAX 100
#define ISP_CONTRAST_MIN -100
#define ISP_CONTRAST_MAX 100
#define ISP_SATURATION_MIN -100
#define ISP_SATURATION_MAX 100
#define ISP_HUE_MIN -90
#define ISP_HUE_MAX 90

#define ISP_DENOISE_SHAPE_MIN 0
#define ISP_DENOISE_SHAPE_MAX 10


enum {
    ISP_AGC_ON,
    ISP_AGC_OFF,
};

enum {
    ISP_AE_METERING_AVERAGE,
    ISP_AE_METERING_PEAK,
    ISP_AE_METERING_AUTO,
};



int isp_start_func(int fd, const char* _isp_iq_file);
int isp_stop_func();
void* isp_get_rkisp_engine();

void isp_do_awb();
void isp_set_autoAE();
void isp_set_aepoint(float val);
void isp_set_denoise_shape(int level_denoise,int level_shape);
void isp_set_brightness(int level);
void isp_set_contrast(int level);
void isp_set_saturation(int level);
void isp_set_hue(int level);
void isp_set_agc(int type);
void isp_change_AE_metering(int type);
void isp_sample_rkisp_test();


#endif
