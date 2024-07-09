#pragma once
#ifndef __LIVE_TEST__
#define __LIVE_TEST__


#include <stdio.h>
#include <iostream>

#ifdef _WIN32
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/opt.h"
#include "libavutil/avutil.h"
#include "libavutil/mathematics.h"
#include "libavutil/time.h"
};
#else
#ifdef __cplusplus
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/avutil.h>
#include <libavutil/mathematics.h>
#include <libavutil/time.h>
};
#endif
#endif


class live_test
{
public:
	int begin(const char* filename);

};







#endif