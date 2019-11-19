#ifndef UNDISTORTFILTER_H
#define UNDISTORTFILTER_H

#include "../macros.h"
#include "Filter.hpp"

NS_GI_BEGIN

class UndistortFilter : public Filter
{
public:
    UndistortFilter(){}

    static UndistortFilter* create(int contexID=0);
    bool init();
    bool proceed(bool bUpdateTargets = true) override;
    void setEnable(bool able);

private:
    GLuint _colorMap_location;
    int _enable;
    
};

NS_GI_END

#endif // UNDISTORTFILTER_H
