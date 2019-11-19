#include "ZoomFilter.h"
#include "Context.hpp"

USING_NS_GI


ZoomFilter::ZoomFilter(){}


ZoomFilter* ZoomFilter::create(int contexID)
{
    ZoomFilter* ret = new (std::nothrow) ZoomFilter();
    ret->_contexID = contexID;
    ret->_zoom = 1.0;
    ret->setZoom(ret->_zoom);
    if (ret && !ret->init()) {
        delete ret;
        ret = 0;
    }
    return ret;
}

bool ZoomFilter::init()
{
    if (!initWithFragmentShaderString(kDefaultFragmentShader)) return false;
    Context::getInstance()->setActiveShaderProgram(_filterProgram);
    _colorMap_location = _filterProgram->getUniformLocation("colorMap");
    _filterTexCoordAttributeLocation = _filterProgram->getAttribLocation("texCoord");
    CHECK_GL(glEnableVertexAttribArray(_filterTexCoordAttributeLocation));

    return true;
}

bool ZoomFilter::proceed(bool bUpdateTargets)
{
    _framebuffer->active();
    CHECK_GL(glClearColor(_backgroundColor.r, _backgroundColor.g, _backgroundColor.b, _backgroundColor.a));
    CHECK_GL(glClear(GL_COLOR_BUFFER_BIT));
    Context::getInstance()->setActiveShaderProgram(_filterProgram);
    std::map<int, InputFrameBufferInfo>::const_iterator it = _inputFramebuffers.begin();
    Framebuffer* fb = it->second.frameBuffer;

    CHECK_GL(glActiveTexture(GL_TEXTURE0));
    CHECK_GL(glBindTexture ( GL_TEXTURE_2D, fb->getTexture()));
    CHECK_GL(glUniform1i(_colorMap_location, 0));

    CHECK_GL(glVertexAttribPointer(_filterPositionAttribute, 2, GL_FLOAT, 0, 0, _zoomVertices));
    CHECK_GL(glVertexAttribPointer(_filterTexCoordAttributeLocation, 2, GL_FLOAT, 0, 0, _getTexureCoordinate(it->second.rotationMode)));
    CHECK_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

    _framebuffer->inactive();

    return Source::proceed(bUpdateTargets);
}

void ZoomFilter::setZoom(float zoomVal)
{
    float scaledWidth = zoomVal;
    float scaledHeight = zoomVal;
    _zoomVertices[0] = -scaledWidth;
    _zoomVertices[1] = -scaledHeight;
    _zoomVertices[2] = scaledWidth;
    _zoomVertices[3] = -scaledHeight;
    _zoomVertices[4] = -scaledWidth;
    _zoomVertices[5] = scaledHeight;
    _zoomVertices[6] = scaledWidth;
    _zoomVertices[7] = scaledHeight;
}
