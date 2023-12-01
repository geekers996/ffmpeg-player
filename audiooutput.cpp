#include "audiooutput.h"

AudioOutput::AudioOutput(AVSync *avsync, const AudioParams &aduio_params, AVFrameQueue *frame_queue,  AVRational time_base)
    : avsync_(avsync), src_tgt_(aduio_params), frame_queue_(frame_queue), time_base_(time_base)
{
}

AudioOutput::~AudioOutput()
{
}


void sdl_audio_callback(void *userdata, Uint8 * stream, int len)
{
    AudioOutput *audio_output = (AudioOutput *)userdata;
    printf("sdl_audio_callback len: %d\n", len);
    while(len > 0) {
        if(audio_output->audio_buf_index == audio_output->audio_buf_size) {
            // 1. 读取pcm的数据
            audio_output->audio_buf_index = 0;
            AVFrame *frame = audio_output->frame_queue_->Pop(2);
            audio_output->pts = frame->pts * av_q2d(audio_output->time_base_);
            if(frame) {
                // 2. 做重采样
                // 2.1初始化重采样器
                if(( (frame->format != audio_output->dst_tgt_.fmt)      // 采样格式的判断
                     || (frame->sample_rate != audio_output->dst_tgt_.freq) //采样率相关
                     || av_channel_layout_compare(&frame->ch_layout, &audio_output->dst_tgt_.ch_layout) != 0)
                   && (!audio_output->swr_ctx_)) {
                    swr_alloc_set_opts2(&audio_output->swr_ctx_,
                                        &audio_output->dst_tgt_.ch_layout,
                                        audio_output->dst_tgt_.fmt,
                                        audio_output->dst_tgt_.freq,
                                        &frame->ch_layout,
                                        (enum AVSampleFormat)frame->format,
                                        frame->sample_rate,
                                        0, NULL);
                    if(!audio_output->swr_ctx_ || swr_init(audio_output->swr_ctx_) < 0) {
                        printf("swr_init failed");
                        if(audio_output->swr_ctx_) {
                            swr_free(&audio_output->swr_ctx_);
                        }
                        return;
                    }
                }
                if(audio_output->swr_ctx_) {
                    // 需要重采样
                    const uint8_t **in = (const uint8_t **)frame->extended_data;
                    uint8_t **out = &audio_output->audio_buf1_;
                    int out_samples = frame->nb_samples * audio_output->dst_tgt_.freq / frame->sample_rate + 256;
                    int out_bytes = av_samples_get_buffer_size(NULL,
                                    audio_output->dst_tgt_.ch_layout.nb_channels,
                                    out_samples,
                                    audio_output->dst_tgt_.fmt, 0) ;
                    if(out_bytes < 0) {
                        printf("av_samples_get_buffer_size failed");
                        return;
                    }
                    // audio_output->audio_buf1_size < out_bytes,内部重新分配数据
                    av_fast_malloc(&audio_output->audio_buf1_, &audio_output->audio_buf1_size, out_bytes);
                    int len2 = swr_convert(audio_output->swr_ctx_, out, out_samples, in, frame->nb_samples);
                    //                swr_convert(audio_output->swr_ctx_, );
                    if(len2 < 0) {
                        printf("swr_convert failed\n");
                        return;
                    }
                    audio_output->audio_buf_size = av_samples_get_buffer_size(NULL,
                                                   audio_output->dst_tgt_.ch_layout.nb_channels,
                                                   len2,
                                                   audio_output->dst_tgt_.fmt, 0);
                    audio_output->audio_buf_ = audio_output->audio_buf1_;
                } else { //没有重采样
                    int out_bytes = av_samples_get_buffer_size(NULL,
                                    frame->ch_layout.nb_channels,
                                    frame->nb_samples,
                                    (enum AVSampleFormat)frame->format, 0);
                    av_fast_malloc(&audio_output->audio_buf1_, &audio_output->audio_buf1_size, out_bytes);
                    audio_output->audio_buf_ = audio_output->audio_buf1_;
                    audio_output->audio_buf_size = out_bytes;
                    memcpy(audio_output->audio_buf_, frame->extended_data[0], out_bytes);
                }
                av_frame_free(&frame);
            } else {
                audio_output->audio_buf_ = NULL;
                audio_output->audio_buf_size = 512;
            }
        } // end of  if(audio_output->audio_buf_index < audio_output->audio_buf_size)
        // 3. 拷贝数据到stream buffer
        int len3 =  audio_output->audio_buf_size - audio_output->audio_buf_index;
        //        len -= xx;
        if(len3 > len) {
            len3 = len;
        }
        if(!audio_output->audio_buf_) {
            memset(stream, 0, len3);
        } else {
            memcpy(stream, audio_output->audio_buf_ + audio_output->audio_buf_index, len3);
        }
        len -= len3;
        audio_output->audio_buf_index += len3;
        stream += len3;
        printf("len:%d, audio_buf_index:%d, %d\n", len, audio_output->audio_buf_index,
               audio_output->audio_buf_size);
    }
    printf("audio pts: %0.3lf\n", audio_output->pts);
    audio_output->avsync_->SetClock(audio_output->pts);
}
int AudioOutput::Init()
{
    if(SDL_Init(SDL_INIT_AUDIO) != 0) {
        printf("SDL_Init failed\n");
        return -1;
    }
    SDL_AudioSpec wanted_spec;
    wanted_spec.channels = 2;
    wanted_spec.freq = src_tgt_.freq;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.silence = 0;
    wanted_spec.callback = sdl_audio_callback;
    wanted_spec.userdata = this;
    wanted_spec.samples = 1024; // 2*2*1024 = 4096,   1024/freq
    int ret = SDL_OpenAudio(&wanted_spec, NULL);
    if(ret != 0) {
        printf("SDL_OpenAudio failed\n");
        return -1;
    }
    av_channel_layout_default(&dst_tgt_.ch_layout, wanted_spec.channels);
    dst_tgt_.fmt = AV_SAMPLE_FMT_S16;
    dst_tgt_.freq = wanted_spec.freq;
    SDL_PauseAudio(0);
    printf("AudioOutput::Init() finish\n");
    return 0;
}

int AudioOutput::DeInit()
{
    SDL_PauseAudio(1);
    SDL_CloseAudio();
    printf("AudioOutput::DeInit() finish\n");
    return 0;
}
