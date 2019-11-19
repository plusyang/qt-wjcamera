#ifndef COLORENHANCEFILTER_H
#define COLORENHANCEFILTER_H

#include "../macros.h"
#include "FilterGroup.hpp"
#include "HSBFilter.hpp"
#include "ContrastFilter.hpp"
#include "RGBFilter.hpp"

NS_GI_BEGIN

class ColorEnhanceFilter : public FilterGroup
{
public:
    ColorEnhanceFilter(){}
    ~ColorEnhanceFilter();
    static ColorEnhanceFilter* create(int contexID=0);
    bool init();
    virtual bool proceed(bool bUpdateTargets = true) override;
    void setRGBContrast(float contrast_r,float contrast_g,float contrast_b);
    void setRGBAdjust(float r,float g,float b);
    void setHsbAttrib(float brightness,float saturation);

protected:

    ContrastFilter* _contrastFilter;
    RGBFilter* _rgbFilter;
    HSBFilter* _hsbFilter;
};

NS_GI_END
#endif // COLORENHANCEFILTER_H
