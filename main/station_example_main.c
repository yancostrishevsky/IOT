#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_http_client.h"

#define WEB_SERVER "www.example.com"
#define WEB_PORT "80"
#define WEB_URL "http://www.example.com/"

#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD


static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT     BIT0
#define WIFI_FAIL_BIT          BIT1
static const char *TAG = "WIFI";

#define WIFI_NO_CONNECTION_BIT BIT2

static void event_handler(void* arg, esp_event_base_t event_base,
                         int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT | WIFI_NO_CONNECTION_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupClearBits(s_wifi_event_group, WIFI_NO_CONNECTION_BIT);
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}



void http_get_task(void *pvParameters)
{
    while(1) {
        EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                WIFI_CONNECTED_BIT,
                pdFALSE,
                pdFALSE,
                portMAX_DELAY);

        if(bits & WIFI_CONNECTED_BIT) {
            ESP_LOGI(TAG, "connected to AP SSID:%s password:%s",
                     EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);

            esp_http_client_config_t config = {
    .url = WEB_URL,
    .timeout_ms = 20000,  // Set timeout to 10 seconds (or higher)
};

        



            esp_http_client_handle_t client = esp_http_client_init(&config);

            esp_err_t err = esp_http_client_perform(client);

            if(err == ESP_OK) {
                ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %lld",
                         esp_http_client_get_status_code(client),
                         esp_http_client_get_content_length(client));

                // Read the response
char *buffer;
int content_length = esp_http_client_get_content_length(client);
if (content_length > 0) {
    buffer = malloc(content_length + 1); // Allocate memory for the buffer
    if (buffer != NULL) {
        int read_len = esp_http_client_read(client, buffer, content_length);
        if(read_len == content_length) {
            buffer[read_len] = '\0'; // Null-terminate the string
            ESP_LOGI(TAG, "Read %d bytes: %s", read_len, buffer); // Print the response
        } else {
            ESP_LOGE(TAG, "Error reading response");
        }
        free(buffer); // Free the buffer
    } else {
        ESP_LOGE(TAG, "Could not allocate memory for the buffer");
    }
} else {
    ESP_LOGE(TAG, "Invalid content length");
}

            }else {
                ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
            }

            esp_http_client_cleanup(client);
        }

        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}


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
            vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second if there's a Wi-Fi connection
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
    xTaskCreate(&http_get_task, "http_get_task", 4096, NULL, 6, NULL);

    xTaskCreate(&led_blink_task, "led_blink_task", 2048, NULL, 5, NULL);
}
