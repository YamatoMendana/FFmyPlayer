#ifndef __WAVE_H__
#define __WAVE_H__

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <memory>
#include <vector>

using namespace std;

typedef struct WAV_HEADER
{
	char chunkid[4];	//大端
	unsigned long chunksize;	//小端
	char format[4];
}wavHeader;

typedef struct WAV_FMT
{
	char subformat[4];
	unsigned long subsize;
	unsigned short audioFormat;
	unsigned short numChannels;
	unsigned long sampleRate;
	unsigned long byteRate;
	unsigned long blockAlign;
	unsigned short bitPerSample;
}wavFmt;


typedef struct WAV_DATA
{
	char data[4];
	unsigned long dataSize;
}wavData;

class PCM2WAV
{
public:
	explicit PCM2WAV(char* in,char* out,int channels,int sampleRate,int bits);

	long getFileSize(char* filename);

	int start();

private:
	char* inFilePath = nullptr;
	char* outFilePath = nullptr;
	int channels;
	int sampleRate;
	int fmtSize;
	int bits;
	long pcmDataSize;

};

struct RIFFHeader {
	char chunkID[4];      // "RIFF"
	uint32_t chunkSize;   // 文件大小（不包括 RIFF 标识符和文件大小字段）
	char format[4];       // "WAVE"
};

struct FMTChunk {
	char subchunk1ID[4];  // "fmt "
	uint32_t subchunk1Size; // fmt 块的大小
	uint16_t audioFormat; // 音频格式
	uint16_t numChannels; // 通道数
	uint32_t sampleRate;  // 采样率
	uint32_t byteRate;    // 比特率
	uint16_t blockAlign;  // 块对齐
	uint16_t bitsPerSample; // 位深度
};

struct LISTChunk {
	char subchunk2ID[4];  // "LIST"
	uint32_t subchunk2Size; // LIST 块的大小
	char listType[4];     // "INFO"
	char isft[4];         // "ISFT"
	std::vector<char> isftData; // ISFT 块的内容
};

struct DataChunk {
	char subchunk3ID[4];  // "data"
	uint32_t subchunk3Size; // 音频数据的大小
	std::vector<char> data; // 音频数据
};


class WAV2PCM
{
public:
	explicit WAV2PCM(char* in,char* out);

	// 读取 RIFF 块
	bool readRIFFHeader(FILE* file, RIFFHeader& riffHeader);
	// 读取 fmt 块
	bool readFMTChunk(FILE* file, FMTChunk& fmtChunk);
	// 读取 LIST 块
	bool readLISTChunk(FILE* file, LISTChunk& listChunk);
	// 读取 data 块
	bool readDataChunk(FILE* file, DataChunk& dataChunk);
	int start();


private:
	char* inFilename = nullptr;
	char* outFilename = nullptr;
	RIFFHeader riffHeader;
	FMTChunk fmtChunk;
	LISTChunk listChunk;
	DataChunk dataChunk;
};





#endif	//__WAVE_H__
