#ifndef SHARPENFILTER_H
#define SHARPENFILTER_H

#include "../macros.h"
#include "Filter.hpp"
#include <string>

NS_GI_BEGIN


class SharpenFilter:public Filter
{
public:
    SharpenFilter(){}


    static SharpenFilter* create(int contexID=0,float sharpness=0);
    bool init(float sharpness);
    bool proceed(bool bUpdateTargets = true) override;

    void setParamters(float sharpness);

private:
    float _sharpness;
    GLuint _filterTexCoordAttribute;

};


NS_GI_END

#endif // SHARPENFILTER_H









