#include "encode_video.h"


Encode_Video::Encode_Video()
{

}

void Encode_Video::encode(AVCodecContext *encCtx, AVFrame *frame, AVPacket *pkt, FILE *outfile)
{
    int ret = 0;
    if(frame)
        printf("Send frame %3" PRId64 "\n",frame->pts);
    ret = avcodec_send_frame(encCtx,frame);
    if(ret < 0)
    {
        fprintf(stderr,"Error sending a frame for encoding\n");
        exit(1);
    }
    while(ret >= 0)
    {
        ret = avcodec_receive_packet(encCtx,pkt);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if(ret < 0)
        {
            fprintf(stderr,"Error during encoding\n");
            exit(1);
        }
        printf("Write Packet %3" PRId64 " (size=%5d)\n",pkt->pts,pkt->size);
        fwrite(pkt->data,1,pkt->size,outfile);
        av_packet_unref(pkt);
    }
}

int Encode_Video::begin(const char *inputName, const char *outputName)
{
    const char* filename,*codecname;
    const AVCodec* codec;
    AVCodecContext* pctx = nullptr;
    int x,y,i,ret;
    FILE* fp;
    AVFrame* frame;
    AVPacket* pkt;
    uint8_t encodec[] = {0,0,1,0xb7};

    filename = inputName;
    codecname = outputName;

    codec = avcodec_find_encoder_by_name(codecname);
    if(!codec)
    {
        fprintf(stderr,"Codec '%s' not found\n",codecname);
        exit(1);
    }

    pctx = avcodec_alloc_context3(codec);
    if(!pctx)
    {
        fprintf(stderr,"could not allocate video codec context\n");
        exit(1);
    }

    pkt = av_packet_alloc();
    if(!pkt)
        exit(1);

    pctx->bit_rate = 400000;
    pctx->width = 1920;
    pctx->height = 1080;
    pctx->time_base = {1,25};
    pctx->framerate = {25,1};

    pctx->gop_size = 10;
    pctx->max_b_frames = 1;
    pctx->pix_fmt = AV_PIX_FMT_YUV420P;

    if(codec->id == AV_CODEC_ID_H264)
        av_opt_set(pctx->priv_data,"preset","slow",0);
    ret = avcodec_open2(pctx,codec,nullptr);
    fp = fopen(filename,"wb");
    if(!fp)
    {
        fprintf(stderr,"could not open %s\n",filename);
        exit(1);
    }

    frame = av_frame_alloc();
    if(!frame)
    {
        fprintf(stderr,"Codec not allocate video frame\n");
        exit(1);
    }
    frame->format = pctx->pix_fmt;
    frame->width = pctx->width;
    frame->height = pctx->height;

    ret = av_frame_get_buffer(frame,0);
    if(ret < 0)
    {
        fprintf(stderr,"Codec not allocate video frame data buffer\n");
        exit(1);
    }

    for(i = 0; i < 25; i++)
    {
        fflush(stdout);

        ret = av_frame_make_writable(frame);
        if(ret < 0)
            exit(1);
        //Y
        for(y = 0;y < pctx->height; y++)
            for(x = 0; x < pctx->width;x++)
            {
                frame->data[0][y*frame->linesize[0] + x] = x + y + i * 3;
            }
        //U V
        for(y = 0;y < pctx->height / 2; y++)
            for(x = 0; x < pctx->width / 2; x++)
            {
                frame->data[1][y*frame->linesize[1] + x] = 128 + y + i * 2;
                frame->data[2][y*frame->linesize[2] + x] = 64 + x + i * 5;
            }
        frame->pts = i;

        encode(pctx,frame,pkt,fp);
    }
    encode(pctx,NULL,pkt,fp);

    fclose(fp);

    avcodec_free_context(&pctx);
    av_frame_free(&frame);
    av_packet_free(&pkt);

    return 0;

}

















