NAME := beken

$(NAME)_TYPE := kernel

-include $(SOURCE_ROOT)/platform/mcu/$(HOST_MCU_FAMILY)/.config

WPA_VERSION := wpa_supplicant-2.9
ifeq ($(CFG_USE_WPA_29),0)
WPA_VERSION := hostapd-2.5
endif

$(NAME)_INCLUDES := app/standalone-ap \
					app/standalone-station \
					driver/sdio \
					driver/uart \
					driver/sys_ctrl \
					driver/gpio \
					driver/common/reg \
					driver/dma \
					driver/intc \
					driver/phy \
					driver/rc_beken \
					driver/general_dma \
					driver/spidma \
					driver/rw_pub \
					driver/icu \
					driver/i2c \
					driver/calendar \
					func/sdio_intf \
					func/power_save \
					func/temp_detect \
					func/saradc_intf \
					func/spidma_intf \
					func/ethernet_intf \
					func/rwnx_intf \
					func/rf_test \
					func/camera_intf \
					func/video_transfer \
					func/user_driver \
					func/ble_wifi_exchange

$(NAME)_INCLUDES += ip/ke \
					ip/mac \
					ip/lmac/src/hal \
					ip/lmac/src/mm \
					ip/lmac/src/ps \
					ip/lmac/src/rd \
					ip/lmac/src/rwnx \
					ip/lmac/src/rx \
					ip/lmac/src/scan \
					ip/lmac/src/sta \
					ip/lmac/src/tx \
					ip/lmac/src/vif \
					ip/lmac/src/rx/rxl \
					ip/lmac/src/tx/txl \
					ip/lmac/src/tpc \
					ip/lmac/src/tdls \
					ip/lmac/src/p2p \
					ip/lmac/src/chan \
					ip/lmac/src/td \
					ip/umac/src/bam \
					ip/umac/src/llc \
					ip/umac/src/me \
					ip/umac/src/rxu \
					ip/umac/src/scanu \
					ip/umac/src/sm \
					ip/umac/src/txu \
					ip/umac/src/apm \
					ip/umac/src/mesh \
					ip/umac/src/rc

$(NAME)_SOURCES :=  app/app.c \
					app/config/param_config.c \
					app/standalone-ap/sa_ap.c \
					app/standalone-station/sa_station.c \
					driver/common/dd.c \
					driver/common/drv_model.c \
					driver/dma/dma.c \
					driver/driver.c \
					driver/flash/flash.c \
					driver/general_dma/general_dma.c \
					driver/gpio/gpio.c \
					driver/i2c/i2c1.c \
					driver/i2c/i2c2.c \
					driver/icu/icu.c \
					driver/irda/irda.c \
					driver/macphy_bypass/mac_phy_bypass.c \
					driver/phy/phy_trident.c \
					driver/pwm/pwm.c \
					driver/pwm/pwm_new.c \
					driver/pwm/bk_timer.c \
					driver/pwm/mcu_ps_timer.c \
					driver/saradc/saradc.c \
					driver/sdio/sdio.c \
					driver/sdio/sdma.c \
					driver/sdio/sutil.c \
					driver/spi/spi.c \
					driver/spi/spi_master.c \
					driver/spi/spi_slave.c \
					driver/spidma/spidma.c \
					driver/sys_ctrl/sys_ctrl.c \
					driver/uart/Retarget.c \
					driver/uart/uart.c \
					driver/wdt/wdt.c \
					driver/rw_pub/rw_platf_pub.c \
					driver/intc/intc.c \
					driver/calendar/calendar.c \
					func/bk7011_cal/bk7231_cal.c \
					func/bk7011_cal/bk7231U_cal.c \
					func/bk7011_cal/bk7231N_cal.c \
					func/bk7011_cal/bk7221U_cal.c \
					func/bk7011_cal/manual_cal_bk7231.c \
					func/bk7011_cal/manual_cal_bk7231U.c \
					func/func.c \
					func/hostapd_intf/hostapd_intf.c \
					func/misc/fake_clock.c \
					func/misc/target_util.c \
					func/misc/start_type.c \
					func/rf_test/rx_sensitivity.c \
					func/rf_test/tx_evm.c \
					func/rwnx_intf/rw_ieee80211.c \
					func/rwnx_intf/rw_msdu.c \
					func/rwnx_intf/rw_msg_rx.c \
					func/rwnx_intf/rw_msg_tx.c \
					func/sdio_intf/sdio_intf.c \
					func/sdio_trans/sdio_trans.c \
					func/sim_uart/gpio_uart.c \
					func/sim_uart/pwm_uart.c \
					func/spidma_intf/spidma_intf.c \
					func/temp_detect/temp_detect.c \
					func/saradc_intf/saradc_intf.c \
					func/uart_debug/cmd_evm.c \
					func/uart_debug/cmd_help.c \
					func/uart_debug/cmd_reg.c \
					func/uart_debug/cmd_rx_sensitivity.c \
					func/uart_debug/command_line.c \
					func/uart_debug/command_table.c \
					func/uart_debug/udebug.c \
					func/power_save/power_save.c \
					func/power_save/mcu_ps.c \
					func/power_save/manual_ps.c \
					func/power_save/ap_idle.c \
					func/wlan_ui/wlan_ui.c \
					func/net_param_intf/net_param.c \
					func/lwip_intf/dhcpd/dhcp-server-main.c \
					func/lwip_intf/dhcpd/dhcp-server.c \
					func/camera_intf/camera_intf.c \
					func/video_transfer/video_transfer.c \
					func/video_transfer/video_buffer.c \
					func/rf_use/arbitrate.c \
					func/ble_wifi_exchange/ble_wifi_port.c

$(NAME)_INCLUDES += func/$(WPA_VERSION)/src \
					func/$(WPA_VERSION)/src/ap \
					func/$(WPA_VERSION)/src/utils \
					func/$(WPA_VERSION)/src/common \
					func/$(WPA_VERSION)/src/drivers \
					func/$(WPA_VERSION)/bk_patch \
					func/$(WPA_VERSION)/hostapd \
					func/$(WPA_VERSION)/wpa_supplicant

$(NAME)_SOURCES +=  func/$(WPA_VERSION)/bk_patch/ddrv.c \
					func/$(WPA_VERSION)/bk_patch/signal.c \
					func/$(WPA_VERSION)/bk_patch/sk_intf.c \
					func/$(WPA_VERSION)/bk_patch/fake_socket.c \
					func/$(WPA_VERSION)/src/common/hw_features_common.c \
					func/$(WPA_VERSION)/src/common/ieee802_11_common.c \
					func/$(WPA_VERSION)/src/common/wpa_common.c \
					func/$(WPA_VERSION)/src/common/notifier.c \
					func/$(WPA_VERSION)/src/common/wpa_psk_cache.c \
					func/$(WPA_VERSION)/src/crypto/aes-unwrap.c \
					func/$(WPA_VERSION)/src/crypto/rc4.c \
					func/$(WPA_VERSION)/src/crypto/sha1-pbkdf2.c \
					func/$(WPA_VERSION)/src/crypto/sha1-prf.c \
					func/$(WPA_VERSION)/src/crypto/tls_none.c \
					func/$(WPA_VERSION)/src/crypto/aes-wrap.c \
					func/$(WPA_VERSION)/src/drivers/driver_beken.c \
					func/$(WPA_VERSION)/src/drivers/driver_common.c \
					func/$(WPA_VERSION)/src/drivers/drivers.c \
					func/$(WPA_VERSION)/src/l2_packet/l2_packet_none.c \
					func/$(WPA_VERSION)/src/rsn_supp/pmksa_cache.c \
					func/$(WPA_VERSION)/src/rsn_supp/wpa.c \
					func/$(WPA_VERSION)/src/rsn_supp/wpa_ie.c \
					func/$(WPA_VERSION)/src/utils/common.c \
					func/$(WPA_VERSION)/src/utils/eloop.c \
					func/$(WPA_VERSION)/src/utils/os_none.c \
					func/$(WPA_VERSION)/src/utils/wpabuf.c \
					func/$(WPA_VERSION)/src/utils/wpa_debug.c \
					func/$(WPA_VERSION)/src/ap/ap_config.c \
					func/$(WPA_VERSION)/src/ap/ap_drv_ops.c \
					func/$(WPA_VERSION)/src/ap/ap_list.c \
					func/$(WPA_VERSION)/src/ap/ap_mlme.c \
					func/$(WPA_VERSION)/src/ap/beacon.c \
					func/$(WPA_VERSION)/src/ap/drv_callbacks.c \
					func/$(WPA_VERSION)/src/ap/hostapd.c \
					func/$(WPA_VERSION)/src/ap/hw_features.c \
					func/$(WPA_VERSION)/src/ap/ieee802_11.c \
					func/$(WPA_VERSION)/src/ap/ieee802_11_auth.c \
					func/$(WPA_VERSION)/src/ap/ieee802_11_ht.c \
					func/$(WPA_VERSION)/src/ap/ieee802_11_shared.c \
					func/$(WPA_VERSION)/src/ap/ieee802_1x.c \
					func/$(WPA_VERSION)/src/ap/pmksa_cache_auth.c \
					func/$(WPA_VERSION)/src/ap/sta_info.c \
					func/$(WPA_VERSION)/src/ap/tkip_countermeasures.c \
					func/$(WPA_VERSION)/src/ap/utils.c \
					func/$(WPA_VERSION)/src/ap/wmm.c \
					func/$(WPA_VERSION)/src/ap/wpa_auth.c \
					func/$(WPA_VERSION)/src/ap/wpa_auth_glue.c \
					func/$(WPA_VERSION)/src/ap/wpa_auth_ie.c \
					func/$(WPA_VERSION)/hostapd/main_none.c \
					func/$(WPA_VERSION)/wpa_supplicant/blacklist.c \
					func/$(WPA_VERSION)/wpa_supplicant/bss.c \
					func/$(WPA_VERSION)/wpa_supplicant/config.c \
					func/$(WPA_VERSION)/wpa_supplicant/config_none.c \
					func/$(WPA_VERSION)/wpa_supplicant/events.c \
					func/$(WPA_VERSION)/wpa_supplicant/main_supplicant.c \
					func/$(WPA_VERSION)/wpa_supplicant/notify.c \
					func/$(WPA_VERSION)/wpa_supplicant/wmm_ac.c \
					func/$(WPA_VERSION)/wpa_supplicant/wpa_scan.c \
					func/$(WPA_VERSION)/wpa_supplicant/wpa_supplicant.c \
					func/$(WPA_VERSION)/wpa_supplicant/wpas_glue.c \
					func/$(WPA_VERSION)/wpa_supplicant/ctrl_iface.c \
					func/$(WPA_VERSION)/wpa_supplicant/wlan.c \
					func/$(WPA_VERSION)/wpa_supplicant/sme.c					

ifneq ($(CFG_USE_WPA_29), 0)
$(NAME)_SOURCES +=  func/$(WPA_VERSION)/wpa_supplicant/op_classes.c
endif

# for WPA3
ifeq ($(CFG_WPA3), 1)
$(NAME)_SOURCES +=  func/$(WPA_VERSION)/src/common/sae.c \
					func/$(WPA_VERSION)/src/crypto/aes-ctr.c \
					func/$(WPA_VERSION)/src/crypto/aes-omac1.c \
					func/$(WPA_VERSION)/src/crypto/aes-siv.c \
					func/$(WPA_VERSION)/src/crypto/crypto_wolfssl.c \
					func/$(WPA_VERSION)/src/crypto/dh_group5.c \
					func/$(WPA_VERSION)/src/crypto/dh_groups.c \
					func/$(WPA_VERSION)/src/crypto/sha256.c \
					func/$(WPA_VERSION)/src/crypto/sha256-internal.c \
					func/$(WPA_VERSION)/src/crypto/sha256-prf.c
endif

$(NAME)_SOURCES +=  func/$(WPA_VERSION)/src/crypto/crypto_ali.c \
					alios/lwip-2.0.2/port/ethernetif.c \
					alios/lwip-2.0.2/port/net.c \
					alios/lwip-2.0.2/apps/ping/ping.c \
					alios/os/mem_arch.c \
					alios/os/str_arch.c \
					alios/flash_hal.c \
					alios/entry/arch_main.c
