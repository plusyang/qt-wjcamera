#ifndef GPUIMAGE_FILTER_H
#define GPUIMAGE_FILTER_H

enum ROTATION_MODE {
    NoRotation = 0,
    RotateLeft,
    RotateRight,
    FlipVertical,
    FlipHorizontal,
    RotateRightFlipVertical,
    RotateRightFlipHorizontal,
    Rotate180
};

enum ZOOM_FACTOR {
    ZOOM_FACTOR_1x0,
    ZOOM_FACTOR_1x2,
    ZOOM_FACTOR_1x4,
    ZOOM_FACTOR_1x6,
    ZOOM_FACTOR_1x8,
    ZOOM_FACTOR_2x0,
};

void gpu_set_shapness(int level);
void gpu_set_denoise(int level);
void gpu_set_rotation(ROTATION_MODE mode);
void gpu_set_zoom(ZOOM_FACTOR zoomFactor);

void gpu_color_enhance(float r, float g, float b); //TODO
void gpu_8mask_enable(bool able);
void gpu_undist_enable(bool able);

#endif // GPUIMAGE_FILTER_H
