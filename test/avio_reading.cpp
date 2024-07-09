#include "avio_reading.h"

avio_reading::avio_reading()
{

}

int avio_reading::read_packet(void *opaque, uint8_t *buf, int buf_size)
{
    struct buffer_data* bd = (struct buffer_data*)opaque;
    buf_size = FFMIN(buf_size, bd->size);

    if (!buf_size)
        return AVERROR_EOF;
    printf("ptr:%p size:%zu\n", bd->ptr, bd->size);

    /* copy internal buffer data to buf */
    memcpy(buf, bd->ptr, buf_size);
    bd->ptr += buf_size;
    bd->size -= buf_size;

    return buf_size;
}


int avio_reading::begin(const char* filename)
{
    AVFormatContext* pAVFmtCtx = nullptr;

    AVIOContext* pAVIOCtx = nullptr;
    uint8_t* buffer = nullptr;
    uint8_t* avio_ctx_buffer = nullptr;

    size_t buffer_size;
    size_t avio_ctx_buffer_size = 4096;
    char input_name[128];
    int ret = 0;
    struct buffer_data bd = {0};

    strcpy(input_name,filename);

    /* slurp file content into buffer 读取所有数据*/
    ret = av_file_map(input_name, &buffer, &buffer_size, 0, NULL);
    if (ret < 0)
    {
        goto end;
    }

    /* fill opaque structure used by the AVIOContext read callback */
    bd.ptr = buffer;          //数据内容指针
    bd.size = buffer_size;    //数据字节大小

    if (!(pAVFmtCtx = avformat_alloc_context())) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    avio_ctx_buffer = (uint8_t*)av_malloc(avio_ctx_buffer_size);
    if (!avio_ctx_buffer) {
        ret = AVERROR(ENOMEM);
        goto end;
    }
    pAVIOCtx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size,
        0, &bd, &read_packet, NULL, NULL);
    if (!pAVIOCtx) {
        ret = AVERROR(ENOMEM);
        goto end;
    }
    pAVFmtCtx->pb = pAVIOCtx;

    ret = avformat_open_input(&pAVFmtCtx, NULL, NULL, NULL); //  init_input——>av_probe_input_buffer2——>avio_read——>read_packet_wrapper  中调用read_packet
    if (ret < 0) {
        fprintf(stderr, "Could not open input\n");
        goto end;
    }
    //获取数据包中的流信息
    ret = avformat_find_stream_info(pAVFmtCtx, NULL);    //read_frame_internal——>ff_read_packet  中调用read_packet
    if (ret < 0) {
        fprintf(stderr, "Could not find stream information\n");
        goto end;
    }

    //抛出数据流信息
    av_dump_format(pAVFmtCtx, 0, input_name, 0);

end:
    avformat_close_input(&pAVFmtCtx);

    /* note: the internal buffer could have changed, and be != avio_ctx_buffer */
    if (pAVIOCtx)
        av_freep(&pAVIOCtx->buffer);
    avio_context_free(&pAVIOCtx);

    av_file_unmap(buffer, buffer_size);

    if (ret < 0) {
        fprintf(stderr, "Error occurred: %s\n", av_err2str(ret));
        return 1;
    }

    return 0;

}


