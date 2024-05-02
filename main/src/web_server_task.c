//==================================================================================================
/// @file       web_server_task.c
/// @brief      Web server task
//==================================================================================================

//==================================================================================================
// Include definition
//==================================================================================================
#include "mount.h"
#include "file_server.h"
#include "web_server_task.h"
#include <stdio.h> 
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "mdns.h"
#include "lwip/sockets.h"
#include "arpa/inet.h"
#include "lwip/ip4_addr.h"
#include "protocol_examples_common.h"


//==================================================================================================
// Constant definition
//==================================================================================================
/* mDNS hostname */
#define MDNS_HOSTNAME               ( CONFIG_MDNS_HOSTNAME )

/* AP mode or ST mode */
#ifdef CONFIG_WIFI_AP_MODE_ENABLED
#define WIFI_AP_MODE_ENABLED        ( CONFIG_WIFI_AP_MODE_ENABLED) 
#else
#define WIFI_AP_MODE_ENABLED        ( false )
#endif

/* Main board SSID or Sub board SSID */
#ifdef CONFIG_WIFI_AP_SUB_SSID_ENABLED
#define WIFI_AP_SSID                ( CONFIG_WIFI_AP_SUB_SSID )
#else
#define WIFI_AP_SSID                ( CONFIG_WIFI_AP_MAIN_SSID )
#endif

/* WiFi AP parameters */
#define WIFI_AP_STATIC_IP           ( CONFIG_WIFI_AP_STATIC_IP )
#define WIFI_AP_NET_MASK            ( CONFIG_WIFI_AP_NET_MASK )
#define WIFI_AP_PASSWORD            ( CONFIG_WIFI_AP_PASSWORD )
#define WIFI_AP_CHANNEL             ( CONFIG_WIFI_AP_CHANNEL )
#define WIFI_AP_MAX_STA_CONN        ( CONFIG_WIFI_AP_MAX_STA_CONN )

/* FW version */
#define FW_VERSION_STR              ( CONFIG_FW_VERSION )


//==================================================================================================
// Prototype
//==================================================================================================
void web_server_task( void );
static esp_err_t start_mdns_service( void );
static void init_wifi_soft_ap( void );
static void get_tag_string( void );


//==================================================================================================
// Private values definition
//==================================================================================================
/* TAG string for console log */
static const char WEB_SERVER_TAG_STR[] = "WEB SERVER";
static char WEB_SERVER_TAG[50];

//==================================================================================================
// Public functions
//==================================================================================================
void web_server_task_start( void *pvParameters )  {
    // Start task
    web_server_task();
}

//==================================================================================================
// Private functions
//==================================================================================================

static esp_err_t start_mdns_service( void ) {
    // Initialize mDNS service
    esp_err_t err = mdns_init();
    if (err) {
        printf("MDNS Init failed: %d\n", err);
        return err;
    }

    // Set hostname
    mdns_hostname_set(MDNS_HOSTNAME);
    // Set default instance
    mdns_instance_name_set("ESP32 Web Server");

    return ESP_OK;
}

static void init_wifi_soft_ap( void ) {
    esp_netif_t* wifiAP = esp_netif_create_default_wifi_ap();
    esp_netif_ip_info_t ipInfo;

    inet_pton(AF_INET, WIFI_AP_STATIC_IP, &ipInfo.ip);
    inet_pton(AF_INET, WIFI_AP_NET_MASK, &ipInfo.netmask);
    ipInfo.gw.addr = ipInfo.ip.addr;
    esp_netif_dhcps_stop(wifiAP);
	esp_netif_set_ip_info(wifiAP, &ipInfo);
	esp_netif_dhcps_start(wifiAP);
    ESP_LOGI(WEB_SERVER_TAG, "esp_netif_create_default_wifi_ap end");
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_LOGI(WEB_SERVER_TAG, "esp_wifi_init end");

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_AP_SSID,
            .ssid_len = strlen(WIFI_AP_SSID),
            .channel = WIFI_AP_CHANNEL,
            .password = WIFI_AP_PASSWORD,
            .max_connection = WIFI_AP_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen(WIFI_AP_PASSWORD) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(WEB_SERVER_TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             WIFI_AP_SSID, WIFI_AP_PASSWORD, WIFI_AP_CHANNEL);
}

static void get_tag_string( void ) {
    sprintf(WEB_SERVER_TAG, "FW v%s: %s", FW_VERSION_STR, WEB_SERVER_TAG_STR);
}

//--------------------------------------------------------------------------------------------------
/// @brief  Web server task
//--------------------------------------------------------------------------------------------------
void web_server_task( void )  {
    // Get TAG string for log info
    get_tag_string();

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* Initialize file storage */
    const char* base_path = "/data";
    ESP_ERROR_CHECK(example_mount_storage(base_path));

    if( WIFI_AP_MODE_ENABLED ) {
        // AP mode
        ESP_LOGI(WEB_SERVER_TAG, "WiFi AP Mode");
        init_wifi_soft_ap();

    } else {
        // ST mode
        ESP_LOGI(WEB_SERVER_TAG, "WiFi ST Mode");
        ESP_ERROR_CHECK(example_connect());
    }

    // Set hostname with mDNS
    start_mdns_service();

    /* Start the file server */
    ESP_ERROR_CHECK(example_start_file_server(base_path));
    ESP_LOGI(WEB_SERVER_TAG, "Web server started");

    while( 1 ) {
        // Wait for a short period to allow other tasks to run
        // This loop keeps the web server task alive without consuming excessive CPU resources
        vTaskDelay( 100 );
    }
}