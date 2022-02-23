#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include <string.h>
#include <stdio.h>

#include "user_audio.h"

#define CHALLEN			0
#define SAMPLERATE		1
#define VOLUME			3


#define MP_ADUIO_OPEN_CHECK(audio) 	MP_MACHINE_OPEN_CHECK_SECOND_LEVEL(audio, audio_dev_t, "audio");

const mp_obj_type_t mp_audio_type;

typedef struct _device_audio_obj_t {
    mp_obj_base_t base;
	audio_dev_t	  audio;
}device_audio_obj_t;

mp_obj_t mp_audio_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
    mp_arg_check_num(n_args, n_kw, 0, MP_OBJ_FUN_ARGS_MAX, true);
    device_info_t * audio = mpycall_device_find("audio");
    if (!audio){
        mp_raise_ValueError("audio could not find!\n");
    }
    device_audio_obj_t *self = m_new_obj(device_audio_obj_t);
    self->base.type = &mp_audio_type;
    self->audio.device = audio;
    return (mp_obj_t) self;
}

STATIC mp_obj_t mp_audio_init(mp_obj_t self_in)
{
    device_audio_obj_t *self = self_in;

    OPEN_MP_MACHINE_DEVICE(self->audio.device->open_flag,
                        self->audio.device->ops->open,
                        self->audio.device->owner.name);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(mp_audio_init_obj,  mp_audio_init);

STATIC mp_obj_t mp_audio_deinit (mp_obj_t self_in)
{
    device_audio_obj_t *self = (device_audio_obj_t *)self_in;

    CLOSE_MP_MACHINE_DEVICE(self->audio.device->open_flag, 
                        self->audio.device->ops->close, 
                        self->audio.device->owner.name);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(mp_audio_deinit_obj, mp_audio_deinit);


STATIC mp_obj_t mp_audio_volume (mp_obj_t self_in, mp_obj_t param)
{
	device_audio_obj_t *audio_boj = (device_audio_obj_t *)self_in;
	MP_ADUIO_OPEN_CHECK(&audio_boj->audio);
	
	self->volume = mp_obj_get_int(param);
	self->device->ops->ioctl(self, AUDIO_WRITE_VOLUME_CMD, 0);
	return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(mp_audio_set_volume_obj, mp_audio_volume);


STATIC mp_obj_t mp_audio_player(mp_obj_t self_in, mp_obj_t wr_buf)
{
    device_audio_obj_t *audio_boj = MP_OBJ_TO_PTR(self_in);
    if (!audio_boj)
    {
        mp_raise_ValueError("audio is null \n");
        return mp_const_none;
    }
    MP_ADUIO_OPEN_CHECK(&audio_boj->audio);

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(wr_buf, &bufinfo, MP_BUFFER_READ);

    //if (0 != self->device->ops->write(self, 0, bufinfo.buf, bufinfo.len)) {
    if (0 != self->device->ops->ioctl(self, AUDIO_PLAYER_START_CMD, bufinfo.buf)) 
    {
        mp_raise_ValueError("mp_audio_player failed!\n");
        return mp_const_none;
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(mp_audio_player_obj, mp_audio_player);


STATIC mp_obj_t player_continue(mp_obj_t self_in)
{
    device_audio_obj_t *audio_boj = MP_OBJ_TO_PTR(self_in);
	if (!audio_boj){
		mp_raise_ValueError("audio is null!\n");
		return mp_const_none;
	}
	
	MP_ADUIO_OPEN_CHECK(&audio_boj->audio);
	
    self->device->ops->ioctl(self, AUDIO_PLAYER_CONTINUE_CMD, 0);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(player_continue_obj, player_continue);

STATIC mp_obj_t player_stop(mp_obj_t self_in)
{
    device_audio_obj_t *audio_boj = MP_OBJ_TO_PTR(self_in);
	if (!audio_boj){
		mp_raise_ValueError("audio is null!\n");
		return mp_const_none;
	}
	MP_ADUIO_OPEN_CHECK(&audio_boj->audio);
	
    self->device->ops->ioctl(self, AUDIO_PLAYER_STOP_CMD, 0);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(player_stop_obj, player_stop);

STATIC const mp_rom_map_elem_t mp_module_audio_globals_table[] =
{
    { MP_ROM_QSTR(MP_QSTR_init), 	MP_ROM_PTR(&mp_audio_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), 	MP_ROM_PTR(&mp_audio_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_volume), 	MP_ROM_PTR(&mp_audio_set_volume_obj) },
	{ MP_ROM_QSTR(MP_QSTR_player), 	MP_ROM_PTR(&mp_audio_player_obj) },
	{ MP_ROM_QSTR(MP_QSTR_stop), 	MP_ROM_PTR(&player_stop_obj) },
	{ MP_ROM_QSTR(MP_QSTR_start), 	MP_ROM_PTR(&player_continue_obj) },

};

STATIC MP_DEFINE_CONST_DICT(device_audio_locals_dict, mp_module_audio_globals_table);

const mp_obj_type_t mp_audio_type =
{
    { &mp_type_type },
    .name = MP_QSTR_AUDIO,
    .make_new = mp_audio_make_new,
    .locals_dict = (mp_obj_dict_t *)&device_audio_locals_dict,
};


