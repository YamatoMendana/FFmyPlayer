#include "avio_file.h"
#include <QString>
#include <QDebug>

avio_file::avio_file()
{

}

avio_file::~avio_file()
{

}

int avio_file::read_func(void* ptr, uint8_t* buf, int buf_size)
{
    FILE* fp = (FILE*)ptr;
    size_t size = fread(buf,1, buf_size,fp);
    int ret = size;
    printf("Read Byte:%d\n",size);
    return ret;
}

int64_t avio_file::seek_func(void *opaque, int64_t offset, int whence)
{
    int64_t ret;
    FILE* fp = (FILE*)opaque;
    if(whence == AVSEEK_SIZE)
    {
        return -1;
    }

    fseek(fp,offset,whence);
    return ftell(fp);
}



int avio_file::openfile(const char* filename)
{

    avformat_network_init();
printf("Hello,FFmpeg!\n");

    AVFormatContext* pAVFmtCtx = nullptr;
    AVInputFormat* pAVIFmt = nullptr;

printf("Hello,avformat_alloc_context\n");
    pAVFmtCtx = avformat_alloc_context();

    bool isFail = false;

printf("Hello,avformat_open_input\n");
    int ret = avformat_open_input(&pAVFmtCtx,filename,pAVIFmt,nullptr);
    if(ret == 0)
    {
        printf("open stream success!\n");
    }
    else
    {
        printf("avformat open %s fail!\n",filename);
        isFail = true;
        goto quit;
    }

    ret = avformat_find_stream_info(pAVFmtCtx,nullptr);
    if(ret >= 0)
    {
        printf("avformat_find_stream_info success!\n");
        printf("nb_stream = %d \n",pAVFmtCtx->nb_streams);

    }
    else
    {
        printf("avformat_find_stream_info error!\n");
        isFail = true;
        goto quit;
    }

quit:
    avformat_free_context(pAVFmtCtx);
    avformat_close_input(&pAVFmtCtx);
    avformat_network_deinit();

    return isFail;
}

int avio_file::openfile2(const char *filename)
{
    int ret = 0;
    bool isFail = false;

    AVFormatContext* pAVFmtCtx = nullptr;
    AVInputFormat* pAVIFmt = nullptr;

    FILE* fp = fopen(filename,"rb");
    int nBufferSize = 1024*1024;
    unsigned char*  pBuffer = (unsigned char*)malloc(nBufferSize);

    AVIOContext* pAVIOCtx = avio_alloc_context(pBuffer,nBufferSize,0,fp,read_func,nullptr,seek_func);

    pAVFmtCtx = avformat_alloc_context();
    pAVFmtCtx->pb = pAVIOCtx;
    pAVFmtCtx->flags = AVFMT_FLAG_CUSTOM_IO;

    ret = avformat_open_input(&pAVFmtCtx,"",pAVIFmt,nullptr);
    if(ret == 0)
    {
        printf("open stream success!\n");
    }
    else
    {
        printf("avformat open %s fail!\n",filename);
        isFail = true;
        goto quit;
    }

    ret = avformat_find_stream_info(pAVFmtCtx,nullptr);
    if(ret >= 0)
    {
        printf("avformat_find_stream_info success!\n");
        printf("nb_stream = %d \n",pAVFmtCtx->nb_streams);

    }
    else
    {
        printf("avformat_find_stream_info error!\n");
        isFail = true;
        goto quit;
    }
quit:
    avformat_free_context(pAVFmtCtx);
    avformat_close_input(&pAVFmtCtx);
    avformat_network_deinit();

    return 0;

}

















