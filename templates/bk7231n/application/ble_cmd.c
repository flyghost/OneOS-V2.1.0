//#include <rtthread.h>
#include "shell.h"
#include "common.h"
#include "param_config.h"

#if defined(CFG_SUPPORT_BLE) && defined(BEKEN_ATE)

#include "ble_api.h"
#if (CFG_BLE_VERSION == BLE_VERSION_5_x)
#include "app_ble.h"
#endif
#include "ble_pub.h"

#if (CFG_BLE_VERSION == BLE_VERSION_4_2)
#define BUILD_UINT16(loByte, hiByte) \
          ((uint16_t)(((loByte) & 0x00FF) + (((hiByte) & 0x00FF) << 8)))

#define BK_ATT_DECL_PRIMARY_SERVICE         0x2800
#define BK_ATT_DECL_CHARACTERISTIC          0x2803
#define BK_ATT_DESC_CLIENT_CHAR_CFG         0x2902
#define TEST_SERVICE_UUID                   0xFFFF
#define WRITE_REQ_CHARACTERISTIC            0xFF01
#define INDICATE_CHARACTERISTIC             0xFF02

static const uint8_t test_svc_uuid[16] = {0xFF,0xFF,0,0,0x34,0x56,0,0,0,0,0x28,0x37,0,0,0,0};

enum
{
	TEST_IDX_SVC,	 
	TEST_IDX_FF01_VAL_CHAR,
	TEST_IDX_FF01_VAL_VALUE,
	TEST_IDX_FF02_VAL_CHAR,
	TEST_IDX_FF02_VAL_VALUE,
	TEST_IDX_FF02_VAL_IND_CFG,
	TEST_IDX_NB,
};

bk_attm_desc_t test_att_db[6] =
{
	//  Service Declaration
	[TEST_IDX_SVC]              = {BK_ATT_DECL_PRIMARY_SERVICE, BK_PERM_SET(RD, ENABLE), 0, 0},
	
	//  Level Characteristic Declaration
	[TEST_IDX_FF01_VAL_CHAR]    = {BK_ATT_DECL_CHARACTERISTIC,  BK_PERM_SET(RD, ENABLE), 0, 0},
	//  Level Characteristic Value
	[TEST_IDX_FF01_VAL_VALUE]   = {WRITE_REQ_CHARACTERISTIC,    BK_PERM_SET(WRITE_REQ, ENABLE), BK_PERM_SET(RI, ENABLE) , 128},
	
	[TEST_IDX_FF02_VAL_CHAR]    = {BK_ATT_DECL_CHARACTERISTIC,  BK_PERM_SET(RD, ENABLE), 0, 0},
	//  Level Characteristic Value
	[TEST_IDX_FF02_VAL_VALUE]   = {INDICATE_CHARACTERISTIC,     BK_PERM_SET(IND, ENABLE) , BK_PERM_SET(RI, ENABLE) , 128},

	//  Level Characteristic - Client Characteristic Configuration Descriptor

	[TEST_IDX_FF02_VAL_IND_CFG] = {BK_ATT_DESC_CLIENT_CHAR_CFG, BK_PERM_SET(RD, ENABLE)|BK_PERM_SET(WRITE_REQ, ENABLE), 0, 0},
};

ble_err_t bk_ble_init(void)
{
    ble_err_t status;
    struct bk_ble_db_cfg ble_db_cfg;

    ble_db_cfg.att_db = test_att_db;
    ble_db_cfg.att_db_nb = TEST_IDX_NB;
    ble_db_cfg.prf_task_id = 0;
    ble_db_cfg.start_hdl = 0;
    ble_db_cfg.svc_perm = 0;
    memcpy(&(ble_db_cfg.uuid[0]), &test_svc_uuid[0], 16);

    status = bk_ble_create_db(&ble_db_cfg);

    return status;
}

void appm_adv_data_decode(uint8_t len, const uint8_t *data)
{
    uint8_t index;
	uint8_t i;
    for(index = 0; index < len;)
    {
        switch(data[index + 1])
        {
            case 0x01:
            {
                bk_printf("AD_TYPE : ");
                for(i = 0; i < data[index] - 1; i++)
                {
                    bk_printf("%02x ",data[index + 2 + i]);
                }
                bk_printf("\r\n");
                index +=(data[index] + 1);
            }
            break;
            case 0x08:
            case 0x09:
            {
                bk_printf("ADV_NAME : ");
                for(i = 0; i < data[index] - 1; i++)
                {
                    bk_printf("%c",data[index + 2 + i]);
                }
                bk_printf("\r\n");
                index +=(data[index] + 1);
            }
            break;
            case 0x02:
            {
                bk_printf("UUID : ");
                for(i = 0; i < data[index] - 1;)
                {
                    bk_printf("%02x%02x  ",data[index + 2 + i],data[index + 3 + i]);
                    i+=2;
                }
                bk_printf("\r\n");
                index +=(data[index] + 1);
            }
            break;
            default:
            {
                index +=(data[index] + 1);
            }
            break;
        }
    }
    return ;
}

void ble_write_callback(write_req_t *write_req)
{
    bk_printf("write_cb[prf_id:%d, att_idx:%d, len:%d]\r\n", write_req->prf_id, write_req->att_idx, write_req->len);
}

uint8_t ble_read_callback(read_req_t *read_req)
{
    bk_printf("read_cb[prf_id:%d, att_idx:%d]\r\n", read_req->prf_id, read_req->att_idx);
    read_req->value[0] = 0x10;
    read_req->value[1] = 0x20;
    read_req->value[2] = 0x30;
    return 3;
}

void ble_event_callback(ble_event_t event, void *param)
{
    switch(event)
    {
        case BLE_STACK_OK:
        {
            bk_printf("STACK INIT OK\r\n");
            bk_ble_init();
        }
        break;
        case BLE_STACK_FAIL:
        {
            bk_printf("STACK INIT FAIL\r\n");
        }
        break;
        case BLE_CONNECT:
        {
            bk_printf("BLE CONNECT\r\n");
        }
        break;
        case BLE_DISCONNECT:
        {
            bk_printf("BLE DISCONNECT\r\n");
        }
        break;
        case BLE_MTU_CHANGE:
        {
            bk_printf("BLE_MTU_CHANGE:%d\r\n", *(uint16_t *)param);
        }
        break;
        case BLE_TX_DONE:
        {
            bk_printf("BLE_TX_DONE\r\n");
        }
        break;
        case BLE_GEN_DH_KEY:
        {
            bk_printf("BLE_GEN_DH_KEY\r\n");
        }    
        break;
        case BLE_GET_KEY:
        {
            bk_printf("BLE_GET_KEY\r\n");
        }    
        break;
        case BLE_CREATE_DB_OK:
        {
            bk_printf("CREATE DB SUCCESS\r\n");
        }
        break;
        default:
            bk_printf("UNKNOW EVENT\r\n");
        break;
    }
}

static void ble_command_usage(void)
{
    bk_printf("ble help           - Help information\n");
    bk_printf("ble active         - Active ble to with BK7231BTxxx\n");    
	bk_printf("ble start_adv      - Start advertising as a slave device\n");
	bk_printf("ble stop_adv       - Stop advertising as a slave device\n");
    bk_printf("ble notify prf_id att_id value\n");
    bk_printf("                   - Send ntf value to master\n");
    bk_printf("ble indicate prf_id att_id value\n");
    bk_printf("                   - Send ind value to master\n");

    bk_printf("ble disc           - Disconnect\n");
    bk_printf("ble dut            - dut test\n");
}

static void ble_get_info_handler(void)
{
    UINT8 *ble_mac;
    bk_printf("\r\n****** ble information ************\r\n");

    if (ble_is_start() == 0) {
        bk_printf("no ble startup          \r\n");
        return;
    }
    ble_mac = ble_get_mac_addr();    
    bk_printf("* name: %s             \r\n", ble_get_name());
    bk_printf("* mac:%02x-%02x-%02x-%02x-%02x-%02x\r\n", ble_mac[0], ble_mac[1],ble_mac[2],ble_mac[3],ble_mac[4],ble_mac[5]);  
    bk_printf("***********  end  *****************\r\n");     
}

typedef adv_info_t ble_adv_param_t;

static void ble_advertise(void)
{
    UINT8 mac[6];
    char ble_name[20];
    UINT8 adv_idx, adv_name_len;

    wifi_get_mac_address((char *)mac, CONFIG_ROLE_STA);
    adv_name_len = snprintf(ble_name, sizeof(ble_name), "bk72xx-%02x%02x", mac[4], mac[5]);

    memset(&adv_info, 0x00, sizeof(adv_info));

    adv_info.channel_map = 7;
    adv_info.interval_min = 160;
    adv_info.interval_max = 160;

    adv_idx = 0;
    adv_info.advData[adv_idx] = 0x02; adv_idx++;
    adv_info.advData[adv_idx] = 0x01; adv_idx++;
    adv_info.advData[adv_idx] = 0x06; adv_idx++;

    adv_info.advData[adv_idx] = adv_name_len + 1; adv_idx +=1;
    adv_info.advData[adv_idx] = 0x09; adv_idx +=1; //name
    memcpy(&adv_info.advData[adv_idx], ble_name, adv_name_len); adv_idx +=adv_name_len;

    adv_info.advDataLen = adv_idx;

    adv_idx = 0;

    adv_info.respData[adv_idx] = adv_name_len + 1; adv_idx +=1;
    adv_info.respData[adv_idx] = 0x08; adv_idx +=1; //name
    memcpy(&adv_info.respData[adv_idx], ble_name, adv_name_len); adv_idx +=adv_name_len;
    adv_info.respDataLen = adv_idx;

    if (ERR_SUCCESS != appm_start_advertising())
    {
        bk_printf("ERROR\r\n");
    }
}

static void ble(int argc, char **argv)
{
    ble_adv_param_t adv_param;

    if ((argc < 2) || (os_strcmp(argv[1], "help") == 0))
    {
        ble_command_usage();
        return ;
    }

    if (os_strcmp(argv[1], "active") == 0)
    {
        ble_set_write_cb(ble_write_callback);
        ble_set_read_cb(ble_read_callback);
        ble_set_event_cb(ble_event_callback);
        ble_activate(NULL);
    }
	else if(os_strcmp(argv[1], "start_adv") == 0)
    { 
        ble_advertise();
    }
    else if(os_strcmp(argv[1], "stop_adv") == 0)
    {
        if(ERR_SUCCESS != appm_stop_advertising())
        {
            bk_printf("ERROR\r\n");
        }
    }
    else if(os_strcmp(argv[1], "notify") == 0)
    {
        uint8 len;
        uint16 prf_id;
        uint16 att_id;
        uint8 write_buffer[20];
        
        if(argc != 5)
        {
            ble_command_usage();
            return ;
        }

        len = os_strlen(argv[4]);
        if(len % 2 != 0)
        {
            bk_printf("ERROR\r\n");
            return ;
        }
        hexstr2bin(argv[4], write_buffer, len/2);

        prf_id = atoi(argv[2]);
        att_id = atoi(argv[3]);

        if(ERR_SUCCESS != bk_ble_send_ntf_value(len, write_buffer, prf_id, att_id))
        {
            bk_printf("ERROR\r\n");
        }
    }
    else if(os_strcmp(argv[1], "indicate") == 0)
    {
        uint8 len;
        uint16 prf_id;
        uint16 att_id;
        uint8 write_buffer[20];
        
        if(argc != 5)
        {
            ble_command_usage();
            return ;
        }

        len = os_strlen(argv[4]);
        if(len % 2 != 0)
        {
            bk_printf("ERROR\r\n");
            return ;
        }
        hexstr2bin(argv[4], write_buffer, len/2);

        prf_id = atoi(argv[2]);
        att_id = atoi(argv[3]);

        if(ERR_SUCCESS != bk_ble_send_ind_value(len / 2, write_buffer, prf_id, att_id))
        {
            bk_printf("ERROR\r\n");
        }
    }
    else if(os_strcmp(argv[1], "disc") == 0)
    {
        appm_disconnect();
    }
    else if(os_strcmp(argv[1], "dut") == 0)
    {
        ble_dut_start();
    }
}

MSH_CMD_EXPORT(ble, ble command);
#endif

#if (CFG_BLE_VERSION == BLE_VERSION_5_x)
extern struct app_env_tag app_ble_ctx;

#define BUILD_UINT16(loByte, hiByte) \
          ((uint16_t)(((loByte) & 0x00FF) + (((hiByte) & 0x00FF) << 8)))

#define BK_ATT_DECL_PRIMARY_SERVICE_128     {0x00,0x28,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
#define BK_ATT_DECL_CHARACTERISTIC_128      {0x03,0x28,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
#define BK_ATT_DESC_CLIENT_CHAR_CFG_128     {0x02,0x29,0,0,0,0,0,0,0,0,0,0,0,0,0,0}

#define WRITE_REQ_CHARACTERISTIC_128        {0x01,0xFF,0,0,0x34,0x56,0,0,0,0,0x28,0x37,0,0,0,0}
#define INDICATE_CHARACTERISTIC_128         {0x02,0xFF,0,0,0x34,0x56,0,0,0,0,0x28,0x37,0,0,0,0}
#define NOTIFY_CHARACTERISTIC_128           {0x03,0xFF,0,0,0x34,0x56,0,0,0,0,0x28,0x37,0,0,0,0}

static const uint8_t test_svc_uuid[16] = {0xFF,0xFF,0,0,0x34,0x56,0,0,0,0,0x28,0x37,0,0,0,0};

enum
{
	TEST_IDX_SVC,
	TEST_IDX_FF01_VAL_CHAR,
	TEST_IDX_FF01_VAL_VALUE,
	TEST_IDX_FF02_VAL_CHAR,
	TEST_IDX_FF02_VAL_VALUE,
	TEST_IDX_FF02_VAL_IND_CFG,
	TEST_IDX_FF03_VAL_CHAR,
	TEST_IDX_FF03_VAL_VALUE,
	TEST_IDX_FF03_VAL_NTF_CFG,
	TEST_IDX_NB,
};

bk_attm_desc_t test_att_db[TEST_IDX_NB] =
{
	//  Service Declaration
	[TEST_IDX_SVC]              = {BK_ATT_DECL_PRIMARY_SERVICE_128, BK_PERM_SET(RD, ENABLE), 0, 0},

	//  Level Characteristic Declaration
	[TEST_IDX_FF01_VAL_CHAR]    = {BK_ATT_DECL_CHARACTERISTIC_128,  BK_PERM_SET(RD, ENABLE), 0, 0},
	//  Level Characteristic Value
	[TEST_IDX_FF01_VAL_VALUE]   = {WRITE_REQ_CHARACTERISTIC_128,    BK_PERM_SET(WRITE_REQ, ENABLE), BK_PERM_SET(RI, ENABLE)|BK_PERM_SET(UUID_LEN, UUID_16), 128},

	[TEST_IDX_FF02_VAL_CHAR]    = {BK_ATT_DECL_CHARACTERISTIC_128,  BK_PERM_SET(RD, ENABLE), 0, 0},
	//  Level Characteristic Value
	[TEST_IDX_FF02_VAL_VALUE]   = {INDICATE_CHARACTERISTIC_128,     BK_PERM_SET(IND, ENABLE), BK_PERM_SET(RI, ENABLE)|BK_PERM_SET(UUID_LEN, UUID_16), 128},

	//  Level Characteristic - Client Characteristic Configuration Descriptor

	[TEST_IDX_FF02_VAL_IND_CFG] = {BK_ATT_DESC_CLIENT_CHAR_CFG_128, BK_PERM_SET(RD, ENABLE)|BK_PERM_SET(WRITE_REQ, ENABLE), 0, 0},

    [TEST_IDX_FF03_VAL_CHAR]    = {BK_ATT_DECL_CHARACTERISTIC_128,  BK_PERM_SET(RD, ENABLE), 0, 0},
	//  Level Characteristic Value
	[TEST_IDX_FF03_VAL_VALUE]   = {NOTIFY_CHARACTERISTIC_128,       BK_PERM_SET(NTF, ENABLE), BK_PERM_SET(RI, ENABLE)|BK_PERM_SET(UUID_LEN, UUID_16), 128},

	//  Level Characteristic - Client Characteristic Configuration Descriptor

	[TEST_IDX_FF03_VAL_NTF_CFG] = {BK_ATT_DESC_CLIENT_CHAR_CFG_128, BK_PERM_SET(RD, ENABLE)|BK_PERM_SET(WRITE_REQ, ENABLE), 0, 0},
};
ble_err_t bk_ble_init(void)
{
    ble_err_t status;
    struct bk_ble_db_cfg ble_db_cfg;

    ble_db_cfg.att_db = test_att_db;
    ble_db_cfg.att_db_nb = TEST_IDX_NB;
    ble_db_cfg.prf_task_id = 0;
    ble_db_cfg.start_hdl = 0;
    ble_db_cfg.svc_perm = 0;
    memcpy(&(ble_db_cfg.uuid[0]), &test_svc_uuid[0], 16);

    status = bk_ble_create_db(&ble_db_cfg);

    return status;
}

void ble_write_callback(write_req_t *write_req)
{
    bk_printf("write_cb[prf_id:%d, att_idx:%d, len:%d]\r\n", write_req->prf_id, write_req->att_idx, write_req->len);
}

uint8_t ble_read_callback(read_req_t *read_req)
{
    bk_printf("read_cb[prf_id:%d, att_idx:%d]\r\n", read_req->prf_id, read_req->att_idx);
    read_req->value[0] = 0x10;
    read_req->value[1] = 0x20;
    read_req->value[2] = 0x30;
    return 3;
}

void ble_event_callback(ble_event_t event, void *param)
{
    switch(event)
    {
        case BLE_STACK_OK:
        {
            bk_printf("STACK INIT OK\r\n");
        }
        break;
        case BLE_STACK_FAIL:
        {
            bk_printf("STACK INIT FAIL\r\n");
        }
        break;
        case BLE_CONNECT:
        {
            bk_printf("BLE CONNECT\r\n");
        }
        break;
        case BLE_DISCONNECT:
        {
            bk_printf("BLE DISCONNECT\r\n");
        }
        break;
        case BLE_MTU_CHANGE:
        {
            bk_printf("BLE_MTU_CHANGE:%d\r\n", *(uint16_t *)param);
        }
        break;
        case BLE_TX_DONE:
        {
            bk_printf("BLE_TX_DONE\r\n");
        }
        break;
        case BLE_GEN_DH_KEY:
        {
            bk_printf("BLE_GEN_DH_KEY\r\n");
        }    
        break;
        case BLE_GET_KEY:
        {
            bk_printf("BLE_GET_KEY\r\n"); 
        }    
        break;
        case BLE_CREATE_DB_OK:
        {
            bk_printf("CREATE DB SUCCESS\r\n");
        }
        break;
        default:
            bk_printf("UNKNOW EVENT\r\n");
        break;
    }
}

static void ble(int argc, char **argv)
{
	uint8_t adv_data[31];
	uint32_t data_len;
//	if (os_strcmp(argv[1], "active") == 0)
//    {
//        ble_set_write_cb(ble_write_callback);
//        ble_set_read_cb(ble_read_callback);
//        ble_set_event_cb(ble_event_callback);
//        bk_ble_init();
//    }
//	else if (os_strcmp(argv[1], "adv_create") == 0)
//    {
//        ble_appm_create_advertising(0x7, 160, 160);
//    }
//	else if (os_strcmp(argv[1], "set_adv_data") == 0)
//    {
//    	adv_data[0] = 0x02;
//		adv_data[1] = 0x01;
//		adv_data[2] = 0x06;

//		adv_data[3] = 0x0B;
//		adv_data[4] = 0x09;
//		memcpy(&adv_data[5], "7231N_BLE", 10);
//        ble_appm_set_adv_data(app_ble_ctx.adv_actv_idx, adv_data, 0xF);
//    }
//	else if (os_strcmp(argv[1], "set_rsp_data") == 0)
//	{
//		adv_data[0] = 0x07;
//		adv_data[1] = 0x08;
//		memcpy(&adv_data[2], "7231N", 6);
//        ble_appm_set_scan_rsp_data(app_ble_ctx.adv_actv_idx, adv_data, 0x8);
//	}
//	else if (os_strcmp(argv[1], "adv_start") == 0)
//	{
//		ble_appm_start_advertising(app_ble_ctx.adv_actv_idx, 0);
//	}
//	else
        if (os_strcmp(argv[1], "dut") == 0)
	{
		ble_dut_start();
	}
        else 
        {
            bk_printf("only support [ble dut] cmd\r\n"); 
        }
}

SH_CMD_EXPORT(ble, ble, "ble command");

#endif
#endif
