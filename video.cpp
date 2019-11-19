/****************************************
* video.c - definitions for the video interface
*
* 作者:yangzhenhao
* 日期:20190122
* 版本:1.0
*
*****************************************/

#include "video.h"
#include <QDebug>

/***************************************************
* 全局变量
****************************************************/
static MP4FileHandle m_Mp4File   = MP4_INVALID_FILE_HANDLE; //写文件的句柄
static MP4TrackId        video   = MP4_INVALID_TRACK_ID;    //写视频轨道ID
/***************************************************
* 函数功能：录制MP4文件初始化
* 参数1：文件路径名
* 参数2：帧率
* 返回值：success：true, failure: false
****************************************************/
bool MP4_Write_Init(char *filePath, int fs)
{
    /*********************************************************
    * 1 create save-file
    *********************************************************/
    //printf("Create MP4 file, 路径：%s! \n", m_filePath);
    m_Mp4File = MP4Create(filePath);

    if(m_Mp4File == MP4_INVALID_FILE_HANDLE)
    {
        printf("Create %s failure! \n", filePath);
        return false;
    }
    else
    {
        printf("Create %s success! \n", filePath);
    }

    /*********************************************************
    * 2 设置时间刻度
    *********************************************************/
    if(!MP4SetTimeScale(m_Mp4File, 90000))
    {
        if (m_Mp4File != MP4_INVALID_FILE_HANDLE)
        {
            MP4Close(m_Mp4File,0);
            m_Mp4File = MP4_INVALID_FILE_HANDLE;
        }
        return false;
    }

    /*********************************************************
    * 3 添加h264 track
    **********************************************************/
    //添加h264 track    // 90000 / 28.5 = 3157
    video = MP4AddH264VideoTrack(m_Mp4File, 90000, 90000/fs, 1920, 1080,
                0x64, //sps[1] AVCProfileIndication
                0x00, //sps[2] profile_compat
                0x1f, //sps[3] AVCLevelIndication
                  3); // 4 bytes length before each NAL unit
    if (video == MP4_INVALID_TRACK_ID)
    {
        printf("add video track failed.\n");
        return false;
    }
    MP4SetVideoProfileLevel(m_Mp4File, 0x7F);

    return true;
}


/***************************************************
* 函数功能： Write H264 SPS and PPS
* 参数1：帧buffer指针
* 返回值：
*
* modify by yzh, 20190215
****************************************************/
void MP4_Write_SPS_PPS(uint8_t* ptr, int len)
{
    //TODO optimize
    bool ret = false;
    int sps_pps_blk_len = 29+4*3;
    unsigned char data[len-sps_pps_blk_len+4];
    data[0] = (len-sps_pps_blk_len) >> 24;
    data[1] = (len-sps_pps_blk_len) >> 16;
    data[2] = (len-sps_pps_blk_len) >> 8;
    data[3] = (len-sps_pps_blk_len) & 0xff;
    memcpy(data + 4, ptr+sps_pps_blk_len, len-sps_pps_blk_len);
    ret = MP4WriteSample(m_Mp4File, video, data, len-sps_pps_blk_len+4, MP4_INVALID_DURATION, 0, 1);
    if(!ret)
    {
        printf("MP4 WriteSample Track failure! \n");
    }

    //TODO optimize
    int ssp_size = 25;
    int pps_size = 4;
    unsigned char* ssp;
    unsigned char* pps;
    ssp=(unsigned char*)(ptr+4);
    pps=(unsigned char*)(ptr+33);

    MP4AddH264SequenceParameterSet(m_Mp4File, video, ssp, ssp_size);
    MP4AddH264PictureParameterSet(m_Mp4File, video, pps, pps_size);

}


/***************************************************
* 函数功能： 视频帧数据写入文件
* 参数1：帧buffer指针
* 返回值：success：true, failure: false
*
* modify by yzh, 20190215
****************************************************/
bool MP4_Write_Main(uint8_t* ptr, int len)
{
    bool ret = false;
    unsigned char data[len];
    data[0] = (len-4) >> 24;
    data[1] = (len-4) >> 16;
    data[2] = (len-4) >> 8;
    data[3] = (len-4) & 0xff;
    memcpy(data + 4, ptr+4, len-4);
    ret = MP4WriteSample(m_Mp4File, video, data, len, MP4_INVALID_DURATION, 0, 1);
    if(!ret)
    {
        printf("MP4 WriteSample Track failure! \n");
    }

    return ret;
}

/***************************************************
* 函数功能：关闭视频文件
* 参数1：文件的路径名
* 返回值：无
****************************************************/
void MP4_Write_Exit(void)
{
    if (m_Mp4File != MP4_INVALID_FILE_HANDLE)
    {
        MP4Close(m_Mp4File,0);
        m_Mp4File = MP4_INVALID_FILE_HANDLE;
    }
    return;
}


