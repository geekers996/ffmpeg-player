#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H
#include "avframequeue.h"
#include "AVSync.h"
#ifdef __cplusplus  ///
extern "C"
{
#include "SDL.h"
#include "libswresample/swresample.h"
}
#endif


typedef struct _AudioParams {
    int freq; //采样率
    AVChannelLayout ch_layout;  // 通道布局
    enum AVSampleFormat fmt; // 采样格式
} AudioParams;

class AudioOutput
{
public:
    AudioOutput(AVSync *avsync, const AudioParams &aduio_params, AVFrameQueue *frame_queue,  AVRational time_base);
    ~AudioOutput();
    int Init();
    int DeInit();

public:
    AVFrameQueue *frame_queue_ = NULL;
    AudioParams src_tgt_; // 解码后的源pcm格式
    AudioParams dst_tgt_; // SDL需要的格式

    struct SwrContext *swr_ctx_ = NULL;

    uint8_t *audio_buf1_ = NULL;
    uint32_t audio_buf1_size = 0;  // 真正分配的空间大小  audio_buf_size <= audio_buf1_size;
    uint8_t *audio_buf_ = NULL;
    uint32_t audio_buf_size = 0;  // 真正重采样后总共占用字节数
    uint32_t audio_buf_index = 0;

    AVRational time_base_ ;
    AVSync *avsync_ = NULL;
    double pts = 0;
};

#endif // AUDIOOUTPUT_H
