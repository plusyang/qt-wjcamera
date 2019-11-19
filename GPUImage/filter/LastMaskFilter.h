#ifndef LASTMASKFILTER_H
#define LASTMASKFILTER_H

#include "../macros.h"
#include "Filter.hpp"

NS_GI_BEGIN


class LastMaskFilter:public Filter
{
public:
    LastMaskFilter(){}
    ~LastMaskFilter();

    static LastMaskFilter* create(int contexID=0);
    bool init();
    bool proceed(bool bUpdateTargets = true) override;

    void setMaskPixel(int width,int height, unsigned char* pixel);
    void setEnable(bool able);

private:

    GLuint _colorMap_location;
    GLuint _maskMap_textureID;
    GLuint _maskMap_location;
    int _enable;

};


NS_GI_END

#endif // LASTMASKFILTER_H
