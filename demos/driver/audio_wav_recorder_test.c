#include <os_task.h>
#include <device.h>
#include <vfs_posix.h>
#include <audio/audio.h>
#include <shell.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFSZ   OS_AUDIO_RECORD_MP_BLOCK_SIZE                                           
#define RECORDER_DEVICE_NAME    "audio1"    /* Audio device name */
#define PLAYER_DEVICE_NAME      "audio0"    /* Audio device name */

#define RECORD_TIME_MS      5000
#define RECORD_SAMPLERATE   OS_AUDIO_SAMPLERATE
#define RECORD_CHANNEL      OS_AUDIO_CHANNEL
#define RECORD_CHUNK_SZ     OS_AUDIO_RECORD_FIFO_SIZE   /*((OS_AUDIO_SAMPLERATE * RECORD_CHANNEL * 2) * 20 / 1000) = 20ms data size*/

struct wav_header
{
    char  riff_id[4];              /* "RIFF" */
    int   riff_datasize;           /* RIFF chunk data size,exclude riff_id[4] and riff_datasize,total - 8 */
    char  riff_type[4];            /* "WAVE" */
    char  fmt_id[4];               /* "fmt " */
    int   fmt_datasize;            /* fmt chunk data size,16 for pcm */
    short fmt_compression_code;    /* 1 for PCM */
    short fmt_channels;            /* 1(mono) or 2(stereo) */
    int   fmt_sample_rate;         /* samples per second */
    int   fmt_avg_bytes_per_sec;   /* sample_rate * channels * bit_per_sample / 8 */
    short fmt_block_align;         /* number bytes per sample, bit_per_sample * channels / 8 */
    short fmt_bit_per_sample;      /* bits of each sample(8,16,32). */
    char  data_id[4];              /* "data" */
    int   data_datasize;           /* data chunk size,pcm_size - 44 */
};

static void wavheader_init(struct wav_header *header, int sample_rate, int channels, int datasize)
{
    memcpy(header->riff_id, "RIFF", 4);
    header->riff_datasize = datasize + 44 - 8;
    memcpy(header->riff_type, "WAVE", 4);
    memcpy(header->fmt_id, "fmt ", 4);
    header->fmt_datasize = 16;
    header->fmt_compression_code = 1;
    header->fmt_channels = channels;
    header->fmt_sample_rate = sample_rate;
    header->fmt_bit_per_sample = 16;
    header->fmt_avg_bytes_per_sec = header->fmt_sample_rate * header->fmt_channels * header->fmt_bit_per_sample / 8;
    header->fmt_block_align = header->fmt_bit_per_sample * header->fmt_channels / 8;
    memcpy(header->data_id, "data", 4);
    header->data_datasize = datasize;
}

static void wavrecord_sample_task(void *parameter)
{
    int fd = -1;
    int length, total_length = 0;
    uint8_t *buffer = NULL;
    struct wav_header header;
    os_device_t *snd_dev1;

    char *recorder_name = parameter;

    fd = open(recorder_name, O_WRONLY | O_CREAT);
    if (fd < 0)
    {
        os_kprintf("open file failed!\r\n");
        goto __exit;
    }

    buffer = os_calloc(1, RECORD_CHUNK_SZ);
    if (buffer == OS_NULL)
        goto __exit;

    write(fd, &header, sizeof(struct wav_header));
    
    /* open the recorder device */
    snd_dev1 = os_device_find(RECORDER_DEVICE_NAME);
    OS_ASSERT(snd_dev1 != OS_NULL);

    os_device_open(snd_dev1);
    
    while (1)
    {
        length = os_device_read_nonblock(snd_dev1, 0, buffer, RECORD_CHUNK_SZ);

        if (length)
        {
            write(fd, buffer, length);
            total_length += length;
        }

        if ((total_length / RECORD_CHUNK_SZ) >  (RECORD_TIME_MS / 20))
            break;
    }

    wavheader_init(&header, RECORD_SAMPLERATE, RECORD_CHANNEL, total_length);
    lseek(fd, 0, SEEK_SET);
    write(fd, &header, sizeof(struct wav_header));

    os_device_close(snd_dev1);

    os_kprintf("end of audio recording!\r\n");

__exit:

    if (fd >= 0)
        close(fd);

    if (buffer)
        os_free(buffer);
}

int wavrecord_sample(int argc, char **argv)
{
    os_task_t *task;

    if (argc != 2)
    {
        os_kprintf("Usage:\r\n");
        os_kprintf("wavplay_sample song.wav\r\n");
        return 0;
    }
    
    task = os_task_create("media_player", wavrecord_sample_task, argv[1], 4096, 18);
    OS_ASSERT(task);
    os_task_startup(task);
    
    return 0;
}

SH_CMD_EXPORT(wav_recorder, wavrecord_sample, "record wav file in task");

