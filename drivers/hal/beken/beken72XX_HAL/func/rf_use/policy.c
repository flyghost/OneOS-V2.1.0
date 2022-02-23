#include "include.h"
#include "arch.h"
#include "policy.h"

ble_scenarios_t ble_scenarios = SCEN_BLE_NORMAL;
wifi_scenarios_t wifi_scenarios = SCEN_WIFI_NORMAL;

static uint8_t wifi_normal_ble_normal[RF_EVENT_MAX]
{
    [RF_EVENT_WIFI_DATA_SEND] = 2,
};

static uint8_t wifi_normal_ble_connecting[RF_EVENT_MAX]
{
    [RF_EVENT_WIFI_DATA_SEND] = 2,
};

static uint8_t wifi_connecting_ble_connecting[RF_EVENT_MAX]
{
    [RF_EVENT_WIFI_DATA_SEND] = 2,
};

static uint8_t wifi_connecting_ble_connecting[RF_EVENT_MAX]
{
    [RF_EVENT_WIFI_DATA_SEND] = 2,
};

uint8_t * rf_scenarios_event[SCEN_WIFI_MAX][SCEN_BLE_MAX] = 
{
    {wifi_normal_ble_normal, wifi_normal_ble_connecting, },
    {wifi_connecting_ble_connecting, wifi_connecting_ble_connecting, },
};

OSStatus get_event_priority(uint8_t event, uint8_t *priority)
{
    OSStatus ret = kNoErr;
    uint8_t *priority_table;
    
    if(event >= RF_EVENT_MAX)
    {
        os_printf("unknow event!!!\r\n");
        ret = kGeneralErr;
    }

    priority_table = rf_scenarios_event[wifi_scenarios][ble_scenarios];
    
    *priority = priority_table[event];

    return ret;
}

OSStatus set_event_priority(uint8_t event, uint8_t priority)
{
    OSStatus ret = kNoErr;
    uint8_t *priority_table;
    
    GLOBAL_INT_DECLARATION();
    GLOBAL_INT_DISABLE();
    
    if(event >= RF_EVENT_MAX)
    {
        os_printf("unknow event!!!\r\n");
        ret = kGeneralErr;
    }

    priority_table = rf_scenarios_event[wifi_scenarios][ble_scenarios];
    priority_table[event] = priority;

    GLOBAL_INT_RESTORE();

    return ret;
}

void set_ble_scenarios(ble_scenarios_t scenarios)
{
    GLOBAL_INT_DECLARATION();
    GLOBAL_INT_DISABLE();
    ble_scenarios = scenarios;
    GLOBAL_INT_RESTORE();
}

void set_wifi_scenarios(wifi_scenarios_t scenarios)
{
    GLOBAL_INT_DECLARATION();
    GLOBAL_INT_DISABLE();
    wifi_scenarios = scenarios;
    GLOBAL_INT_RESTORE();
}

ble_scenarios_t get_ble_scanarios(void)
{
    return ble_scenarios;
}

wifi_scenarios_t get_wifi_scanarios(void)
{
    return wifi_scenarios;
}

