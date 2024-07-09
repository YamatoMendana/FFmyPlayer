#include "libavutiltest.h"


#define TEST_OPT 1
#define TEST_LOG 1
#define TEST_DIC 0

void cAVutil::list_obj_test(void* obj)
{


}

void cAVutil::test_opt()
{
    AVFormatContext* pAVFmtCtx = nullptr;
    pAVFmtCtx = avformat_alloc_context();
    list_obj_test(pAVFmtCtx);
    avformat_free_context(pAVFmtCtx);


}

void cAVutil::test_log()
{
    AVFormatContext* pAVFmtCtx = nullptr;
    pAVFmtCtx = avformat_alloc_context();

    printf("======================================================\n");
    av_log(pAVFmtCtx,AV_LOG_PANIC,"Panic: Something went really wrong\n");
    av_log(pAVFmtCtx,AV_LOG_FATAL,"Fatal: Something went really wrong\n");
    av_log(pAVFmtCtx,AV_LOG_ERROR,"Error: Something went really wrong\n");
    av_log(pAVFmtCtx,AV_LOG_WARNING,"Warning: Something went really wrong\n");
    av_log(pAVFmtCtx,AV_LOG_INFO,"Info: Something went really wrong\n");
    av_log(pAVFmtCtx,AV_LOG_VERBOSE,"Verbose: Something went really wrong\n");
    av_log(pAVFmtCtx,AV_LOG_DEBUG,"Debug: Something went really wrong\n");
    printf("======================================================\n");

}

void cAVutil::print_opt(const AVOption* opt_test)
{
    printf("======================================================\n");
    printf("Option Information:\n");
    printf("[name]:%s\n",opt_test->name);
    printf("[help]:%s\n",opt_test->help);
    printf("[offset]:%s\n",opt_test->offset);

    switch (opt_test->type)
    {
    case AV_OPT_TYPE_INT:{
        printf("[type]int\n[default]%d\n",opt_test->default_val.i64);
    }
    case AV_OPT_TYPE_INT64:{
        printf("[type]int64\n[default]%lld\n",opt_test->default_val.i64);
    }
    case AV_OPT_TYPE_FLOAT:{
        printf("[type]float\n[default]%f\n",opt_test->default_val.dbl);
    }
    case AV_OPT_TYPE_STRING:{
        printf("[type]string\n[default]%d\n",opt_test->default_val.str);
    }
    case AV_OPT_TYPE_RATIONAL:{
        printf("[type]rational\n[default]%d/%d\n",opt_test->default_val.q.num,opt_test->default_val.q.den);
    }
    default:{
        printf("[type]others\n");
        break;
    }

    }

    printf("[max val]%f\n",opt_test->max);
    printf("[min val]%f\n",opt_test->min);

    if(opt_test->flags & AV_OPT_FLAG_ENCODING_PARAM)
    {
        printf("Enconding param.\n");
    }
    if(opt_test->flags & AV_OPT_FLAG_DECODING_PARAM)
    {
        printf("Deconding param.\n");
    }
    if(opt_test->flags & AV_OPT_FLAG_AUDIO_PARAM)
    {
        printf("Audio param.\n");
    }
    if(opt_test->flags & AV_OPT_FLAG_VIDEO_PARAM)
    {
        printf("Video param.\n");
    }

}

void cAVutil::custom_outopt(void *ptr, int level, const char *fmt, va_list vl)
{
    FILE* fp = fopen("simple_ffmpeg_log.log","a+");
    if(fp)
    {
        vfprintf(fp,fmt,vl);
        fflush(fp);
        fclose(fp);
    }
}

void cAVutil::test_parseutil()
{
    char strInput[100] = {0};
    printf("================ Parse Video Size =====================\n");
    int output_w = 0,output_h = 0;
    strcpy(strInput,"1920x1080");
    av_parse_video_size(&output_w,&output_h,strInput);
    printf("w: %4d | h: %4d\n",output_w,output_h);

    strcpy(strInput,"vga");//640*480(4:3)
    av_parse_video_size(&output_w,&output_h,strInput);
    printf("w: %4d | h: %4d\n",output_w,output_h);

    printf("================ Parse Frame Rate =====================\n");
    AVRational output_rational = {0,0};
    strcpy(strInput,"15/1");
    av_parse_video_rate(&output_rational,strInput);
    printf("framerate: %d/%d\n",output_rational.num,output_rational.den);

    strcpy(strInput,"pal");
    av_parse_video_rate(&output_rational,strInput);
    printf("framerate: %d/%d\n",output_rational.num,output_rational.den);

    printf("================ Parse Time =====================\n");
    int64_t output_timeval;//微秒
    strcpy(strInput,"00:01:01");
    av_parse_time(&output_timeval,strInput,1);
    printf("microseconds: %lld\n",output_timeval);

    printf("======================================================\n");
}



void cAVutil::test_avdictionary()
{
    AVDictionary *pAVdict = nullptr;
    AVDictionaryEntry *pAVdictEnpty = nullptr;

    av_dict_set(&pAVdict,"name","Tom",0);
    av_dict_set(&pAVdict,"age","3",0);
    av_dict_set(&pAVdict,"gender","man",0);

    char* k = av_strdup("location");
    char* v = av_strdup("USA");
    av_dict_set(&pAVdict,k,v,AV_DICT_DONT_STRDUP_KEY | AV_DICT_DONT_STRDUP_VAL);

    printf("======================================================\n");
    int dict_cnt = av_dict_count(pAVdict);
    printf("dict_count : %d\n",dict_cnt);
    printf("dict_element:\n");
    while(pAVdictEnpty = av_dict_get(pAVdict,"",pAVdictEnpty,AV_DICT_IGNORE_SUFFIX))
    {
        printf("key : %s | value : %s\n",pAVdictEnpty->key,pAVdictEnpty->value);
    }

    pAVdictEnpty = av_dict_get(pAVdict,"email",pAVdictEnpty,AV_DICT_IGNORE_SUFFIX);
    printf("email is %s \n",pAVdictEnpty->value);
    printf("======================================================\n");
    av_dict_free(&pAVdict);
}














