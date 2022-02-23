/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <os_assert.h>
#include <string.h>

#include <oneos_config.h>
#include <shell.h>
#include "os/os.h"
#include "nimble-console/console.h"

/* BLE */
#include "nimble/ble.h"
#include "host/ble_hs.h"
#include "host/util/util.h"

/* Mandatory services. */
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

/* Application-specified header. */
#include "blescan.h"

#include "nimble/nimble_port.h"

static int blescan_gap_event(struct ble_gap_event *event, void *arg);

/**
 * Initiates the GAP general discovery procedure.
 */
static void
blescan_scan(void)
{
    uint8_t own_addr_type;
    struct ble_gap_disc_params disc_params;
    int rc;

    /* Figure out address to use while advertising (no privacy for now) */
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0)
    {
        MODLOG_DFLT(ERROR, "error determining address type; rc=%d\n", rc);
        return;
    }

    /* Tell the controller to filter duplicates; we don't want to process
     * repeated advertisements from the same device.
     */
    disc_params.filter_duplicates = 1;

    /**
     * Perform a passive scan.  I.e., don't send follow-up scan requests to
     * each advertiser.
     */
    disc_params.passive = 1;

    /* Use defaults for the rest of the parameters. */
    disc_params.itvl = 0;
    disc_params.window = 0;
    disc_params.filter_policy = 0;
    disc_params.limited = 0;

    rc = ble_gap_disc(own_addr_type, BLE_HS_FOREVER, &disc_params,
                      blescan_gap_event, NULL);
    if (rc != 0)
    {
        MODLOG_DFLT(ERROR, "Error initiating GAP discovery procedure; rc=%d\n",
                    rc);
    }
}

/**
 * Initiates the GAP general ext discovery procedure.
 */
static void
blescan_ext_scan(void)
{
    uint8_t own_addr_type;
    int rc;
    struct ble_gap_ext_disc_params uncoded_params;
    struct ble_gap_ext_disc_params coded_params;

    /* Figure out address to use while advertising (no privacy for now) */
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0)
    {
        MODLOG_DFLT(ERROR, "error determining address type; rc=%d\n", rc);
        return;
    }

    uncoded_params.itvl = 200;
    uncoded_params.window = 80;
    uncoded_params.passive = 0;

    coded_params.itvl = 200;
    coded_params.window = 80;
    coded_params.passive = 0;

    rc = ble_gap_ext_disc(own_addr_type, 0, 0, 0, 0, 0, &uncoded_params, &coded_params, blescan_gap_event, NULL);
    if (rc != 0)
    {
        MODLOG_DFLT(ERROR, "Error initiating GAP discovery procedure; rc=%d\n",
                    rc);
    }
}

/**
 * The nimble host executes this callback when a GAP event occurs.  The
 * application associates a GAP event callback with each connection that is
 * established.  blecent uses the same callback for all connections.
 *
 * @param event                 The event being signalled.
 * @param arg                   Application-specified argument; unused by
 *                                  blecent.
 *
 * @return                      0 if the application successfully handled the
 *                                  event; nonzero on failure.  The semantics
 *                                  of the return code is specific to the
 *                                  particular GAP event being signalled.
 */
static int
blescan_gap_event(struct ble_gap_event *event, void *arg)
{
    struct ble_gap_conn_desc desc;
    struct ble_hs_adv_fields fields;
    int rc;

    switch (event->type)
    {
    case BLE_GAP_EVENT_DISC:
        rc = ble_hs_adv_parse_fields(&fields, event->disc.data,
                                     event->disc.length_data);
        if (rc != 0)
        {
            return 0;
        }

        /* An advertisment report was received during GAP discovery. */
        print_adv_fields(&fields);
        return 0;

    case BLE_GAP_EVENT_DISC_COMPLETE:
        MODLOG_DFLT(INFO, "discovery complete; reason=%d\n",
                    event->disc_complete.reason);
        return 0;

    case BLE_GAP_EVENT_EXT_DISC:
        if (event->ext_disc.data_status == BLE_GAP_EXT_ADV_DATA_STATUS_COMPLETE)
        {
            rc = ble_hs_adv_parse_fields(&fields, event->ext_disc.data, event->ext_disc.length_data);
            if (rc == 0)
            {
                if ((fields.name != NULL) && (0 == strncmp(fields.name, "NimBLE", strlen("NimBLE") - 1)))
                {
                    ble_addr_t *addr = &event->ext_disc.addr;

                    if (event->ext_disc.props & BLE_HCI_ADV_LEGACY_MASK)
                    {
                        console_printf("legacy adv");
                    }
                    else if (event->ext_disc.props & BLE_HCI_ADV_SCAN_MASK)
                    {
                        console_printf("ext adv");
                    }
                    else if (event->ext_disc.props & BLE_HCI_ADV_SCAN_RSP_MASK)
                    {
                        console_printf("ext scann adv");
                    }

                    console_printf("p_phy %d\ts_phy %d\tlen %d\taddr %02x:%02x:%02x:%02x:%02x:%02x",
                                   event->ext_disc.prim_phy, event->ext_disc.sec_phy, event->ext_disc.length_data,
                                   addr->val[0], addr->val[1], addr->val[2], addr->val[3], addr->val[4], addr->val[5]);

                    print_adv_fields(&fields);
                }
            }
        }
        return 0;

    default:
        return 0;
    }
}

static void
blescan_on_reset(int reason)
{
    MODLOG_DFLT(ERROR, "Resetting state; reason=%d\n", reason);
}

static void
blescan_on_sync(void)
{
    int rc;

    /* Make sure we have proper identity address set (public preferred) */
    rc = ble_hs_util_ensure_addr(0);
    OS_ASSERT(rc == 0);

    /* Begin scanning for a peripheral to connect to. */
    blescan_ext_scan();
}

/**
 * main
 *
 * All application logic and NimBLE host work is performed in default task.
 *
 * @return int NOTE: this function should never return!
 */
int main(void)
{
    int rc;

    /* Initialize OS */
    nimble_port_oneos_init();

    /* Configure the host. */
    ble_hs_cfg.reset_cb = blescan_on_reset;
    ble_hs_cfg.sync_cb = blescan_on_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    /* As the last thing, process events from default event queue */
    ble_hs_task_startup();

    while (1)
    {
        os_task_msleep(100);
    }

    return 0;
}
