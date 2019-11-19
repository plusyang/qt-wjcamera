#ifndef ZOOMFILTER_H
#define ZOOMFILTER_H


#include "../macros.h"
#include "Filter.hpp"

NS_GI_BEGIN


class ZoomFilter : public Filter
{
public:
    ZoomFilter();

    static ZoomFilter* create(int contexID=0);
    bool init();
    bool proceed(bool bUpdateTargets = true) override;
    void setZoom(float zoomVal);

private:
    GLuint _colorMap_location;
    GLuint _filterTexCoordAttributeLocation;

    float _zoom;
    GLfloat _zoomVertices[8];
    GLuint _zoomFrameBuffer;
    GLuint _zoomTexture;

};

NS_GI_END

#endif // ZOOMFILTER_H
