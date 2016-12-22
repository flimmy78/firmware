/**
 ******************************************************************************
  Copyright (c) 2013-2014 IntoRobot Team.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, see <http://www.gnu.org/licenses/>.
  ******************************************************************************
*/

#include "hw_config.h"
#include "wlan_hal.h"
#include "core_esp8266_wifi_generic.h"
#include "flash_map.h"
#include "memory_hal.h"
#include "macaddr_hal.h"
#include "delay_hal.h"

static volatile uint32_t wlan_timeout_start;
static volatile uint32_t wlan_timeout_duration;

inline void WLAN_TIMEOUT(uint32_t dur) {
    wlan_timeout_start = HAL_Timer_Get_Milli_Seconds();
    wlan_timeout_duration = dur;
    DEBUG("WLAN WD Set %d",(dur));
}
inline bool IS_WLAN_TIMEOUT() {
    return wlan_timeout_duration && ((HAL_Timer_Get_Milli_Seconds()-wlan_timeout_start)>wlan_timeout_duration);
}

inline void CLR_WLAN_TIMEOUT() {
    wlan_timeout_duration = 0;
    DEBUG("WLAN WD Cleared, was %d", wlan_timeout_duration);
}

uint32_t HAL_NET_SetNetWatchDog(uint32_t timeOutInMS)
{
    return 0;
}

int wlan_clear_credentials()
{
    return 0;
}

int wlan_has_credentials()
{
    return 0;
}

wlan_result_t wlan_activate()
{
    return 0;
}

wlan_result_t wlan_deactivate()
{
    return 0;
}

int wlan_connect()
{
    int result = 0;
    if(wlan_status()) {
        result = esp8266_connect();
        return result;
    }
    else {
        return 0;
    }
}

wlan_result_t wlan_disconnect()
{
    return esp8266_disconnect();
}

int wlan_status()
{
    wl_status_t status = esp8266_status();
    switch(status) {
        case WL_CONNECTED:
            return 0;
        default:
            return 1;
    }
    return 0;
}

int wlan_connected_rssi(void)
{
    return esp8266_getRSSI();
}

int wlan_set_credentials(WLanCredentials* c)
{
    struct station_config conf;
    strcpy((char*)(conf.ssid), c->ssid);

    if(c->password) {
        if (strlen(c->password) == 64) // it's not a passphrase, is the PSK
            memcpy((char*)(conf.password), c->password, 64);
        else
            strcpy((char*)(conf.password), c->password);
    } else {
        *conf.password = 0;
    }
    conf.bssid_set = 0;
    ETS_UART_INTR_DISABLE();
    // workaround for #1997: make sure the value of ap_number is updated and written to flash
    // to be removed after SDK update
    wifi_station_ap_number_set(2);
    wifi_station_ap_number_set(1);

    wifi_station_set_config(&conf);
    ETS_UART_INTR_ENABLE();
    return 0;
}

void wlan_Imlink_start()
{
    HAL_Delay_Milliseconds(2000);  //调用连接后需要延时一段时间，否则直接进入imlink模式，配置不了，不确定是否还会影响其他功能。chenkaiyao 2016-12-16
    esp8266_beginSmartConfig();
}

imlink_status_t wlan_Imlink_get_status()
{
    if(!esp8266_smartConfigDone())
    return IMLINK_DOING;
    else
    return IMLINK_SUCCESS;
}

void wlan_Imlink_stop()
{
    esp8266_stopSmartConfig();
}

void wlan_setup()
{
    esp8266_setMode(WIFI_STA);
    esp8266_setDHCP(true);
    esp8266_setAutoConnect(true);
    esp8266_setAutoReconnect(true);
}

void wlan_fetch_ipconfig(WLanConfig* config)
{
    memset(config, 0, sizeof(WLanConfig));
    config->size = sizeof(WLanConfig);

    struct ip_info ip;
    wifi_get_ip_info(STATION_IF, &ip);
    config->nw.aucIP.ipv4 = ip.ip.addr;
    config->nw.aucSubnetMask.ipv4 = ip.netmask.addr;
    config->nw.aucDefaultGateway.ipv4 = ip.gw.addr;

    ip_addr_t dns_ip = dns_getserver(0);
    config->nw.aucDNSServer.ipv4 = dns_ip.addr;
    wifi_get_macaddr(STATION_IF, config->nw.uaMacAddr);

    struct station_config conf;
    wifi_station_get_config(&conf);
    memcpy(config->uaSSID, conf.ssid, 32);
    memcpy(config->BSSID, conf.bssid, 6);
}

void wlan_set_error_count(uint32_t errorCount)
{
}


/**
 * Sets the IP source - static or dynamic.
 */
void wlan_set_ipaddress_source(IPAddressSource source, bool persist, void* reserved)
{
}

/**
 * Sets the IP Addresses to use when the device is in static IP mode.
 * @param device
 * @param netmask
 * @param gateway
 * @param dns1
 * @param dns2
 * @param reserved
 */
void wlan_set_ipaddress(const HAL_IPAddress* device, const HAL_IPAddress* netmask,
        const HAL_IPAddress* gateway, const HAL_IPAddress* dns1, const HAL_IPAddress* dns2, void* reserved)
{

}

WLanSecurityType toSecurityType(AUTH_MODE authmode)
{
    switch(authmode)
    {
        case AUTH_OPEN:
            return WLAN_SEC_UNSEC;
            break;
        case AUTH_WEP:
            return WLAN_SEC_WEP;
            break;
        case AUTH_WPA_PSK:
            return WLAN_SEC_WPA;
            break;
        case AUTH_WPA2_PSK:
        case AUTH_WPA_WPA2_PSK:
            return WLAN_SEC_WPA2;
            break;
        case AUTH_MAX:
            return WLAN_SEC_NOT_SET;
            break;
    }
}

WLanSecurityCipher toCipherType(AUTH_MODE authmode)
{
    switch(authmode)
    {
        case AUTH_WEP:
            return WLAN_CIPHER_AES;
            break;
        case AUTH_WPA_PSK:
            return WLAN_CIPHER_TKIP;
            break;
        default:
            break;
    }
    return WLAN_CIPHER_NOT_SET;
}

struct WlanScanInfo
{
    wlan_scan_result_t callback;
    void* callback_data;
    int count;
    bool completed;
};

WlanScanInfo scanInfo;
void scan_done_cb(void *arg, STATUS status)
{
    WiFiAccessPoint data;

    if(status == OK)
    {
        int i = 0;
        bss_info *it = (bss_info*)arg;
        for(; it; it = STAILQ_NEXT(it, next), i++)
        {
            memset(&data, 0, sizeof(WiFiAccessPoint));
            memcpy(data.ssid, it->ssid, it->ssid_len);
            data.ssidLength = it->ssid_len;
            memcpy(data.bssid, it->bssid, 6);
            data.security = toSecurityType(it->authmode);
            data.cipher = toCipherType(it->authmode);
            data.channel = it->channel;
            data.rssi = it->rssi;
            scanInfo.callback(&data, scanInfo.callback_data);
        }
        scanInfo.count = i;
        scanInfo.completed = true;
    }
}

extern "C" {
void optimistic_yield(uint32_t interval_us);
}

int wlan_scan(wlan_scan_result_t callback, void* cookie)
{
    esp8266_setMode(WIFI_STA);
    memset((void *)&scanInfo, 0, sizeof(struct WlanScanInfo));
    scanInfo.callback = callback;
    scanInfo.callback_data = cookie;
    scanInfo.count = 0;
    scanInfo.completed = false;
    if(wifi_station_scan(NULL, scan_done_cb))
    {
        WLAN_TIMEOUT(5000);
        while(!scanInfo.completed)
        {
            optimistic_yield(100);
            if(IS_WLAN_TIMEOUT()) {
                CLR_WLAN_TIMEOUT();
                break;
            }
        }
        return scanInfo.count;
    }
    else
    {
        return -1;
    }
}

/**
 * Lists all WLAN credentials currently stored on the device
 */
int wlan_get_credentials(wlan_scan_result_t callback, void* callback_data)
{
    return 0;
}

#define STATION_IF      0x00
#define SOFTAP_IF       0x01

/**
 * wifi set station and ap mac addr
 */
int wlan_set_macaddr(uint8_t *stamacaddr, uint8_t *apmacaddr)
{
    if (stamacaddr != NULL && apmacaddr != NULL){
        mac_param_t mac_addrs;
        mac_addrs.header = FLASH_MAC_HEADER;

        memset(mac_addrs.stamac_addrs, 0, sizeof(mac_addrs.stamac_addrs));
        memcpy(mac_addrs.stamac_addrs, stamacaddr, sizeof(mac_addrs.stamac_addrs));

        memset(mac_addrs.apmac_addrs, 0, sizeof(mac_addrs.apmac_addrs));
        memcpy(mac_addrs.apmac_addrs, apmacaddr, sizeof(mac_addrs.apmac_addrs));

        uint32_t len = sizeof(mac_addrs);
        HAL_FLASH_Interminal_Erase(HAL_FLASH_Interminal_Get_Sector(FLASH_MAC_START_ADDR));
        HAL_FLASH_Interminal_Write(FLASH_MAC_START_ADDR, (uint32_t *)&mac_addrs, len);
        return 0;
    }
    return -1;
}

/**
 * wifi get station and ap mac addr
 */
int wlan_get_macaddr(uint8_t *stamacaddr, uint8_t *apmacaddr)
{
    if(!wifi_get_macaddr(STATION_IF, stamacaddr))
    {
        return -1;
    }
    if(!wifi_get_macaddr(SOFTAP_IF, apmacaddr))
    {
        return -1;
    }
    return 0;
}

int wlan_set_macaddr_from_flash(uint8_t *stamacaddr, uint8_t *apmacaddr)
{
    if(!wifi_set_macaddr(STATION_IF, stamacaddr))
        {
            return -1;
        }
    if(!wifi_set_macaddr(SOFTAP_IF, apmacaddr))
        {
            return -1;
        }
    return 0;
}

int wlan_set_macaddr_when_init(void)
{
    mac_param_t mac_addrs;
    HAL_FLASH_Interminal_Read(FLASH_MAC_START_ADDR, (uint32_t *)&mac_addrs, sizeof(mac_addrs));
    if (FLASH_MAC_HEADER == mac_addrs.header){
        esp8266_setMode(WIFI_AP_STA);
        wlan_set_macaddr_from_flash(mac_addrs.stamac_addrs, mac_addrs.apmac_addrs);
        esp8266_setMode(WIFI_STA);
        // for (int i = 0; i < 6; i++){
        //     DEBUG("stamac: %x", mac_addrs.stamac_addrs[i]);
        // }
        // HAL_FLASH_Interminal_Erase(HAL_FLASH_Interminal_Get_Sector(FLASH_MAC_START_ADDR));
    }
}
