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
#include <stdio.h>
#include <errno.h>
#include <oneos_config.h>
#include <shell.h>

#include "os/os.h"
#include "nimble-console/console.h"
#include "nimble/ble.h"
#include "host/ble_hs.h"
#include "host/util/util.h"

#include "patterns.h"

static uint8_t id_addr_type;

static void start_legacy_duration(uint8_t pattern, bool configure);
static void start_ext_max_events(uint8_t pattern, bool configure);

static int
start_ext_max_events_gap_event(struct ble_gap_event *event, void *arg)
{
    static uint8_t pattern = 1;

    switch (event->type)
    {
    case BLE_GAP_EVENT_ADV_COMPLETE:
        break;
    default:
        OS_ASSERT(0);
        return 0;
    }

    OS_ASSERT(event->adv_complete.instance == 4);
    OS_ASSERT(event->adv_complete.reason == BLE_HS_ETIMEOUT);
    OS_ASSERT(event->adv_complete.num_ext_adv_events == 10);

    console_printf("instance %u terminated\n", event->adv_complete.instance);

    pattern++;

    start_ext_max_events(pattern, false);

    return 0;
}

/* Starts advertising instance with 100 max events and changing adv data pattern
 * and SID.
 */
static void
start_ext_max_events(uint8_t pattern, bool configure)
{
    struct ble_gap_ext_adv_params params;
    static uint8_t adv_data[600];
    struct os_mbuf *data;
    uint8_t instance = 4;
    ble_addr_t addr;
    struct ble_hs_adv_fields fields;
    uint8_t fields_sz;
    const char *device_name = "NimBLE-ExtMax";
    uint8_t buf[BLE_HS_ADV_MAX_SZ];
    int events = 10;
    int rc;

    if (configure)
    {
        /* use defaults for non-set params */
        memset(&params, 0, sizeof(params));

        /* advertise using random addr */
        params.own_addr_type = BLE_OWN_ADDR_RANDOM;

        params.primary_phy = BLE_HCI_LE_PHY_1M;
        params.secondary_phy = BLE_HCI_LE_PHY_1M;
        params.tx_power = 127;
        params.sid = pattern % 16;

        /* allow larger interval, 400 * 0.625ms with 100 events will give up to
         * ~2.5 seconds for instance
         */
        params.itvl_min = BLE_GAP_ADV_FAST_INTERVAL1_MIN;
        params.itvl_max = 400;

        /* configure instance 0 */
        rc = ble_gap_ext_adv_configure(instance, &params, NULL,
                                       start_ext_max_events_gap_event, NULL);
        OS_ASSERT(rc == 0);

        /* set random (NRPA) address for instance */
        rc = ble_hs_id_gen_rnd(1, &addr);
        OS_ASSERT(rc == 0);

        console_printf("ins %d\taddr %02x:%02x:%02x:%02x:%02x:%02x", instance, addr.val[0], addr.val[1],
                       addr.val[2], addr.val[3], addr.val[4], addr.val[5]);

        rc = ble_gap_ext_adv_set_addr(instance, &addr);
        OS_ASSERT(rc == 0);
    }

    /* in this case both advertising data and scan response is allowed, but
     * both are limited to 31 bytes each
     */

    /* get mbuf for adv data */
    data = os_msys_get_pkthdr(BLE_HS_ADV_MAX_SZ, 0);
    OS_ASSERT(data);

    memset(&fields, 0, sizeof(fields));

    /* Fill the fields with advertising data - flags, tx power level, name */
    fields.flags = BLE_HS_ADV_F_DISC_GEN;
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = 0x10;
    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;

    rc = ble_hs_adv_set_fields(&fields, buf, &fields_sz, BLE_HS_ADV_MAX_SZ);
    OS_ASSERT(rc == 0);

    rc = os_mbuf_append(data, buf, fields_sz);
    OS_ASSERT(rc == 0);

    rc = ble_gap_ext_adv_set_data(instance, data);
    OS_ASSERT(rc == 0);

    /* start advertising */
    rc = ble_gap_ext_adv_start(instance, 0, events);
    OS_ASSERT(rc == 0);

    console_printf("instance %u started (PDUs with max events %d)\n",
                   instance, events);
}

static int
start_legacy_duration_gap_event(struct ble_gap_event *event, void *arg)
{
    static uint8_t pattern = 1;

    switch (event->type)
    {
    case BLE_GAP_EVENT_ADV_COMPLETE:
        break;
    default:
        OS_ASSERT(0);
        return 0;
    }

    OS_ASSERT(event->adv_complete.instance == 3);
    OS_ASSERT(event->adv_complete.reason == BLE_HS_ETIMEOUT);

    console_printf("instance %u terminated\n", event->adv_complete.instance);

    pattern++;

    start_legacy_duration(pattern, false);

    return 0;
}

/* Starts advertising instance with 5sec timeout and changing adv data pattern
 * and SID.
 */
static void
start_legacy_duration(uint8_t pattern, bool configure)
{
    struct ble_gap_ext_adv_params params;
    uint8_t adv_data[31];
    struct os_mbuf *data;
    uint8_t instance = 3;
    ble_addr_t addr;
    int duration = 500; /* 5seconds, 10ms units */
    struct ble_hs_adv_fields fields;
    uint8_t fields_sz;
    const char *device_name = "NimBLE-Legacy";
    uint8_t buf[BLE_HS_ADV_MAX_SZ];
    int rc;

    if (configure)
    {
        /* use defaults for non-set params */
        memset(&params, 0, sizeof(params));

        /* enable advertising using legacy PDUs */
        params.legacy_pdu = 1;

        /* advertise using random addr */
        params.own_addr_type = BLE_OWN_ADDR_RANDOM;

        params.primary_phy = BLE_HCI_LE_PHY_1M;
        params.secondary_phy = BLE_HCI_LE_PHY_1M;
        params.tx_power = 127;
        params.sid = pattern % 16;

        /* configure instance 0 */
        rc = ble_gap_ext_adv_configure(instance, &params, NULL,
                                       start_legacy_duration_gap_event, NULL);
        OS_ASSERT(rc == 0);

        /* set random (NRPA) address for instance */
        rc = ble_hs_id_gen_rnd(1, &addr);
        OS_ASSERT(rc == 0);

        console_printf("ins %d\taddr %02x:%02x:%02x:%02x:%02x:%02x", instance, addr.val[0], addr.val[1],
            addr.val[2], addr.val[3], addr.val[4], addr.val[5]);

        rc = ble_gap_ext_adv_set_addr(instance, &addr);
        OS_ASSERT(rc == 0);
    }

    /* in this case both advertising data and scan response is allowed, but
     * both are limited to 31 bytes each
     */

    /* get mbuf for adv data */
    data = os_msys_get_pkthdr(31, 0);
    OS_ASSERT(data);

    memset(&fields, 0, sizeof(fields));

    /* Fill the fields with advertising data - flags, tx power level, name */
    fields.flags = BLE_HS_ADV_F_DISC_GEN;
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = 0x10;
    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;

    rc = ble_hs_adv_set_fields(&fields, buf, &fields_sz, BLE_HS_ADV_MAX_SZ);
    OS_ASSERT(rc == 0);
    
    rc = os_mbuf_append(data, buf, fields_sz);
    OS_ASSERT(rc == 0);

    memset(adv_data, pattern, sizeof(adv_data) - fields_sz);

    rc = ble_gap_ext_adv_set_data(instance, data);
    OS_ASSERT(rc == 0);

    /* start advertising */
    rc = ble_gap_ext_adv_start(instance, duration, 0);
    OS_ASSERT(rc == 0);

    console_printf("instance %u started (legacy PDUs with duration %d)\n",
                   instance, duration);
}

/* this is simple non-connectable scannable instance using legacy PUDs that
 * runs forever
 */
static void
start_scannable_legacy_ext(void)
{
    struct ble_gap_ext_adv_params params;
    struct os_mbuf *data;
    uint8_t instance = 2;
    ble_addr_t addr;
    struct ble_hs_adv_fields fields;
    uint8_t fields_sz;
    const char *device_name = "NimBLE-ScannLegacy";
    const char *device_resp_name="NimBLE-ScannLegacyResp";
    uint8_t buf[BLE_HS_ADV_MAX_SZ];
    int rc;

    /* use defaults for non-set params */
    memset(&params, 0, sizeof(params));

    /* enable scannable advertising using legacy PDUs */
    params.scannable = 1;
    params.legacy_pdu = 1;

    /* advertise using random addr */
    params.own_addr_type = BLE_OWN_ADDR_RANDOM;

    params.primary_phy = BLE_HCI_LE_PHY_1M;
    params.secondary_phy = BLE_HCI_LE_PHY_1M;
    params.tx_power = 127;
    params.sid = 2;

    /* configure instance 0 */
    rc = ble_gap_ext_adv_configure(instance, &params, NULL, NULL, NULL);
    OS_ASSERT(rc == 0);

    /* set random (NRPA) address for instance */
    rc = ble_hs_id_gen_rnd(1, &addr);
    OS_ASSERT(rc == 0);

    console_printf("ins %d\taddr %02x:%02x:%02x:%02x:%02x:%02x", instance, addr.val[0], addr.val[1],
        addr.val[2], addr.val[3], addr.val[4], addr.val[5]);

    rc = ble_gap_ext_adv_set_addr(instance, &addr);
    OS_ASSERT(rc == 0);

    /* in this case both advertising data and scan response is allowed, but
     * both are limited to 31 bytes each
     */

    /* get mbuf for adv data */
    data = os_msys_get_pkthdr(BLE_HS_ADV_MAX_SZ, 0);
    OS_ASSERT(data);

    memset(&fields, 0, sizeof(fields));

    /* Fill the fields with advertising data - flags, tx power level, name */
    fields.flags = BLE_HS_ADV_F_DISC_GEN;
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = 0x10;
    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;

    rc = ble_hs_adv_set_fields(&fields, buf, &fields_sz, BLE_HS_ADV_MAX_SZ);
    OS_ASSERT(rc == 0);

    rc = os_mbuf_append(data, buf, fields_sz);
    OS_ASSERT(rc == 0);

    rc = ble_gap_ext_adv_set_data(instance, data);
    OS_ASSERT(rc == 0);

    /* get mbuf for adv data */
    data = os_msys_get_pkthdr(BLE_HS_ADV_MAX_SZ, 0);
    OS_ASSERT(data);

    memset(&fields, 0, sizeof(fields));

    /* Fill the fields with advertising data - flags, tx power level, name */
    fields.name = (uint8_t *)device_resp_name;
    fields.name_len = strlen(device_resp_name);
    fields.name_is_complete = 1;

    rc = ble_hs_adv_set_fields(&fields, buf, &fields_sz, BLE_HS_ADV_MAX_SZ);
    OS_ASSERT(rc == 0);

    rc = os_mbuf_append(data, buf, fields_sz);
    OS_ASSERT(rc == 0);

    rc = ble_gap_ext_adv_rsp_set_data(instance, data);
    OS_ASSERT(rc == 0);

    /* start advertising */
    rc = ble_gap_ext_adv_start(instance, 0, 0);
    OS_ASSERT(rc == 0);

    console_printf("instance %u started (scannable legacy PDUs)\n", instance);
}

static int
scannable_ext_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
    default:
        break;
    }

    return 0;
}

/* this is simple scannable instance that runs forever
 * TODO Get scan request notifications.
 */
static void
start_scannable_ext(void)
{
    struct ble_gap_ext_adv_params params;
    struct os_mbuf *data;
    uint8_t instance = 1;
    ble_addr_t addr;
    struct ble_hs_adv_fields fields;
    uint8_t fields_sz;
    const char *device_name = "NimBLE-Scann";
    uint8_t buf[BLE_HS_ADV_MAX_SZ];
    int rc;

    /* use defaults for non-set params */
    memset(&params, 0, sizeof(params));

    /* enable scannable advertising */
    params.scannable = 1;

    /* enable scan request notification */
    params.scan_req_notif = 1;

    /* advertise using random addr */
    params.own_addr_type = BLE_OWN_ADDR_RANDOM;

    params.primary_phy = BLE_HCI_LE_PHY_CODED;
    params.secondary_phy = BLE_HCI_LE_PHY_2M;
    params.tx_power = 127;
    params.sid = 1;

    /* configure instance 0 */
    rc = ble_gap_ext_adv_configure(instance, &params, NULL,
                                   scannable_ext_gap_event, NULL);
    OS_ASSERT(rc == 0);

    /* set random (NRPA) address for instance */
    rc = ble_hs_id_gen_rnd(1, &addr);
    OS_ASSERT(rc == 0);

    console_printf("ins %d\taddr %02x:%02x:%02x:%02x:%02x:%02x", instance, addr.val[0], addr.val[1],
                   addr.val[2], addr.val[3], addr.val[4], addr.val[5]);

    rc = ble_gap_ext_adv_set_addr(instance, &addr);
    OS_ASSERT(rc == 0);

    /* in this case only scan response is allowed */

    /* get mbuf for scan rsp data */
    data = os_msys_get_pkthdr(BLE_HS_ADV_MAX_SZ, 0);
    OS_ASSERT(data);

    memset(&fields, 0, sizeof(fields));

    /* Fill the fields with advertising data - flags, tx power level, name */
    fields.flags = BLE_HS_ADV_F_DISC_GEN;
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = 0x10;
    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;

    rc = ble_hs_adv_set_fields(&fields, buf, &fields_sz, BLE_HS_ADV_MAX_SZ);
    OS_ASSERT(rc == 0);

    rc = os_mbuf_append(data, buf, fields_sz);
    OS_ASSERT(rc == 0);

    rc = ble_gap_ext_adv_rsp_set_data(instance, data);
    OS_ASSERT(rc == 0);

    /* start advertising */
    rc = ble_gap_ext_adv_start(instance, 0, 0);
    OS_ASSERT(rc == 0);

    console_printf("instance %u started (scannable)\n", instance);
}

/* this is simple non-connectable instance that runs forever */
static void
start_non_connectable_ext(void)
{
    struct ble_gap_ext_adv_params params;
    struct os_mbuf *data;
    uint8_t instance = 0;
    struct ble_hs_adv_fields fields;
    uint8_t fields_sz;
    const char *device_name = "NimBLE-NonConn";
    uint8_t buf[BLE_HS_ADV_MAX_SZ];
    int rc;

    /* use defaults for non-set params */
    memset(&params, 0, sizeof(params));

    /* advertise using ID addr */
    params.own_addr_type = id_addr_type;

    params.primary_phy = BLE_HCI_LE_PHY_1M;
    params.secondary_phy = BLE_HCI_LE_PHY_2M;
    params.tx_power = 127;
    params.sid = 0;

    /* configure instance */
    rc = ble_gap_ext_adv_configure(instance, &params, NULL, NULL, NULL);
    OS_ASSERT(rc == 0);

    /* in this case only advertisign data is allowed */

    /* get mbuf for adv data */
    data = os_msys_get_pkthdr(BLE_HS_ADV_MAX_SZ, 0);
    OS_ASSERT(data);

    memset(&fields, 0, sizeof(fields));

    /* Fill the fields with advertising data - flags, tx power level, name */
    fields.flags = BLE_HS_ADV_F_DISC_GEN;
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = 0x10;
    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;

    rc = ble_hs_adv_set_fields(&fields, buf, &fields_sz, BLE_HS_ADV_MAX_SZ);
    OS_ASSERT(rc == 0);

    rc = os_mbuf_append(data, buf, fields_sz);
    OS_ASSERT(rc == 0);

    rc = ble_gap_ext_adv_set_data(instance, data);
    OS_ASSERT(rc == 0);

    console_printf("ins %d\taddr %02x:%02x:%02x:%02x:%02x:%02x", instance, 0xcc, 0xbb, 0xaa, 0x33, 0x22, 0x11);

    /* start advertising */
    rc = ble_gap_ext_adv_start(instance, 0, 0);
    OS_ASSERT(rc == 0);

    console_printf("instance %u started (non-con non-scan)\n", instance);
}

static void
on_sync(void)
{
    int rc;

    console_printf("Synced, starting advertising\n");

    /* Make sure we have proper identity address set (public preferred) */
    rc = ble_hs_util_ensure_addr(0);
    OS_ASSERT(rc == 0);

    /* configure global address */
    rc = ble_hs_id_infer_auto(0, &id_addr_type);
    OS_ASSERT(rc == 0);

    start_non_connectable_ext();

    start_scannable_ext();

    start_scannable_legacy_ext();

    start_legacy_duration(0, true);

    start_ext_max_events(0, true);
}

/*
 * main
 *
 * The main task for the project. This function initializes the packages,
 * then starts serving events from default event queue.
 *
 * @return int NOTE: this function should never return!
 */
int main(void)
{
    int rc;

    /* Initialize OS */
    nimble_port_oneos_init();

    console_printf("Extended Advertising sample application\n");

    /* Set sync callback */
    ble_hs_cfg.sync_cb = on_sync;

    /* As the last thing, process events from default event queue */
    ble_hs_task_startup();

    while (1)
    {
        os_task_msleep(100);
    }

    return 0;
}
