/*
This file is part of CanFestival, a library implementing CanOpen Stack.

Copyright (C): Edouard TISSERANT and Francis DUPIN
AT91 Port: Peter CHRISTEN

See COPYING file for copyrights details.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#ifndef __CAN_CANFESTIVAL__
#define __CAN_CANFESTIVAL__

#include "cf_applicfg.h"
#include "cf_data.h"
#include "cf_can_driver.h"
#include "cf_timers_driver.h"

// ---------  to be called by user app ---------
void canopen_init(void);
CAN_PORT canopen_portopen(s_BOARD *board);
int canopen_node_init(CAN_PORT const canport, CO_Data *const d,const UNS8 node_id);
CO_Data **get_canopen_nodelist(void);
UNS8 get_canopen_nodenum(void);
UNS8 canSend(CAN_PORT notused, Message *m);

extern void canfestival_heartbeatError(CO_Data *d, UNS8 heartbeatID);
extern void canfestival_initialisation(CO_Data *d);
extern void canfestival_preOperational(CO_Data *d);
extern void canfestival_operational(CO_Data *d);
extern void canfestival_stopped(CO_Data *d);
extern void canfestival_post_sync(CO_Data *d);
extern void canfestival_post_TPDO(CO_Data *d);
extern void canfestival_storeODSubIndex(CO_Data *d, UNS16 wIndex, UNS8 bSubindex);
extern void canfestival_post_emcy(CO_Data *d, UNS8 nodeID, UNS16 errCode, UNS8 errReg, const UNS8 errSpec[5]);
#endif
