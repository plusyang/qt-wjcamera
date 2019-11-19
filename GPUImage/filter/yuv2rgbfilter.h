#ifndef YUV2RGBFILTER_H
#define YUV2RGBFILTER_H

#include "../macros.h"
#include "Filter.hpp"
#include <string>

NS_GI_BEGIN

/**
 * @brief The Yuv2RgbFilter class
 * add by yzh
 */
class Yuv2RgbFilter: public Filter
{
public:
    Yuv2RgbFilter(){}
    ~Yuv2RgbFilter();
    static Yuv2RgbFilter* create(int contexID=0);
    bool init();
    bool proceed(bool bUpdateTargets = true) override;

private:
    GLuint yuv_tex[2];

};

NS_GI_END

#endif // YUV2RGBFILTER_H
