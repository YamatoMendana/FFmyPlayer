#ifndef __PLAYER_DISPLAY_H__
#define __PLAYER_DISPLAY_H__

#include <thread>
#include <mutex>
#include <condition_variable>
#include <unordered_map>

#include "SDL.h"

#include "playerClock.h"
#include "avFrameList.h"
#include "decoder.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavcodec/avfft.h"
#include "libavutil/pixfmt.h"
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
#include "libavutil/bprint.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libswresample/swresample.h"

}
using namespace std;
//��ʾ������Ϣ���¼��ص���������
using streamClosehandler = std::function<void()>;

void sigterm_handler(int sig) { exit(1); }

class PlayerDisplay 
{
public:
	explicit PlayerDisplay();
	~PlayerDisplay();

	int stream_open(const char* filename);

	void stream_close();

	void get_default_windowSize(int* width,int* height, AVRational* sar);
private:
	void stream_component_close(int stream_index);
	int stream_component_open(int stream_index);

	
	int check_stream_specifier(AVFormatContext* s, AVStream* st, const char* spec);
	AVDictionary* filter_codec_opts(AVDictionary* opts, enum AVCodecID codec_id,
		AVFormatContext* s, AVStream* st, const AVCodec* codec);
	AVDictionary** setup_find_stream_info_opts(AVFormatContext* s, AVDictionary* codec_opts);

	int is_realtime(AVFormatContext* s);

	
	void audioStreamClose();
	void videoStreamClose();
	void subtitleStreamClose();
	streamClosehandler getHandler(int codec_type);

	int stream_has_enough_packets(AVStream* st, int stream_id, AvPacketList* list);
	void stream_seek(int64_t pos, int64_t rel, int by_bytes);
	void stream_toggle_pause();
	void step_to_next_frame();

	void read_thread();

public:
	inline int get_Abort_request() { return bAbort_request; }
private:
	enum ShowMode {
		SHOW_MODE_NONE = -1, 
		SHOW_MODE_VIDEO = 0, 
		SHOW_MODE_WAVES, 
		SHOW_MODE_RDFT, 
		SHOW_MODE_NB
	} show_mode;

	// ͬ��������������Ƶͬ��
	PlayerClock audclk;
	PlayerClock vidclk;
	PlayerClock extclk;

	// �߳����
	std::mutex mutex;	//������
	std::condition_variable cond;	//��������

	bool bAbort_request;			//�߳��жϱ�־

	//�ֵ�
	AVDictionary* sws_dict;		//�洢��ͼ������ѡ��
	AVDictionary* swr_opts;		//�洢��Ƶ�ز���ѡ��
	AVDictionary* format_opts;	//�洢����/�����ʽѡ��
	AVDictionary* codec_opts;	//�洢�������ѡ��

	// �ļ����
	AVFormatContext* pFmtCtx;	// ��ʽ������
	AVInputFormat* pIformat;	//�洢�����ļ��ĸ�ʽ��Ϣ
	int nEof;					// �ļ�������־
	char* pFilename;			// �ļ���

	// �����
	AVStream* audio_st;
	AVStream* video_st;
	AVStream* subtitle_st;
	int nVideo_stream;     // ��Ƶ������
	int nAudio_stream;     // ��Ƶ������
	int nSubtitle_stream;  // ��Ļ������

	const char* wanted_stream_spec[AVMEDIA_TYPE_NB] = { 0 };	//�洢ָ�������淶
	int nSt_index[5];

	// ���������
	Decoder auddec;		// ��Ƶ������������
	Decoder viddec;		// ��Ƶ������������
	Decoder subdec;		// ��Ļ������������

	// ֡����
	AvFrameList pictq;  // ��Ƶ֡����
	AvFrameList sampq;  // ��Ƶ֡����
	AvFrameList subpq;  // ��Ļ֡����

	// ������
	AvPacketList videoq;	// ��Ƶ������
	AvPacketList audioq;	// ��Ƶ������
	AvPacketList subtitleq; // ��Ļ������

	// �����ٶȿ���
	int64_t seek_pos;	// ��תλ��
	int seek_req;		// ��ת�����־
//��λ��־��AVSEEK_FLAG_BACKWARD: ����������ļ�֡
// AVSEEK_FLAG_BYTE: ���ֽڶ�λ
// AVSEEK_FLAG_ANY: ��λ������֡(��һ���Ǽ�֡��
// AVSEEK_FLAG_FRAME: ��֡��λ��
	int seek_flags;		// ��ת��־
	int64_t nSeek_rel;	//������תƫ����

	// ����״̬
	int nPaused;       // ��ͣ״̬
	int nLast_paused;	// ��һ����ͣ״̬
	int nAttachments_req; // ������и���
	int64_t duration = AV_NOPTS_VALUE;

	//��Ƶ���
	int nAv_sync_type;					// ����Ƶͬ������
	int nAudio_hw_buf_size;				// ��ƵӲ����������С
	uint8_t* pAudio_buf;				// ��Ƶ������
	uint8_t* pAudio_buf1;				// ������Ƶ������
	unsigned int nAudio_buf_size;		// ��Ƶ��������С���ֽڣ�
	unsigned int nAudio_buf1_size;		// ������Ƶ��������С
	int nAudio_buf_index;				// ��Ƶ�������������ֽڣ�
	int nAudio_write_buf_size;			// ��Ƶд�뻺������С
	int nAudio_volume;					// ��Ƶ����
	bool bMuted;						// �Ƿ���

	struct AudioParams struAudio_src;			// ��ƵԴ����
	struct AudioParams struAudio_filter_src;	// ��Ƶ�˲���Դ����
	struct AudioParams struAudio_tgt;			// ��ƵĿ�����
	struct SwrContext* pSwr_ctx;				// ��Ƶ�ز���������

	int nFrame_drops_early;	// ���ڶ�֡��
	int nFrame_drops_late;	// ���ڶ�֡��

	//��Ƶʱ�����
	double nAudio_clock;				// ��Ƶʱ��
	int nAudio_clock_serial;			// ��Ƶʱ�����к�
	double nAudio_diff_cum;				// ��Ƶ�����ۻ�
	double nAudio_diff_avg_coef;		// ��Ƶ����ƽ��ϵ��
	double nAudio_diff_threshold;		// ��Ƶ������ֵ
	int nAudio_diff_avg_count;			// ��Ƶ����ƽ������

	//��Ƶ���
	int nForce_refresh;		// ǿ��ˢ��
	int nRead_pause_return;	// ��ȡ��ͣ����ֵ
	int nRealtime;			// �Ƿ�ʵʱ��
	int nLast_i_start;		// ���һ�� I ֡����ʼλ��

	double nFrame_timer;					// ֡��ʱ��
	double nFrame_last_returned_time;	// ��һ֡����ʱ��
	double nFrame_last_filter_delay;		// ��һ֡�˲��ӳ�
	double nMax_frame_duration;			// ���֡����ʱ��

	int nVfilter_idx;
	AVFilterContext* pIn_video_filter;   // ��Ƶ���ĵ�һ���˲���
	AVFilterContext* pOut_video_filter;  // ��Ƶ�������һ���˲���
	AVFilterContext* pIn_audio_filter;   // ��Ƶ���ĵ�һ���˲���
	AVFilterContext* pOut_audio_filter;  // ��Ƶ�������һ���˲���
	AVFilterGraph* pAgraph;				// ��Ƶ�˲���ͼ

	int nLast_video_stream;		//���һ����Ƶ��
	int nLast_audio_stream;		//���һ����Ƶ��
	int nLast_subtitle_stream;	//���һ����Ļ��

	int nStep;	// ����

	//��Ļ���
	struct SwsContext* pSub_convert_ctx;	// ��Ļת��������

	//����
	int16_t sample_array[SAMPLE_ARRAY_SIZE];	// ��������
	int nSample_array_index;

	RDFTContext* pRdft;		// ���ٸ���Ҷ�任������
	int nRdft_bits;			// ���ٸ���Ҷ�任λ��
	FFTSample* pRdft_data;	// ���ٸ���Ҷ�任����

	bool bFind_stream_info = true;
	


	int nXpos;	// X ����λ��
	// ��Ƶ��ȡ��߶ȡ����Ͻ� X �� Y ����
	int nWidth, nHeight, nXleft, nYtop;
	double nLast_vis_time;	// �����ӻ�ʱ��

	//����������
	unordered_map<int, streamClosehandler> um_stCloseHandlerMap;


};

int decode_interrupt_cb(void* ctx)
{
	PlayerDisplay* is = (PlayerDisplay*)ctx;
	return is->get_Abort_request();
}






#endif // __PLAYER_DISPLAY_H__
