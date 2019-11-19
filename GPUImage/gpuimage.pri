HEADERS += \
    $$PWD/../GPUImage/Context.hpp \
    $$PWD/../GPUImage/Framebuffer.hpp \
    $$PWD/../GPUImage/FramebufferCache.hpp \
    $$PWD/../GPUImage/GLProgram.hpp \
    $$PWD/../GPUImage/GPUImage-x.h \
    $$PWD/../GPUImage/Ref.hpp \
#    $$PWD/../GPUImage/filter/BeautifyFilter.hpp \
    $$PWD/../GPUImage/filter/BilateralFilter.hpp \
    $$PWD/../GPUImage/filter/BrightnessFilter.hpp \
#    $$PWD/../GPUImage/filter/CannyEdgeDetectionFilter.hpp \
#    $$PWD/../GPUImage/filter/ColorInvertFilter.hpp \
     $$PWD/../GPUImage/filter/ColorMatrixFilter.hpp \
     $$PWD/../GPUImage/filter/ContrastFilter.hpp \
#    $$PWD/../GPUImage/filter/Convolution3x3Filter.hpp \
#    $$PWD/../GPUImage/filter/CrosshatchFilter.hpp \
#    $$PWD/../GPUImage/filter/DirectionalNonMaximumSuppressionFilter.hpp \
#    $$PWD/../GPUImage/filter/DirectionalSobelEdgeDetectionFilter.hpp \
#    $$PWD/../GPUImage/filter/EmbossFilter.hpp \
#    $$PWD/../GPUImage/filter/ExposureFilter.hpp \
    $$PWD/../GPUImage/filter/Filter.hpp \
    $$PWD/../GPUImage/filter/FilterGroup.hpp \
    $$PWD/../GPUImage/filter/GaussianBlurFilter.hpp \
    $$PWD/../GPUImage/filter/GaussianBlurMonoFilter.hpp \
#    $$PWD/../GPUImage/filter/GlassSphereFilter.hpp \
     $$PWD/../GPUImage/filter/GrayscaleFilter.hpp \
     $$PWD/../GPUImage/filter/HSBFilter.hpp \
#    $$PWD/../GPUImage/filter/HalftoneFilter.hpp \
#    $$PWD/../GPUImage/filter/HueFilter.hpp \
#    $$PWD/../GPUImage/filter/IOSBlurFilter.hpp \
#    $$PWD/../GPUImage/filter/LuminanceRangeFilter.hpp \
     $$PWD/../GPUImage/filter/NearbySampling3x3Filter.hpp \
#    $$PWD/../GPUImage/filter/NonMaximumSuppressionFilter.hpp \
#    $$PWD/../GPUImage/filter/PixellationFilter.hpp \
#    $$PWD/../GPUImage/filter/PosterizeFilter.hpp \
    $$PWD/../GPUImage/filter/RGBFilter.hpp \
#    $$PWD/../GPUImage/filter/SaturationFilter.hpp \
#    $$PWD/../GPUImage/filter/SingleComponentGaussianBlurFilter.hpp \
#    $$PWD/../GPUImage/filter/SingleComponentGaussianBlurMonoFilter.hpp \
#    $$PWD/../GPUImage/filter/SketchFilter.hpp \
    $$PWD/../GPUImage/filter/SmoothToonFilter.hpp \
#    $$PWD/../GPUImage/filter/SobelEdgeDetectionFilter.hpp \
#    $$PWD/../GPUImage/filter/SphereRefractionFilter.hpp \
    $$PWD/../GPUImage/filter/ToonFilter.hpp \
#    $$PWD/../GPUImage/filter/WeakPixelInclusionFilter.hpp \
#    $$PWD/../GPUImage/filter/WhiteBalanceFilter.hpp \
    $$PWD/../GPUImage/macros.h \
    $$PWD/../GPUImage/math.hpp \
    $$PWD/../GPUImage/source/Source.hpp \
    $$PWD/../GPUImage/source/SourceCamera.h \
    $$PWD/../GPUImage/source/SourceImage.h \
    $$PWD/../GPUImage/target/Target.hpp \
    $$PWD/../GPUImage/target/TargetView.h \
    $$PWD/../GPUImage/util.h \
    $$PWD/source/sourcecamerayuv.h \
    $$PWD/filter/yuv2rgbfilter.h \
    $$PWD/filter/sharpenfilter.h \
    $$PWD/filter/LastMaskFilter.h \
	$$PWD/filter/UndistortFilter.h \
    $$PWD/filter/ZoomFilter.h \
    $$PWD/filter/ColorEnhanceFilter.h

SOURCES += \
    $$PWD/../GPUImage/Context.cpp \
    $$PWD/../GPUImage/Framebuffer.cpp \
    $$PWD/../GPUImage/FramebufferCache.cpp \
    $$PWD/../GPUImage/GLProgram.cpp \
    $$PWD/../GPUImage/GPUImagexJNI.cpp \
    $$PWD/../GPUImage/Ref.cpp \
#    $$PWD/../GPUImage/filter/BeautifyFilter.cpp \
    $$PWD/../GPUImage/filter/BilateralFilter.cpp \
    $$PWD/../GPUImage/filter/BrightnessFilter.cpp \
#    $$PWD/../GPUImage/filter/CannyEdgeDetectionFilter.cpp \
#    $$PWD/../GPUImage/filter/ColorInvertFilter.cpp \
     $$PWD/../GPUImage/filter/ColorMatrixFilter.cpp \
     $$PWD/../GPUImage/filter/ContrastFilter.cpp \
#    $$PWD/../GPUImage/filter/Convolution3x3Filter.cpp \
#    $$PWD/../GPUImage/filter/CrosshatchFilter.cpp \
#    $$PWD/../GPUImage/filter/DirectionalNonMaximumSuppressionFilter.cpp \
#    $$PWD/../GPUImage/filter/DirectionalSobelEdgeDetectionFilter.cpp \
#    $$PWD/../GPUImage/filter/EmbossFilter.cpp \
#    $$PWD/../GPUImage/filter/ExposureFilter.cpp \
    $$PWD/../GPUImage/filter/Filter.cpp \
    $$PWD/../GPUImage/filter/FilterGroup.cpp \
    $$PWD/../GPUImage/filter/GaussianBlurFilter.cpp \
    $$PWD/../GPUImage/filter/GaussianBlurMonoFilter.cpp \
#    $$PWD/../GPUImage/filter/GlassSphereFilter.cpp \
     $$PWD/../GPUImage/filter/GrayscaleFilter.cpp \
     $$PWD/../GPUImage/filter/HSBFilter.cpp \
#    $$PWD/../GPUImage/filter/HalftoneFilter.cpp \
#    $$PWD/../GPUImage/filter/HueFilter.cpp \
#    $$PWD/../GPUImage/filter/IOSBlurFilter.cpp \
#    $$PWD/../GPUImage/filter/LuminanceRangeFilter.cpp \
    $$PWD/../GPUImage/filter/NearbySampling3x3Filter.cpp \
#    $$PWD/../GPUImage/filter/NonMaximumSuppressionFilter.cpp \
#    $$PWD/../GPUImage/filter/PixellationFilter.cpp \
#    $$PWD/../GPUImage/filter/PosterizeFilter.cpp \
    $$PWD/../GPUImage/filter/RGBFilter.cpp \
#    $$PWD/../GPUImage/filter/SaturationFilter.cpp \
#    $$PWD/../GPUImage/filter/SingleComponentGaussianBlurFilter.cpp \
#    $$PWD/../GPUImage/filter/SingleComponentGaussianBlurMonoFilter.cpp \
#    $$PWD/../GPUImage/filter/SketchFilter.cpp \
    $$PWD/../GPUImage/filter/SmoothToonFilter.cpp \
#    $$PWD/../GPUImage/filter/SobelEdgeDetectionFilter.cpp \
#    $$PWD/../GPUImage/filter/SphereRefractionFilter.cpp \
    $$PWD/../GPUImage/filter/ToonFilter.cpp \
#    $$PWD/../GPUImage/filter/WeakPixelInclusionFilter.cpp \
#    $$PWD/../GPUImage/filter/WhiteBalanceFilter.cpp \
    $$PWD/../GPUImage/math.cpp \
    $$PWD/../GPUImage/source/Source.cpp \
    $$PWD/../GPUImage/source/SourceCamera.cpp \
    $$PWD/../GPUImage/source/SourceImage.cpp \
    $$PWD/../GPUImage/target/Target.cpp \
    $$PWD/../GPUImage/target/TargetView.cpp \
    $$PWD/../GPUImage/util.cpp \
    $$PWD/source/sourcecamerayuv.cpp \
    $$PWD/filter/yuv2rgbfilter.cpp \
    $$PWD/filter/sharpenfilter.cpp \
    $$PWD/filter/LastMaskFilter.cpp \
	$$PWD/filter/UndistortFilter.cpp \
    $$PWD/filter/ZoomFilter.cpp \
    $$PWD/filter/ColorEnhanceFilter.cpp

INCLUDEPATH +=$$PWD \
              $$PWD/filter \
              $$PWD/source \
              $$PWD/target
