#include <QApplication>
#include <QDebug>
#include <pthread.h>
#include "videodisplay.h"
#include "gpuimage_filter.h"
#include "isp.h"
#include "unistd.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    VideoDisplay w;
    w.move(0,0);
    w.show();

   //sleep(1);

    // isp_set_aepoint(30);
    isp_set_denoise_shape(10,10);
    isp_sample_rkisp_test();

    gpu_set_shapness(0);
    gpu_set_denoise(0);
    gpu_set_rotation(NoRotation);
    gpu_set_zoom(ZOOM_FACTOR_1x0);

    return a.exec();
}
