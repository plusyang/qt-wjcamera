#include "record_picture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <QImage>
#include <QString>

int savePictureQt(unsigned char *data, int width, int height, char *outfile)
{
    QImage out(data,width,height,QImage::Format_RGBA8888);
    out.save(QString(outfile));
    
    return 0;
}



