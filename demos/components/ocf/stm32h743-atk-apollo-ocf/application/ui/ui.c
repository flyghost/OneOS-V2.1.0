#include <oneos_config.h>

#ifdef OCF_AIRCONDITIONER
#include "airconditioner_ui.h"
#endif
#ifdef OCF_AIRPURIFIER
#include "airpurifier_ui.h"
#endif
#ifdef OCF_LIGHT
#include "light_ui.h"
#endif
#ifdef OCF_SMARTLOCK
#include "smartlock_ui.h"
#endif
#ifdef OCF_WATERHEATER
#include "waterheater_ui.h"
#endif
#ifdef OCF_CAMERA
#include "camera_ui.h"
#endif

void show(void)
{
#ifdef OCF_AIRCONDITIONER
    show_airconditioner();
#endif
#ifdef OCF_AIRPURIFIER
    show_airpurifier();
#endif
#ifdef OCF_LIGHT
    show_light();
#endif
#ifdef OCF_SMARTLOCK
    show_smartlock();
#endif
#ifdef OCF_WATERHEATER
    show_waterheater();
#endif
#ifdef OCF_CAMERA
    show_camera();
#endif
}

