#include <os_task.h>
#include <device.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <audio/audio.h>
#include <shell.h>

#define BUFSZ   (OS_AUDIO_REPLAY_MP_BLOCK_SIZE)
#define SOUND_DEVICE_NAME    "audio0"    /* Audio device name */

//#define AUDIO_WRITE_NONBLOCK

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

char media_name[24] = {0};

void wavplay_sample_task(void *parameter)
{
    int fd = -1;
    int count, index;
    int length;
    uint8_t *buffer = NULL;
    struct wav_info *info = NULL;
    struct os_audio_caps caps = {0};
    os_device_t *snd_dev;

    fd = open(media_name, O_RDONLY);
    if (fd < 0)
    {
        os_kprintf("open file failed!\r\n");
        goto __exit;
    }

    buffer = os_calloc(1, BUFSZ);
    if (buffer == OS_NULL)
        goto __exit;

    info = (struct wav_info *) os_calloc(1, sizeof * info);
    if (info == OS_NULL)
        goto __exit;

    if (read(fd, &(info->header), sizeof(struct RIFF_HEADER_DEF)) <= 0)    
        goto __exit;
    if (read(fd, &(info->fmt_block),  sizeof(struct FMT_BLOCK_DEF)) <= 0)
        goto __exit;
    if (read(fd, &(info->data_block), sizeof(struct DATA_BLOCK_DEF)) <= 0)
        goto __exit;

    os_kprintf("wav information:\r\n");
    os_kprintf("samplerate %d\r\n", info->fmt_block.wav_format.SamplesPerSec);
    os_kprintf("channel %d\r\n", info->fmt_block.wav_format.Channels);

    snd_dev = os_device_find(SOUND_DEVICE_NAME);
    OS_ASSERT(snd_dev != OS_NULL);

    os_device_open(snd_dev);

    /* parameter settings */
    caps.config_type = AUDIO_PARAM_CMD;
    caps.udata.config.samplerate = info->fmt_block.wav_format.SamplesPerSec;
    caps.udata.config.channels   = info->fmt_block.wav_format.Channels;
    os_device_control(snd_dev, AUDIO_CTL_CONFIGURE, &caps);
    caps.config_type = AUDIO_VOLUME_CMD;
    caps.udata.value = 35;
    os_device_control(snd_dev, AUDIO_CTL_CONFIGURE, &caps);

    while (1)
    {
        length = read(fd, buffer, BUFSZ);
        
        if (length <= 0)
            break;

        index = 0;

        while (index < length)
        {
#ifndef AUDIO_WRITE_NONBLOCK
            count = os_device_write_block(snd_dev, 0, buffer + index, length - index);
#else
            count = os_device_write_nonblock(snd_dev, 0, buffer + index, length - index);
#endif

            if (count <= 0)
                continue;
            
            index += count;
        }
    }
    
    os_device_close(snd_dev);

    os_kprintf("end of audio playing!\r\n");

__exit:

    if (fd >= 0)
        close(fd);

    if (buffer)
        os_free(buffer);

    if (info)
        os_free(info);

    media_name[0] = 0;
}

int wavplay_sample(int argc, char **argv)
{
    os_task_t *task;

    if (argc != 2)
    {
        os_kprintf("Usage:\r\n");
        os_kprintf("wavplay_sample song.wav\r\n");
        return 0;
    }

    if (media_name[0] != 0)
    {
        os_kprintf("player thread running...\r\n");
        return 0;
    }
    
    snprintf(media_name, sizeof(media_name) - 1, "%s", argv[1]);
    
    task = os_task_create("media_player", wavplay_sample_task, NULL, 4096, 5);
    OS_ASSERT(task);
    os_task_startup(task);
    return 0;
}


SH_CMD_EXPORT(wav_player, wavplay_sample, "play wav file in task");
