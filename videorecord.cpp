/****************************************
* 类名 VideoRecord
* 功能:mpp Encoder and MP4 Write(use mp4v2, support by video.h)
*
* 作者:yangzhenhao
* 日期:20190122
* 版本:1.0
*
*****************************************/
#include "videorecord.h"
#include "video.h"
#include <QDebug>
#include <QtGui>
#include <QString>
#include <QByteArray>
#include <QDateTime>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/statfs.h>
#include <jpeglib.h>

#include "common.h"
#include "record_picture.h"

//#define SAVE_PICTURE_TYPE_BMP
//#define SAVE_VIDEO_H264

extern bool isSaveVideo;
extern bool isSavePic;

static struct MPP_ENC_DATA mpp_enc_data;
static QString strSaveDate_Video;
static eStrorage_Mode strorageMode = eStrorageMode_HardDisk; //默认硬盘存储

QString binren = "xxxxxxx";
extern QString binren;
extern OsdMsg osd_msg;
bool videoDevice_StreamOn=false;
extern bool videoDevice_StreamOn;

namespace VideoRecord
{

int take_picture()
{
    if(videoDevice_StreamOn == false)
        return -1;
    isSavePic = true;
    usleep(1000*20);
    return 0;
}

int take_picture_write(unsigned char *p, int width, int height)
{
    isSavePic = false;

    QString floder;
    QString filename;
    QString diskPath_HardDisk, diskPath_UsbDisk;
    eStrorage_State strorageState;
    int ret = 0;

    strorageState = check_disk_state(eStrorageFormat_IMAGE);
    if(strorageState == eStrorageState_Wrong){
        if(strorageMode == eStrorageMode_HardDisk){
            osd_msg.type=1;
            osd_msg.shCnt = 40;
            osd_msg.msg = "拍照失败,硬盘存储空间不足";
            osd_msg.initShCnt = osd_msg.shCnt;
        }else{
            osd_msg.type=1;
            osd_msg.shCnt = 40;
            osd_msg.msg = "拍照失败,U盘存储空间不足";
            osd_msg.initShCnt = osd_msg.shCnt;
        }
        return -1;  
    }else if(strorageState == eStrorageState_PathError){
        osd_msg.type=1;
        osd_msg.shCnt = 40;
        osd_msg.msg = "拍照失败,未检测到硬盘";
        osd_msg.initShCnt = osd_msg.shCnt;
        return -1;
    }
 
    diskPath_HardDisk = FILE_ROOTPATH_HARDDISK;
    diskPath_UsbDisk = FILE_ROOTPATH_USBDISK;
    floder = diskPath_HardDisk+"/"+FILE_ROOTPATH_APP+"/"+binren+"_image";
    ret = CreatDir(floder.toLocal8Bit().data());
    if(ret != 0){
	    printf("%s CreatDir failled\n",__FILE__);
    } 
    
    //当前病人的照片和视频存放位置
    //QString Current_path=QCoreApplication::applicationDirPath();
    QDateTime saveTime =  QDateTime::currentDateTime();
    QString strSaveDate = saveTime.toString("yyyyMMdd_hhmmss");
    QString file_name_prefix=floder+"/"+strSaveDate;
    QString file_name_BMP = file_name_prefix+".bmp";
    QString file_name_JPG =file_name_prefix+".jpg";

    char* dest = file_name_JPG.toLocal8Bit().data();
    savePictureQt(p,width,height, dest);
    sync();

    // //BMP存储
    // #ifdef SAVE_PICTURE_TYPE_BMP   
    //     char* dest = file_name_BMP.toLocal8Bit().data();
    //     ret = savePicBmp(p, width, height, dest);
    //     sync();
    // #else
    //     //jpg存储
    //     char* dest = file_name_JPG.toLocal8Bit().data();
    //     ret = savePicJpg(p, width, height, dest);
    //     sync();
    // #endif
  
    if(ret < 0){
        osd_msg.type=1;
        osd_msg.shCnt = 40;
        osd_msg.msg = "拍照失败";
        osd_msg.initShCnt = osd_msg.shCnt;
    }else{
        osd_msg.type=1;
        osd_msg.shCnt = 40;
        osd_msg.msg = "拍照成功";
        osd_msg.initShCnt = osd_msg.shCnt;
    }

    return ret;
}

int savePicBmp(unsigned char *p,int width,int height, char *outfile)
{
    unsigned char *rgb_buffer;
    rgb_buffer = (unsigned char *)malloc(sizeof(unsigned char)*width*height*IMAGECHANEL);
    NV12_T_RGB(p,rgb_buffer, width,height);

    //只考虑24位位图
    int channels = 3;
    char bfType[2] = { 'B', 'M' };
    BITMAPFILEHEADER bmpHeader;
    BITMAPINFOHEADER bmpInfo;
    int bitmapHeaderSize = 40;
    int headerSize = 14 /* fileheader */ + bitmapHeaderSize;
    int fileStep = (width*channels + 3) & -4;
    int fileSize = fileStep*height + headerSize;
    uchar zeropad[] = "\0\0\0\0";

    bmpHeader.bfSize = fileSize;    // BMP文件总大小
    bmpHeader.bfReserved = 0;
    bmpHeader.bfOffBits = headerSize;

    bmpInfo.biSize = sizeof(BITMAPINFOHEADER);
    bmpInfo.biWidth = width;
    bmpInfo.biHeight = height;
    bmpInfo.biPlanes = 1;
    bmpInfo.biBitCount = channels << 3;
    bmpInfo.biCompression = 0;
    bmpInfo.biSizeImage = 0;
    bmpInfo.biXPelsPerMeter = 0;
    bmpInfo.biYPelsPerMeter = 0;
    bmpInfo.biClrUsed = 0;
    bmpInfo.biClrImportant = 0;

    unsigned char *rgb24_buffer = NULL;  //tmp buffer
    FILE *fp_bmp = NULL;

    rgb24_buffer = (unsigned char *)malloc(width*height * 3);
    memcpy(rgb24_buffer, rgb_buffer, width*height * 3);

    if ((fp_bmp = fopen(outfile, "wb")) == NULL){
        printf("Error: Cannot open output BMP file.\n");
        return -1;
    }

    fwrite(&bfType, 1, sizeof(bfType), fp_bmp);
    fwrite(&bmpHeader, 1, sizeof(bmpHeader), fp_bmp);
    fwrite(&bmpInfo, 1, sizeof(bmpInfo), fp_bmp);

    //BMP storage pixel data in opposite direction of Y-axis (from bottom to top).
    for (int j = height - 1; j >= 0; j--){
        //BMP save R1|G1|B1,R2|G2|B2 as B1|G1|R1,B2|G2|R2
        //So we change 'R' and 'B'
        for (int i = 0; i<width; i++){
            unsigned char temp = rgb24_buffer[(j*width + i) * channels + 2];
            rgb24_buffer[(j*width + i) * channels + 2] = rgb24_buffer[(j*width + i) * channels + 0];
            rgb24_buffer[(j*width + i) * channels + 0] = temp;
        }

        //write a row to file
        fwrite(rgb24_buffer + j*(channels * width), channels * width, 1, fp_bmp);
        if (fileStep > channels * width)
            fwrite(zeropad, fileStep - channels * width, 1, fp_bmp);
    }
    fflush(fp_bmp);
    fclose(fp_bmp);

    free(rgb24_buffer);
    free(rgb_buffer);
    printf("Finish generate %s!\n", outfile);

    return 0;

}

int savePicJpg(unsigned char *p, int width, int height, char *outfile)
{
    int quality = 100;
    unsigned char *rgb_buffer;
    rgb_buffer = (unsigned char *)malloc(sizeof(unsigned char)*width*height*IMAGECHANEL);
    NV12_T_RGB(p,rgb_buffer, width,height);

    FILE* fout = fopen(outfile,"wb");
    if(fout == NULL)
    {
        qDebug()<<"open file failed :"<<outfile;
        return -1;
    }

    JSAMPARRAY image;
    int i = 0, j = 0;
    image = (JSAMPARRAY)calloc(height, sizeof (JSAMPROW));
    for (i = 0; i < height; i++) {
        image[i] = (JSAMPROW)calloc(width * 3, sizeof (JSAMPLE));
        for (j = 0; j < width; j++) {
            image[i][j * 3 + 0] = rgb_buffer[(i * width + j) * 3 + 0];
            image[i][j * 3 + 1] = rgb_buffer[(i * width + j) * 3 + 1];
            image[i][j * 3 + 2] = rgb_buffer[(i * width + j) * 3 + 2];
        }
    }

    struct jpeg_compress_struct compress;
    struct jpeg_error_mgr error;
    compress.err = jpeg_std_error(&error);
    jpeg_create_compress(&compress);
    jpeg_stdio_dest(&compress, fout);

    compress.image_width = width;
    compress.image_height = height;
    compress.input_components = 3;
    compress.in_color_space = JCS_RGB;
    jpeg_set_defaults(&compress);
    jpeg_set_quality(&compress, quality, TRUE);
    jpeg_start_compress(&compress, TRUE);
    jpeg_write_scanlines(&compress, image, height);
    jpeg_finish_compress(&compress);
    jpeg_destroy_compress(&compress);

    for (i = 0; i < height; i++) {
        free(image[i]);
    }
    free(image);
    free(rgb_buffer);
    fflush(fout);
    fclose(fout);
    printf("Finish generate %s!\n", outfile);

    return 0;
}

void NV12_T_RGB(unsigned char *nv12 , unsigned char *rgb, int width , int height)
{
    const int nv_start = width * height;
    int  i, j, index = 0, rgb_index = 0;
    unsigned char y, u, v;
    int r, g, b, nv_index = 0;


    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++){
            //nv_index = (rgb_index / 2 - width / 2 * ((i + 1) / 2)) * 2;
            nv_index = i / 2 * width + j - j % 2;

            y = nv12[rgb_index];
            u = nv12[nv_start + nv_index];
            v = nv12[nv_start + nv_index + 1];

            r = y + (140 * (v - 128)) / 100;  //r
            g = y - (34 * (u - 128)) / 100 - (71 * (v - 128)) / 100; //g
            b = y + (177 * (u - 128)) / 100; //b

            if (r > 255)   r = 255;
            if (g > 255)   g = 255;
            if (b > 255)   b = 255;
            if (r < 0)     r = 0;
            if (g < 0)     g = 0;
            if (b < 0)     b = 0;

            //index = rgb_index % width + (height - i - 1) * width;  //置换行：0 <--> 719 , 1 <--> 718 , 2 <--> 717 ,....
            index = rgb_index ;
            rgb[index * 3 + 0] = r;
            rgb[index * 3 + 1] = g;
            rgb[index * 3 + 2] = b;
            rgb_index++;
        }
    }
    return;
}

int video_record()
{
    if(videoDevice_StreamOn == false)
        return -1;
        
    int ret=0;
    if(isSaveVideo == false){
        ret=video_record_start();
        if(ret == eStrorageState_Normal){
            osd_msg.type =2;
            osd_msg.shCnt=60;
            osd_msg.msg = "录像开始成功";
            osd_msg.initShCnt = osd_msg.shCnt;
            
        }else {
            if(strorageMode == eStrorageMode_HardDisk){
                if(ret == eStrorageState_Wrong){
                    osd_msg.type =2;
                    osd_msg.shCnt=60;
                    osd_msg.msg = "录像开始失败,硬盘存储空间不足";
                    osd_msg.initShCnt = osd_msg.shCnt;
                }else if(ret == eStrorageState_PathError){
                    osd_msg.type =2;
                    osd_msg.shCnt=60;
                    osd_msg.msg = "录像开始失败,未检测到硬盘";
                    osd_msg.initShCnt = osd_msg.shCnt;
                }else{
                    osd_msg.type =2;
                    osd_msg.shCnt=60;
                    osd_msg.msg = "录像开始失败";
                    osd_msg.initShCnt = osd_msg.shCnt;
                }

            }else if(strorageMode == eStrorageMode_UsbDisk){
                if(ret == eStrorageState_Wrong){
                    osd_msg.type =2;
                    osd_msg.shCnt=60;
                    osd_msg.msg = "录像开始失败,U盘存储空间不足";
                    osd_msg.initShCnt = osd_msg.shCnt;
                }else{
                    osd_msg.type =2;
                    osd_msg.shCnt=60;
                    osd_msg.msg = "录像开始失败";
                    osd_msg.initShCnt = osd_msg.shCnt;
                }
            }

        }
    }else{
        ret=video_record_stop();
        if(ret == eStrorageState_Normal){
            osd_msg.type =2;
            osd_msg.shCnt=60;
            osd_msg.msg = "录像结束成功";
            osd_msg.initShCnt = osd_msg.shCnt;
        }else{
            osd_msg.type =2;
            osd_msg.shCnt=60;
            osd_msg.msg = "录像结束失败";
            osd_msg.initShCnt = osd_msg.shCnt;
        }
    }

    usleep(1000*20);
    return ret;

}

int video_record_start()
{
    QString floder;
    QString filename;
    QString filepath;
    QString diskPath_HardDisk,diskPath_UsbDisk;
    eStrorage_State strorageState;
    int ret;
    strorageState = check_disk_state(eStrorageFormat_VIDEO);
    if(strorageState != eStrorageState_Normal){
        return strorageState;
    }

    diskPath_HardDisk = FILE_ROOTPATH_HARDDISK;
    floder = diskPath_HardDisk+"/"+FILE_ROOTPATH_APP+"/"+binren+"_video";
    ret = CreatDir(floder.toLocal8Bit().data());
    if(ret != 0){
	    printf("%s CreatDir failled\n",__FILE__);
	    return eStrorageState_OtherError;
    } 

    //QString Current_path=QCoreApplication::applicationDirPath();
    QDateTime saveTime =  QDateTime::currentDateTime();
    strSaveDate_Video = saveTime.toString("yyyyMMdd_hhmmss");
    QString mp4fileName=floder+"/"+strSaveDate_Video+".mp4";
    char* mp4file = mp4fileName.toLocal8Bit().data();

    ret = (MP4_Write_Init(mp4file, 30) == true) ? 0:-1;
    if(ret!=0){
        qDebug()<<"MP4_Write_Init failed..";
        return eStrorageState_OtherError;
    }

    ret = init_mpp(0,0);
    if(ret!=0){
        qDebug()<<"MPP init failed..";
        return eStrorageState_OtherError;
    }

    isSaveVideo = true;   
    return eStrorageState_Normal;
}

int video_record_stop()
{
    isSaveVideo = false;
    int ret = destroy_mpp();
    MP4_Write_Exit(); 
    sync();
    if(ret){
        qDebug()<<"destroy_mpp failed..";
        return eStrorageState_OtherError;
    }

    return eStrorageState_Normal;
}

int video_record_write(void *addr,int bufSize)
{

    MPP_RET ret = MPP_OK;
    MppFrame frame = NULL;
    MppPacket packet = NULL;
    void *buf = mpp_buffer_get_ptr(mpp_enc_data.frm_buf);

    //TODO: improve performance here?
    memcpy(buf, addr, bufSize);

    ret = mpp_frame_init(&frame);
    if (ret)
    {
        qDebug() << "mpp_frame_init failed";
        return -1;
    }

    mpp_frame_set_width(frame, mpp_enc_data.width);
    mpp_frame_set_height(frame, mpp_enc_data.height);
    mpp_frame_set_hor_stride(frame, mpp_enc_data.hor_stride);
    mpp_frame_set_ver_stride(frame, mpp_enc_data.ver_stride);
    mpp_frame_set_fmt(frame, mpp_enc_data.fmt);
    mpp_frame_set_buffer(frame, mpp_enc_data.frm_buf);
    mpp_frame_set_eos(frame, mpp_enc_data.frm_eos);

    ret = mpp_enc_data.mpi->encode_put_frame(mpp_enc_data.ctx, frame);
    if (ret)
    {
        qDebug() << "mpp encode put frame failed";
        return -1;
    }

    ret = mpp_enc_data.mpi->encode_get_packet(mpp_enc_data.ctx, &packet);
    if (ret)
    {
        qDebug() << "mpp encode get packet failed";
        return -1;
    }

    if(packet)
    {
        // write packet to file here
        void *ptr = mpp_packet_get_pos(packet);
        size_t pkt_len = mpp_packet_get_length(packet);

        mpp_enc_data.pkt_eos = mpp_packet_get_eos(packet);

        if (mpp_enc_data.fp_output)
            fwrite(ptr, 1, pkt_len, mpp_enc_data.fp_output);
        //mp4v2 write
            MP4_Write_Main((uint8_t*)ptr,pkt_len);

        mpp_packet_deinit(&packet);

        mpp_enc_data.frame_count++;
        mpp_enc_data.stream_size += pkt_len;

        if(mpp_enc_data.stream_size >= VIDEO_FILE_LIMIT_SIZE){
            video_record_stop();
            video_record_start();
        }

        if (mpp_enc_data.pkt_eos)
        {
            qDebug() << "found last packet";
        }
    }

/*
    printf("mpp_enc_data.num_frames :%d, frame_count :%d\n",mpp_enc_data.num_frames,mpp_enc_data.frame_count);
    if (mpp_enc_data.num_frames && mpp_enc_data.frame_count >= mpp_enc_data.num_frames) {
        printf("encode max %d frames.\n", mpp_enc_data.frame_count);
        return -1;
    }
*/

    if (mpp_enc_data.frm_eos && mpp_enc_data.pkt_eos)
        return -1;

    return 0;
}


int destroy_mpp()
{
    MPP_RET ret = MPP_OK;
    ret = mpp_enc_data.mpi->reset(mpp_enc_data.ctx);
    if (ret)
    {
        qDebug() << "mpi->reset failed";
        return -1;
    }

    if (mpp_enc_data.ctx)
    {
        mpp_destroy(mpp_enc_data.ctx);
        mpp_enc_data.ctx = NULL;
    }

    if (mpp_enc_data.frm_buf)
    {
        mpp_buffer_put(mpp_enc_data.frm_buf);
        mpp_enc_data.frm_buf = NULL;
    }

    if (mpp_enc_data.fp_output)
        fclose(mpp_enc_data.fp_output);

    return 0;
}

int init_mpp(int width, int height)
{
    MPP_RET ret = MPP_OK;
    memset(&mpp_enc_data, 0, sizeof(mpp_enc_data));

#ifdef SAVE_VIDEO_H264
    /****将当前病人的视频放到对应的文件夹当中******/
    QString Current_path=QCoreApplication::applicationDirPath();
    QString m_videoStrPath=Current_path+"/"+binren+"_video/";
    QDateTime saveTime =  QDateTime::currentDateTime();
    QString strSaveDate = saveTime.toString("yyyyMMdd_hhmmss");
    QString h264fileName=m_videoStrPath+strSaveDate+".h264";
    char* h264file = h264fileName.toLocal8Bit().data();
    mpp_enc_data.fp_output = fopen(h264file, "wb+");
#endif

    mpp_enc_data.width = width;
    mpp_enc_data.height = height;
    mpp_enc_data.hor_stride = MPP_ALIGN(mpp_enc_data.width, 16);
    mpp_enc_data.ver_stride = MPP_ALIGN(mpp_enc_data.height, 16);
    mpp_enc_data.fmt = MPP_FMT_YUV420SP;
    mpp_enc_data.frame_size = mpp_enc_data.hor_stride * mpp_enc_data.ver_stride * 2;
    mpp_enc_data.type = MPP_VIDEO_CodingAVC;
    mpp_enc_data.num_frames = 2000;

    ret = mpp_buffer_get(NULL, &mpp_enc_data.frm_buf, mpp_enc_data.frame_size);
    if (ret)
    {
        qDebug() << "failed to get buffer for input frame ret "  <<  ret;
        goto MPP_INIT_OUT;
    }

    ret = mpp_create(&mpp_enc_data.ctx, &mpp_enc_data.mpi);
    if (ret)
    {
        qDebug() << "mpp_create failed ret " << ret;
        goto MPP_INIT_OUT;
    }

    ret = mpp_init(mpp_enc_data.ctx, MPP_CTX_ENC, mpp_enc_data.type);
    if (ret)
    {
        qDebug() << "mpp_init failed ret " << ret;
        goto MPP_INIT_OUT;
    }

    mpp_enc_data.fps = 30;
    mpp_enc_data.gop = 60;
    mpp_enc_data.bps = mpp_enc_data.width * mpp_enc_data.height / 8 * mpp_enc_data.fps;

    mpp_enc_data.prep_cfg.change        = MPP_ENC_PREP_CFG_CHANGE_INPUT |
            MPP_ENC_PREP_CFG_CHANGE_ROTATION |
            MPP_ENC_PREP_CFG_CHANGE_FORMAT;
    mpp_enc_data.prep_cfg.width         = mpp_enc_data.width;
    mpp_enc_data.prep_cfg.height        = mpp_enc_data.height;
    mpp_enc_data.prep_cfg.hor_stride    = mpp_enc_data.hor_stride;
    mpp_enc_data.prep_cfg.ver_stride    = mpp_enc_data.ver_stride;
    mpp_enc_data.prep_cfg.format        = mpp_enc_data.fmt;
    mpp_enc_data.prep_cfg.rotation      = MPP_ENC_ROT_0;
    ret = mpp_enc_data.mpi->control(mpp_enc_data.ctx, MPP_ENC_SET_PREP_CFG, &mpp_enc_data.prep_cfg);
    if (ret)
    {
        qDebug() << "mpi control enc set prep cfg failed ret " << ret;
        goto MPP_INIT_OUT;
    }

    mpp_enc_data.rc_cfg.change  = MPP_ENC_RC_CFG_CHANGE_ALL;
    mpp_enc_data.rc_cfg.rc_mode = MPP_ENC_RC_MODE_CBR;
    mpp_enc_data.rc_cfg.quality = MPP_ENC_RC_QUALITY_MEDIUM;

    if (mpp_enc_data.rc_cfg.rc_mode == MPP_ENC_RC_MODE_CBR)
    {
        /* constant bitrate has very small bps range of 1/16 bps */
        mpp_enc_data.rc_cfg.bps_target   = mpp_enc_data.bps;
        mpp_enc_data.rc_cfg.bps_max      = mpp_enc_data.bps * 17 / 16;
        mpp_enc_data.rc_cfg.bps_min      = mpp_enc_data.bps * 15 / 16;
    }
    else if (mpp_enc_data.rc_cfg.rc_mode ==  MPP_ENC_RC_MODE_VBR)
    {
        if (mpp_enc_data.rc_cfg.quality == MPP_ENC_RC_QUALITY_CQP)
        {
            /* constant QP does not have bps */
            mpp_enc_data.rc_cfg.bps_target   = -1;
            mpp_enc_data.rc_cfg.bps_max      = -1;
            mpp_enc_data.rc_cfg.bps_min      = -1;
        }
        else
        {
            /* variable bitrate has large bps range */
            mpp_enc_data.rc_cfg.bps_target   = mpp_enc_data.bps;
            mpp_enc_data.rc_cfg.bps_max      = mpp_enc_data.bps * 17 / 16;
            mpp_enc_data.rc_cfg.bps_min      = mpp_enc_data.bps * 1 / 16;
        }
    }

    /* fix input / output frame rate */
    mpp_enc_data.rc_cfg.fps_in_flex      = 0;
    mpp_enc_data.rc_cfg.fps_in_num       = mpp_enc_data.fps;
    mpp_enc_data.rc_cfg.fps_in_denorm    = 1;
    mpp_enc_data.rc_cfg.fps_out_flex     = 0;
    mpp_enc_data.rc_cfg.fps_out_num      = mpp_enc_data.fps;
    mpp_enc_data.rc_cfg.fps_out_denorm   = 1;

    mpp_enc_data.rc_cfg.gop              = mpp_enc_data.gop;
    mpp_enc_data.rc_cfg.skip_cnt         = 0;

    ret = mpp_enc_data.mpi->control(mpp_enc_data.ctx, MPP_ENC_SET_RC_CFG, &mpp_enc_data.rc_cfg);
    if (ret)
    {
        qDebug() << "mpi control enc set rc cfg failed ret " << ret;
        goto MPP_INIT_OUT;
    }

    mpp_enc_data.codec_cfg.coding = mpp_enc_data.type;
    switch (mpp_enc_data.codec_cfg.coding)
    {
    case MPP_VIDEO_CodingAVC :
    {
        mpp_enc_data.codec_cfg.h264.change = MPP_ENC_H264_CFG_CHANGE_PROFILE |
                MPP_ENC_H264_CFG_CHANGE_ENTROPY |
                MPP_ENC_H264_CFG_CHANGE_TRANS_8x8;
        /*
         * H.264 profile_idc parameter
         * 66  - Baseline profile
         * 77  - Main profile
         * 100 - High profile
         */
        mpp_enc_data.codec_cfg.h264.profile  = 100;
        /*
         * H.264 level_idc parameter
         * 10 / 11 / 12 / 13    - qcif@15fps / cif@7.5fps / cif@15fps / cif@30fps
         * 20 / 21 / 22         - cif@30fps / half-D1@@25fps / D1@12.5fps
         * 30 / 31 / 32         - D1@25fps / 720p@30fps / 720p@60fps
         * 40 / 41 / 42         - 1080p@30fps / 1080p@30fps / 1080p@60fps
         * 50 / 51 / 52         - 4K@30fps
         */
        if(width == 1280 && height ==720){
            mpp_enc_data.codec_cfg.h264.level    = 31;
        }else if(width == 1920 && height ==1080){
            mpp_enc_data.codec_cfg.h264.level    = 41;
        }
        
        mpp_enc_data.codec_cfg.h264.entropy_coding_mode  = 1;
        mpp_enc_data.codec_cfg.h264.cabac_init_idc  = 0;
        mpp_enc_data.codec_cfg.h264.transform8x8_mode = 1;
    }
    break;
    case MPP_VIDEO_CodingMJPEG :
    {
        mpp_enc_data.codec_cfg.jpeg.change  = MPP_ENC_JPEG_CFG_CHANGE_QP;
        mpp_enc_data.codec_cfg.jpeg.quant   = 10;
    }
    break;
    case MPP_VIDEO_CodingVP8 :
    case MPP_VIDEO_CodingHEVC :
    default :
    {
        qDebug() << "support encoder coding type " << mpp_enc_data.codec_cfg.coding;
    }
    break;
    }
    ret = mpp_enc_data.mpi->control(mpp_enc_data.ctx, MPP_ENC_SET_CODEC_CFG, &mpp_enc_data.codec_cfg);
    if (ret)
    {
       qDebug() << "mpi control enc set codec cfg failed ret " << ret;
        goto MPP_INIT_OUT;
    }

    /* optional */
    mpp_enc_data.sei_mode = MPP_ENC_SEI_MODE_ONE_FRAME;
    ret = mpp_enc_data.mpi->control(mpp_enc_data.ctx, MPP_ENC_SET_SEI_CFG, &mpp_enc_data.sei_mode);
    if (ret)
    {
        qDebug() << "mpi control enc set sei cfg failed ret " << ret;
        goto MPP_INIT_OUT;
    }

    if (mpp_enc_data.type == MPP_VIDEO_CodingAVC)
    {
        MppPacket packet = NULL;
        ret = mpp_enc_data.mpi->control(mpp_enc_data.ctx, MPP_ENC_GET_EXTRA_INFO, &packet);
        if (ret)
        {
            qDebug() << "mpi control enc get extra info failed";
            goto MPP_INIT_OUT;
        }

        /* get and write sps/pps for H.264 */
        if (packet)
        {
            void *ptr   = mpp_packet_get_pos(packet);
            size_t len  = mpp_packet_get_length(packet);

            #ifdef SAVE_VIDEO_H264
            if (mpp_enc_data.fp_output)
                fwrite(ptr, 1, len, mpp_enc_data.fp_output);
            #endif

            //MP4_Write_Main((uint8_t*)ptr,len);
            MP4_Write_SPS_PPS((uint8_t*)ptr,len);

            packet = NULL;
        }
    }

    return 0;

MPP_INIT_OUT:

    if (mpp_enc_data.ctx)
    {
        mpp_destroy(mpp_enc_data.ctx);
        mpp_enc_data.ctx = NULL;
    }

    if (mpp_enc_data.frm_buf)
    {
        mpp_buffer_put(mpp_enc_data.frm_buf);
        mpp_enc_data.frm_buf = NULL;
    }

    qDebug() << "init mpp failed!";
    return -1;
}


int CreatDir(char *pszDir)  
{  
    int i = 0;  
    int iRet;  
    int iLen = strlen(pszDir);  
  
    //在末尾加/  
    if (pszDir[iLen - 1] != '\\' && pszDir[iLen - 1] != '/')  
    {  
        pszDir[iLen] = '/';  
        pszDir[iLen + 1] = '\0';  
    }  
  
    // 创建目录  
    for (i = 0;i <= iLen;i ++)  
    {  
        if ((pszDir[i] == '\\' || pszDir[i] == '/') && (i != 0))  
        {   
            pszDir[i] = '\0';  
            //如果不存在,创建  
            iRet = ACCESS(pszDir,0);  
            if (iRet != 0)  
            {  
                iRet = MKDIR(pszDir);  
                if (iRet != 0)  
                {  
                    return -1;  
                }   
            }  
            //支持linux,将所有\换成/  
            pszDir[i] = '/';  
        }   
    }  
    return 0;  
} 

//函数功能：检测disk的状态情况
eStrorage_State check_disk_state(eStrorage_Format strorageformat)
{
    struct statfs diskInfo;
    char diskPath[64];
    uint64_t blocksize;
    uint64_t availableDisk;
    unsigned int wronglimit = 0;

    switch (strorageMode)
    {
    case eStrorageMode_HardDisk:
        sprintf((char *)&diskPath,"%s",FILE_ROOTPATH_HARDDISK);
        if(strorageformat == eStrorageFormat_VIDEO){
            wronglimit = VIDEO_HARDDISK_WRONG;    
        }else{
            wronglimit = IMAGE_HARDDISK_WRONG;  
        }
        break;
    case eStrorageMode_UsbDisk:
        sprintf((char *)&diskPath,"%s",FILE_ROOTPATH_USBDISK);
        if(strorageformat == eStrorageFormat_VIDEO){
            wronglimit = VIDEO_USBDISK_WRONG;    
        }else{
            wronglimit = IMAGE_USBDISK_WRONG;  
        }
        break;
    default:
        break;
    }

    if(ACCESS(diskPath, F_OK) != 0)
    {
        printf("disk file is not exist!\n");
        return eStrorageState_PathError;
    }

    statfs(diskPath, &diskInfo);
    blocksize = diskInfo.f_bsize; 
    availableDisk = diskInfo.f_bavail * blocksize;
    availableDisk /= MB;
    printf("availableDisk:%lld wronglimit:%d \n",availableDisk,wronglimit);
    if(availableDisk <= wronglimit){
	    return eStrorageState_Wrong;	    
    }else{
	    return eStrorageState_Normal;
    }

}


int copy_file(char* src, char* des)
{
	int nRet = 0;
	FILE* pSrc = NULL, *pDes = NULL;
	pSrc = fopen(src, "r");
	pDes = fopen(des, "w+");
 
	if (pSrc && pDes){
		int nLen = 0;
		char szBuf[1024] = {0};
		while((nLen = fread(szBuf, 1, sizeof szBuf, pSrc)) > 0){
			fwrite(szBuf, 1, nLen, pDes);
		}
	}
	else{
        nRet = -1;
    }
 
	if (pSrc){
        fclose(pSrc);
        pSrc = NULL;
    }
		
	if (pDes){
        fflush(pDes);
        fclose(pDes);
        pDes = NULL;
    }	
 
	return nRet;
}

}
