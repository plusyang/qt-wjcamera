#ifndef SOURCECAMERAYUV_H
#define SOURCECAMERAYUV_H

#include "Source.hpp"

NS_GI_BEGIN

class SourceCameraYUV : public Source {
public:
    SourceCameraYUV();
    ~SourceCameraYUV();

    static SourceCameraYUV* create(int contexID=0);

    void setFrameData(int width, int height, unsigned char* pixels, RotationMode outputRotation = RotationMode::NoRotation);



};

NS_GI_END

#endif // SOURCECAMERAYUV_H
