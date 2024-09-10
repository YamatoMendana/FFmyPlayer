#include "wave.h"




PCM2WAV::PCM2WAV(char* in, char* out, 
		int ch, int sr, int bs):
	inFilePath(in),outFilePath(out)
	, channels(ch), sampleRate(sr),bits(bs)
{
	pcmDataSize = getFileSize(inFilePath);
}

long PCM2WAV::getFileSize(char* filename)
{
	FILE* fp = fopen(filename, "r");
	if (!fp)
	{
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	fclose(fp);
	return size;
}

int PCM2WAV::start()
{
	FILE* fp, * out;
	WAV_HEADER header;
	WAV_FMT fmt;
	WAV_DATA data;

	fp = fopen(inFilePath, "rb");
	if (fp == nullptr)
	{
		cout << "fopen failed" << endl;
		return -1;
	}

	//1
	memcpy(header.chunkid, "RIFF", 4);
	long fileSize = 44 + pcmDataSize - 8;
	header.chunksize = fileSize;
	memcpy(header.format, "WAVE", 4);

	//2
	memcpy(fmt.subformat, "fmt", 4);
	fmt.subsize = 16;
	fmt.audioFormat = 1;
	fmt.numChannels = channels;
	fmt.sampleRate = sampleRate;
	fmt.byteRate = sampleRate * channels * bits / 8;
	fmt.blockAlign = channels * bits / 8;
	fmt.bitPerSample = bits;


	//3
	memcpy(data.data, "data", 4);
	data.dataSize = getFileSize((char*)inFilePath);

	out = fopen(outFilePath, "wb");
	if (out == nullptr)
	{
		cout << "fopen failed" << endl;
		return -1;
	}
	fwrite(&header, sizeof(header), 1, out);
	fwrite(&fmt, sizeof(fmt), 1, out);
	fwrite(&data, sizeof(data), 1, out);

	char* buf = (char*)malloc(512);
	int len = 0;
	while ((len = fread(buf, sizeof(char), 512, fp)) != 0)
	{
		fwrite(buf, sizeof(char), len, out);
	}

	free(buf);
	fclose(fp);
	fclose(out);


}

WAV2PCM::WAV2PCM(char* in, char* out):
	inFilename(in),outFilename(out)
{

}

bool WAV2PCM::readRIFFHeader(FILE* file, RIFFHeader& riffHeader)
{
	return fread(&riffHeader, sizeof(RIFFHeader), 1, file) == 1;
}

bool WAV2PCM::readFMTChunk(FILE* file, FMTChunk& fmtChunk)
{
	return fread(&fmtChunk, sizeof(FMTChunk), 1, file) == 1;
}

bool WAV2PCM::readLISTChunk(FILE* file, LISTChunk& listChunk)
{
	if (fread(&listChunk, 8, 1, file) != 1) {
		return false;
	}
	if (std::string(listChunk.subchunk2ID, 4) != "LIST") {
		return false;
	}
	if (fread(listChunk.listType, 4, 1, file) != 1) {
		return false;
	}
	if (std::string(listChunk.listType, 4) != "INFO") {
		return false;
	}
	if (fread(listChunk.isft, 4, 1, file) != 1) {
		return false;
	}
	if (std::string(listChunk.isft, 4) != "ISFT") {
		return false;
	}
	listChunk.isftData.resize(listChunk.subchunk2Size - 8); // ��ȡ ISFT �������
	return fread(listChunk.isftData.data(), listChunk.isftData.size(), 1, file) == 1;
}


bool WAV2PCM::readDataChunk(FILE* file, DataChunk& dataChunk)
{
	if (fread(&dataChunk, 8, 1, file) != 1) {
		return false;
	}
	if (std::string(dataChunk.subchunk3ID, 4) != "data") {
		return false;
	}
	dataChunk.data.resize(dataChunk.subchunk3Size); // ��ȡ��Ƶ����
	return fread(dataChunk.data.data(), dataChunk.data.size(), 1, file) == 1;
}

int WAV2PCM::start()
{
	FILE* input = fopen(inFilename, "rb");
	if (!input) {
		std::cerr << "�޷��������ļ�" << std::endl;
		return false;
	}

	if (!readRIFFHeader(input, riffHeader)) {
		std::cerr << "�޷���ȡ RIFF ��" << std::endl;
		fclose(input);
		return false;
	}

	if (!readFMTChunk(input, fmtChunk)) {
		std::cerr << "�޷���ȡ fmt ��" << std::endl;
		fclose(input);
		return false;
	}

	if (!readLISTChunk(input, listChunk)) {
		std::cerr << "�޷���ȡ LIST ��" << std::endl;
		fclose(input);
		return false;
	}

	if (!readDataChunk(input, dataChunk)) {
		std::cerr << "�޷���ȡ data ��" << std::endl;
		fclose(input);
		return false;
	}

	fclose(input);

	FILE* output = fopen(outFilename, "wb");
	if (!output) {
		std::cerr << "�޷�������ļ�" << std::endl;
		return false;
	}

	// д�� PCM ����
	size_t written = fwrite(dataChunk.data.data(), 1, dataChunk.data.size(), output);
	fclose(output);

	return written == dataChunk.data.size();
}
