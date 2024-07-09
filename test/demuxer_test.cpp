#include "demuxer_test.h"

#include <QDebug>


demuxer_test::demuxer_test()
{


}

int demuxer_test::start()
{
    const char* path = "10s.mp4";
//    avformat_network_init();//打开网络流，如果只打开本地则不用
    AVDictionary* pdicts = nullptr;
    AVFormatContext* pFmtCtx = nullptr;

    int ret = avformat_open_input(&pFmtCtx,path,nullptr,&pdicts);
    if(ret != 0)
    {
        char buf[1024] = {0};
        av_strerror(ret,buf,sizeof (buf)-1);
        printf("open %s failed!:%s",path,buf);
    }
    else
    {
        printf("打开媒体文件 %s 成功\n",path);
        avformat_find_stream_info(pFmtCtx,nullptr);

        printf("媒体文件 名称:%s\n",pFmtCtx->url);//filename已弃用
        printf("音视频流 个数：%d\n",pFmtCtx->nb_streams);

        printf("媒体文件的平均码率：%lld bps\n",pFmtCtx->bit_rate);
        printf("duration:%d\n",pFmtCtx->duration);

        int tns,thh,tmm,tss;
        tns = (pFmtCtx->duration) / AV_TIME_BASE;
        thh = tns / 3600;
        tmm = (tns % 3600) / 60;
        tss = (tns % 60);

        printf("媒体文件总时长 %d:%d:%d\n",thh,tmm,tss);

        for(int i = 0;i < pFmtCtx->nb_streams; i++)
        {
            AVStream* stream = pFmtCtx->streams[i];
            if(AVMEDIA_TYPE_AUDIO == stream->codecpar->codec_type)
            {
                printf("音频信息：\n");
                printf("index: %d\n",stream->index);
                printf("音频采样率： %d HZ\n", stream->codecpar->sample_rate);
                if(AV_SAMPLE_FMT_FLTP == stream->codecpar->format)
                {
                    printf("音频采样格式：AV_SAMPLE_FMT_FLTP \n");
                }
                else if(AV_SAMPLE_FMT_S16P == stream->codecpar->format)
                {
                    printf("音频采样格式：AV_SAMPLE_FMT_S16P \n");
                }
                printf("音频 信道数：%d\n",stream->codecpar->channels);
                if(AV_CODEC_ID_AAC == stream->codecpar->codec_id)
                {
                    printf("音频压缩编码格式：AAC\n");
                }
                else if(AV_CODEC_ID_MP3 == stream->codecpar->codec_id)
                {
                    printf("音频压缩编码格式：MP3\n");
                }
                //音频总时长
                int durationAudio = (stream->duration) * r2d(stream->time_base);
                printf("音频总时长:%d:%d:%d \n",durationAudio / 3600,(durationAudio % 3600) / 60,(durationAudio % 60));
            }
            else if(AVMEDIA_TYPE_VIDEO == stream->codecpar->codec_type)
            {
                printf("视频信息：\n");
                printf("index: %d\n",stream->index);
                printf("视频帧率： %lff ps\n", r2d(stream->avg_frame_rate));

                if(AV_CODEC_ID_MPEG4 == stream->codecpar->codec_id)
                {
                    printf("视频压缩编码格式：MPEG4 \n");
                }
                printf("帧宽度：%d 帧高度：%d\n",stream->codecpar->width,stream->codecpar->height);
                int durationVideo = (stream->duration) * r2d(stream->time_base);
                printf("视频总时长:%d:%d:%d \n",durationVideo / 3600,(durationVideo % 3600) / 60,(durationVideo % 60));
            }
        }

        int audioindex = av_find_best_stream(pFmtCtx,AVMEDIA_TYPE_AUDIO,-1,-1,nullptr,0);
        if(audioindex < 0)
        {
            printf("av_find_best_stream %s error\n",av_get_media_type_string(AVMEDIA_TYPE_AUDIO));
            return -1;
        }
        AVStream* audio_stream = pFmtCtx->streams[audioindex];
        printf("-------Audio info:\n");
        printf("index: %d\n",audio_stream->index);
        printf("samplarate: %d Hz\n",audio_stream->codecpar->sample_rate);
        printf("sampleformat: %d \n",audio_stream->codecpar->format);
        printf("audio codec: %d \n",audio_stream->codecpar->codec_id);
        if(audio_stream->duration != AV_NOPTS_VALUE)
        {
            int audio_duration = audio_stream->duration * av_q2d(audio_stream->time_base);
            printf("audio duration: %02d:%02d:%02d\n",audio_duration / 3600, (audio_duration % 3600) / 60,(audio_duration % 60));
        }

        int videoindex = av_find_best_stream(pFmtCtx,AVMEDIA_TYPE_VIDEO,-1,-1,nullptr,0);
        if(audioindex < 0)
        {
            printf("av_find_best_stream %s error\n",av_get_media_type_string(AVMEDIA_TYPE_VIDEO));
            return -1;
        }
        AVStream* video_stream = pFmtCtx->streams[videoindex];
        printf("-------Video info:\n");
        printf("index: %d\n",video_stream->index);
        printf("fps: %lf \n",av_q2d(video_stream->avg_frame_rate));
        printf("width: %d Height: %d\n",video_stream->codecpar->width,video_stream->codecpar->height);
        printf("video codec: %d \n",video_stream->codecpar->codec_id);
        if(video_stream->duration != AV_NOPTS_VALUE)
        {
            int video_duration = video_stream->duration * av_q2d(video_stream->time_base);
            printf("video duration: %02d:%02d:%02d\n",video_duration / 3600, (video_duration % 3600) / 60,(video_duration % 60));
        }

        AVPacket* pkt = av_packet_alloc();
        int pkt_count = 0;
        int print_max_count = 100;
        while(1)
        {
            ret = av_read_frame(pFmtCtx,pkt);
            if(ret < 0)
            {
                printf("av_read_frame end\n");
                break;
            }
            if(pkt_count++ < print_max_count)
            {
                if(pkt->stream_index == audioindex)
                {
                    printf("audio pts : %lld\n",pkt->pts);
                    printf("audio dts : %lld\n",pkt->dts);
                    printf("audio size : %d\n",pkt->size);
                    printf("audio pos : %lld\n",pkt->pos);
                    printf("audio duration : %lf\n",pkt->duration * av_q2d(pFmtCtx->streams[audioindex]->time_base));
                }
                else if(pkt->stream_index == videoindex)
                {
                    printf("video pts : %lld\n",pkt->pts);
                    printf("video dts : %lld\n",pkt->dts);
                    printf("video size : %d\n",pkt->size);
                    printf("video pos : %lld\n",pkt->pos);
                    printf("video duration : %lf\n",pkt->duration * av_q2d(pFmtCtx->streams[videoindex]->time_base));
                }
                else
                {
                    printf("unknown stream index:%d\n",pkt->stream_index);
                }
            }
            av_packet_unref(pkt);
        }
        if(pkt)
        {
            av_packet_free(&pkt);
        }
        if(pFmtCtx)
        {
            avformat_close_input(&pFmtCtx);
        }

    }
    return 0;
}












