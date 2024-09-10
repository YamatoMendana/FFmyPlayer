
// 包含Google Test头文件
#include <gtest/gtest.h>

// 包含项目特定的头文件
#include "wave.h"


TEST(PCM2WAVTest, run) {
	PCM2WAV calc("E:\\Code\\QtCode\\FFmyPlayer\\bin\\10s.pcm",
		"E:\\Code\\QtCode\\FFmyPlayer\\bin\\pcm2wav.wav",
		2, 44100, 16);
	EXPECT_EQ(calc.start(), 0);
}

TEST(WAV2PCMTest, run)
{
	WAV2PCM calc("E:\\Code\\QtCode\\FFmyPlayer\\bin\\10s.wav",
		"E:\\Code\\QtCode\\FFmyPlayer\\bin\\wav2pcm.pcm");
	EXPECT_EQ(calc.start(), 0);
}