

#include "sourcecamerayuv.h"
#include "../Context.hpp"
#include "../util.h"

USING_NS_GI

SourceCameraYUV::SourceCameraYUV()
{
}

SourceCameraYUV::~SourceCameraYUV()
{
}

SourceCameraYUV* SourceCameraYUV::create(int contexID)
{
    SourceCameraYUV* camera = new SourceCameraYUV();
    camera->_contexID = contexID;
    return camera;
}


void SourceCameraYUV::setFrameData(int width, int height, unsigned char* pixels, RotationMode outputRotation)
{
    Framebuffer *framebuffer = Context::getInstance()->getFramebufferCache()->fetchFramebuffer(this->_contexID, width, height);
    framebuffer->setYuvData(pixels);
    this->setFramebuffer(framebuffer, outputRotation);
    framebuffer->release();

}
