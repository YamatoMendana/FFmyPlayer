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
	char chunkid[4];	//���
	unsigned long chunksize;	//С��
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
	uint32_t chunkSize;   // �ļ���С�������� RIFF ��ʶ�����ļ���С�ֶΣ�
	char format[4];       // "WAVE"
};

struct FMTChunk {
	char subchunk1ID[4];  // "fmt "
	uint32_t subchunk1Size; // fmt ��Ĵ�С
	uint16_t audioFormat; // ��Ƶ��ʽ
	uint16_t numChannels; // ͨ����
	uint32_t sampleRate;  // ������
	uint32_t byteRate;    // ������
	uint16_t blockAlign;  // �����
	uint16_t bitsPerSample; // λ���
};

struct LISTChunk {
	char subchunk2ID[4];  // "LIST"
	uint32_t subchunk2Size; // LIST ��Ĵ�С
	char listType[4];     // "INFO"
	char isft[4];         // "ISFT"
	std::vector<char> isftData; // ISFT �������
};

struct DataChunk {
	char subchunk3ID[4];  // "data"
	uint32_t subchunk3Size; // ��Ƶ���ݵĴ�С
	std::vector<char> data; // ��Ƶ����
};


class WAV2PCM
{
public:
	explicit WAV2PCM(char* in,char* out);

	// ��ȡ RIFF ��
	bool readRIFFHeader(FILE* file, RIFFHeader& riffHeader);
	// ��ȡ fmt ��
	bool readFMTChunk(FILE* file, FMTChunk& fmtChunk);
	// ��ȡ LIST ��
	bool readLISTChunk(FILE* file, LISTChunk& listChunk);
	// ��ȡ data ��
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
