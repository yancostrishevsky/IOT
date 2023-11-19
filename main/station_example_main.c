#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "sdkconfig.h"

// request data
#define WEB_SERVER "example.com"
#define WEB_PORT "80"
#define WEB_PATH "/"

// wifi keys
#define EXAMPLE_ESP_WIFI_SSID      "STUPKI"
#define EXAMPLE_ESP_WIFI_PASS      "kochamstopy"

// connection bits
#define WIFI_CONNECTED_BIT     BIT0
#define WIFI_FAIL_BIT          BIT1
#define WIFI_NO_CONNECTION_BIT BIT2

static EventGroupHandle_t s_wifi_event_group;

static const char *TAG = "IOT PROJECT";

static bool is_connected = false;


/// Event handler for wfi connection monitoring.
static void event_handler(void* arg, esp_event_base_t event_base,
                         int32_t event_id, void* event_data)
{
    // Connected to wifi
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();

    // Disconnected
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT | WIFI_NO_CONNECTION_BIT);
        is_connected = false;

    // Got ip
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupClearBits(s_wifi_event_group, WIFI_NO_CONNECTION_BIT);
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        is_connected = true;
    }
}



// Attempts to send HTTP request.
static void http_get_task(void *pvParameters)
{
    const static struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };

    struct addrinfo *res;
    struct in_addr *addr;
    int socket_id, read_res;
    char recv_buf[64];

    while(1) {
        EventBits_t bits = xEventGroupGetBits(s_wifi_event_group);

        if(bits &WIFI_NO_CONNECTION_BIT){
            vTaskDelay(1000 / portTICK_PERIOD_MS);//zeby nie probowac nawiazac polaczenia przy braku wifi
            continue;
        }


        int err = getaddrinfo(WEB_SERVER, WEB_PORT, &hints, &res);

        if(err != 0 || res == NULL) {
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        /* Code to print the resolved IP.

           Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

        socket_id = socket(res->ai_family, res->ai_socktype, 0);
        if(socket_id < 0) {
            ESP_LOGE(TAG, "... Failed to allocate socket.");
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... allocated socket");

        if(connect(socket_id, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
            close(socket_id);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG, "... connected");
        freeaddrinfo(res);

        static const char *REQUEST = "GET " WEB_PATH " HTTP/1.0\r\n"
                                    "Host: "WEB_SERVER":"WEB_PORT"\r\n"
                                    "User-Agent: esp-idf/1.0 esp32\r\n"
                                    "\r\n";

        if (write(socket_id, REQUEST, strlen(REQUEST)) < 0) {
            ESP_LOGE(TAG, "... socket send failed");
            close(socket_id);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... socket send success");

        struct timeval receiving_timeout;
        receiving_timeout.tv_sec = 5;
        receiving_timeout.tv_usec = 0;
        if (setsockopt(socket_id, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
                sizeof(receiving_timeout)) < 0) {
            ESP_LOGE(TAG, "... failed to set socket receiving timeout");
            close(socket_id);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... set socket receiving timeout success");

        /* Read HTTP response */
        do {
            bzero(recv_buf, sizeof(recv_buf));
            read_res = read(socket_id, recv_buf, sizeof(recv_buf)-1);
            for(int i = 0; i < read_res; i++) {
                putchar(recv_buf[i]);
            }
        } while(read_res > 0);

        ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d.", read_res, errno);
        close(socket_id);
        for(int countdown = 10; countdown >= 0; countdown--) {
            ESP_LOGI(TAG, "%d... ", countdown);
            vTaskDelay(1000 / portTICK_PERIOD_MS);

            bits = xEventGroupGetBits(s_wifi_event_group);

            if(bits &WIFI_NO_CONNECTION_BIT){
                ESP_LOGI(TAG, "Connection lost. Waiting for connection...");
                break;
            }
        }
        ESP_LOGI(TAG, "Starting again!");

    }
}

// Init wifi connection
void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}

// Starts to monitor connection and adjusts LED blinking to it.
void led_blink_task(void *pvParameter)
{
    while (1) {
        EventBits_t bits = xEventGroupGetBits(s_wifi_event_group);

        if (bits & WIFI_NO_CONNECTION_BIT) {
            gpio_set_level(GPIO_NUM_2, 1); // Turn on the LED on GPIO 2
            vTaskDelay(pdMS_TO_TICKS(500));  // Delay for 500 ms
            gpio_set_level(GPIO_NUM_2, 0); // Turn off the LED on GPIO 2
            vTaskDelay(pdMS_TO_TICKS(500));  // Delay for 500 ms
        } else {
            vTaskDelay(pdMS_TO_TICKS(100)); // Delay for 1 second if there's a Wi-Fi connection
        }
    }
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    esp_rom_gpio_pad_select_gpio(GPIO_NUM_2);
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
    xTaskCreate(&http_get_task, "http_get_task", 4096, NULL, 5, NULL);

    xTaskCreate(&led_blink_task, "led_blink_task", 2048, NULL, 5, NULL);
}