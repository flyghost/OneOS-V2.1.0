#ifndef __USER_BEEP_H__
#define __USER_BEEP_H__

#include "modmachine.h"
#include "model_device.h"

#define AUDIO_DEINIT_FLAG			MP_MACHINE_DEINIT_FLAG
#define AUDIO_INIT_FLAG				MP_MACHINE_INIT_FLAG
#define AUDIO_WRITE_FLAG			0x02

#define AUDIO_WRITE_VOLUME_CMD		1
#define AUDIO_WRITE_PARAM_CMD		2
#define AUDIO_PLAYER_IDLE_CMD 		3
#define AUDIO_PLAYER_STOP_CMD 		4
#define AUDIO_PLAYER_CONTINUE_CMD	5
#define AUDIO_PLAYER_START_CMD      6


struct RIFF_HEADER_DEF
{
    char riff_id[4];     /* 'R','I','F','F' */
    uint32_t riff_size;
    char riff_format[4]; /* 'W','A','V','E' */
};



struct WAVE_FORMAT_DEF
{
    uint16_t FormatTag;
    uint16_t Channels;
    uint32_t SamplesPerSec;
    uint32_t AvgBytesPerSec;
    uint16_t BlockAlign;
    uint16_t BitsPerSample;
};

struct FMT_BLOCK_DEF
{
    char fmt_id[4];    /* 'f','m','t',' ' */
    uint32_t fmt_size;
    struct WAVE_FORMAT_DEF wav_format;
};

struct DATA_BLOCK_DEF
{
    char data_id[4];     /* 'R','I','F','F' */
    uint32_t data_size;
};

struct wav_info
{
    struct RIFF_HEADER_DEF header;
    struct FMT_BLOCK_DEF   fmt_block;
    struct DATA_BLOCK_DEF  data_block;
};


typedef struct _audio
{
	uint32_t volume;
	uint32_t samplerate;
    uint16_t channels;
    uint16_t samplebits;
	void *	file;
    device_info_t *device;
}audio_dev_t;



#endif


