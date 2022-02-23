#include <stdio.h>
#include <string.h>
#include <os_memory.h>
#include <unistd.h>
#include <fcntl.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"
#include "usr_misc.h"
#include "usr_general.h"
#include <device.h>
#include <drv_cfg.h>
#include <vfs_posix.h>
#include "user_audio.h"
#include <audio/audio.h>


#define BUFSZ   				OS_AUDIO_REPLAY_MP_BLOCK_SIZE   
#define AUDIO_TASK_SIZE  		BUFSZ*2
#define AUDIO_TASK_PRIORITY		SHELL_TASK_PRIORITY -2
#define SOUND_DEVICE_NAME    	MICROPYTHON_DEVICE_AUDIO_NAME    /* Audio device name */

volatile int G_player_state = AUDIO_PLAYER_IDLE_CMD;

static void set_player_state_continue(void);

static int audio_player(void *dev, void *buffer);

static int audio_open(const char *dev_name)
{
    G_player_state = AUDIO_PLAYER_IDLE_CMD;
    mp_log("Open audio device[%s].", SOUND_DEVICE_NAME);
    return 0;
}

static int audio_close(const char *dev_name)
{
    G_player_state= AUDIO_DEINIT_FLAG;
    mp_log("Close audio device[%s].", SOUND_DEVICE_NAME);
    return 0;
}

static void set_player_state_stop(void)
{
    G_player_state = AUDIO_PLAYER_STOP_CMD;
}

static void set_player_state_continue(void)
{
    mp_log("Set player status [Continue]. ");
    G_player_state = AUDIO_PLAYER_CONTINUE_CMD;
}

static int is_audio_device_open(void)
{
    if ((G_player_state == AUDIO_PLAYER_CONTINUE_CMD) 
        || (G_player_state == AUDIO_PLAYER_STOP_CMD))
    {
        return 1;
    }
    
    return 0; 
}

static void * malloc_audio_info(int fd)
{
	struct wav_info * info;
	info = (struct wav_info *)os_malloc(sizeof(struct wav_info));
    if (info == OS_NULL)
        return OS_NULL;

    if (read(fd, &(info->header), sizeof(struct RIFF_HEADER_DEF)) <= 0){    
        goto __exit;
	}
    if (read(fd, &(info->fmt_block),  sizeof(struct FMT_BLOCK_DEF)) <= 0){
        goto __exit;
	}
    if (read(fd, &(info->data_block), sizeof(struct DATA_BLOCK_DEF)) <= 0){
        goto __exit;
	}
	mp_log("samplerate %d", info->fmt_block.wav_format.SamplesPerSec);
    mp_log("channel %d", info->fmt_block.wav_format.Channels);
	return info;
	
__exit:
	os_free(info);
    
	return OS_NULL;
}

static os_device_t * set_audio_parameters(audio_dev_t *audio_file, void *caps_info, void *wav_info)
{
    struct os_audio_caps *caps = caps_info;
    struct wav_info *info = wav_info;
    os_device_t * snd_dev = os_device_find(SOUND_DEVICE_NAME);
    int ret = -1;
    
    if(snd_dev == OS_NULL)
    {
        mp_raise_ValueError("Can not find audio device! ");
        return NULL;
    }

    /* parameter settings */                         
    caps->config_type = AUDIO_PARAM_CMD; 
    caps->udata.config.samplerate = info->fmt_block.wav_format.SamplesPerSec; 
    caps->udata.config.channels   = info->fmt_block.wav_format.Channels;  
    ret = os_device_control(snd_dev, AUDIO_CTL_CONFIGURE, caps);

    audio_file->channels = caps->udata.config.channels;
    audio_file->samplerate = caps->udata.config.samplerate;
    mp_log("Set audio config[channels: %d, samplerate: %d] result[%d]",
            caps->udata.config.channels, caps->udata.config.samplerate, ret);
    
    caps->config_type = AUDIO_VOLUME_CMD;
    caps->udata.value = audio_file->volume;
    ret = os_device_control(snd_dev, AUDIO_CTL_CONFIGURE, caps); 
    mp_log("Set audio volume[%d] result[%d].", caps->udata.value, ret);
    
    return snd_dev;
}

static void audio_player_handle(os_device_t *snd_dev, int fd, int length, uint8_t *buffer)
{
    int index = 0;
    int count = 0;
    
    while (index < length)
    {
        /* Deinit audio obj while playing, need return before write audio device. */
        if (G_player_state == AUDIO_DEINIT_FLAG)
        {
            mp_log("Return in playing status. index: %d, count: %d.", index, count);
            return;
        }
        count = os_device_write_block(snd_dev, 0, buffer + index, length - index);
        if (count <= 0)
        {
            os_task_msleep(10);
            continue;
        }
        
        index += count;
    }

    return;
}

static void audio_player_entry(void *data)
{
    int fd = -1;
    uint8_t *buf = NULL;
    struct wav_info *info = NULL;
    struct os_audio_caps caps = {0};
    audio_dev_t *audio_file = data;
    os_device_t *snd_dev = NULL;
    int length = 0;
    int count = 0;
    
    if(NULL ==data)
    {
        mp_err("Can not operate the audio device, Please open it at first.\n");
        return ;
    }

    fd = open(audio_file->file, O_RDONLY);
    if (fd < 0)
    {
        mp_err("Failed to open %s file!", audio_file->file);
        goto __exit;
    }
    
    if ((buf = os_calloc(1, BUFSZ)) == OS_NULL  || (info = malloc_audio_info(fd)) == OS_NULL)
    {
        mp_err("No memory for play %s!", audio_file->file);
        goto __exit;
    }
    
    if (mpy_usr_driver_open(SOUND_DEVICE_NAME))
    {
        goto __exit;
    }
    
    snd_dev = set_audio_parameters(audio_file, &caps, info);
    set_player_state_continue();
    while (1)
    {
        if (G_player_state == AUDIO_PLAYER_CONTINUE_CMD)
        {
            length = read(fd, buf, BUFSZ);  
            if (length <= 0)
            {
                mp_log("Read audio file over.");
                break;
            }
            audio_player_handle(snd_dev, fd, length, buf);
        } 
        else if(G_player_state == AUDIO_DEINIT_FLAG)
        {
            mp_log("Close the audio device.");
            break;
        } 
        else 
        {
            if ((count % 1000 == 0) &&(G_player_state == AUDIO_PLAYER_STOP_CMD))
            {
                mp_log("Pausing the audio, please start it again.");
            }
            os_task_msleep(100);
            count++;
        }
    }

    mpy_usr_driver_close(SOUND_DEVICE_NAME);
__exit:
    
    if (fd >= 0)
    {
        close(fd);
    }
    if (buf)
    {
        os_free(buf);
    }
    
    if (info)
    {
        os_free(info);
    }
    
    G_player_state = AUDIO_PLAYER_IDLE_CMD;
    mp_log("Audio player operate over.");
    return ;
}


static int audio_player(void *dev, void *buffer)
{
    os_task_t  *audio_task;
    audio_dev_t *audio_file = (audio_dev_t *)dev;

    audio_file->file = buffer;
    if (G_player_state != AUDIO_PLAYER_IDLE_CMD)
    {
        mp_raise_ValueError("Player is busy!\n");
        return 0;
    }
    if (NULL == audio_file->device)
    {
        mp_raise_ValueError("Audio device is NULL!\n");
        return 0;
    }
    
    mp_log("Start the task to play file:%s", audio_file->file);
    audio_task = os_task_create("audio_player", audio_player_entry, audio_file, AUDIO_TASK_SIZE, 5);
    if (audio_task != OS_NULL)
    {
        os_task_startup(audio_task);
    }
    else
    {
        mp_err("Create task failed!\n");
    }

    return 0;
}

static int audio_ioctl(void *dev, int cmd, void *arg)
{
    struct os_audio_caps caps={0};
    audio_dev_t * audio = (audio_dev_t *)dev;
    os_device_t *audio_dev = NULL;
    
    audio_dev = os_device_find(SOUND_DEVICE_NAME);
    if (NULL == audio_dev)
    {
        mp_err("Find device[%s] failed.", SOUND_DEVICE_NAME);
        return MP_ERROR;
    }

    switch (cmd)
    {
    case AUDIO_WRITE_VOLUME_CMD: 
        if (is_audio_device_open())
        {
            caps.config_type = AUDIO_VOLUME_CMD;
            caps.udata.value = audio->volume;
            mp_log("volume %d", audio->volume);
            os_device_control(audio_dev, AUDIO_CTL_CONFIGURE, &caps);
        }
        return 0;
    case AUDIO_PLAYER_START_CMD:
        mp_log("Start playing song!");
        audio_player(dev, arg);
        return 0;
    case AUDIO_PLAYER_STOP_CMD:
        mp_log("Stop playing song!");
        set_player_state_stop();
        return 0;
    case AUDIO_PLAYER_CONTINUE_CMD:
        mp_log("Continue playing song!");
        set_player_state_continue();
        return 0;
    default:
        mp_raise_ValueError("the cmd is wrong, please check!\n");
        return -1;
    }
}

struct operate audio_ops = {
    .open =  audio_open,
    .ioctl = audio_ioctl,
    .close = audio_close,
};

static int audio_register(void)
{
	device_info_t * audio = (device_info_t *)os_malloc(sizeof(device_info_t));
	
	if(NULL == audio)
	{
		mp_err("malloc mem failed!");
		return -1;
	}
    memset(audio, 0, sizeof(device_info_t));
	

	audio->owner.name = "audio";
	audio->owner.type = DEV_AUDIO;
	
	audio->ops = &audio_ops;
    

	mpycall_device_add(audio);

	return 0;
}

OS_CMPOENT_INIT(audio_register, OS_INIT_SUBLEVEL_LOW);




