/*
 * Copyright (c) 2022-present joaocarlosfr. 
 * 
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

#include "esp_err.h" 
#include "nvs_flash.h"
#include <esp_task_wdt.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#include "bme280.h"
#include "bh1750.h"
#include "rainsensor.h"
#include "wifi.h"
#include "mqtt.h"

SemaphoreHandle_t conexaoWiFi;
SemaphoreHandle_t conexaoMQTT;

static void task1(void *param)
{
    char mensagem[50];
    bme280_start();
    bh1750_start();
    rainsensor_start();
    esp_task_wdt_add(NULL); // Habilita o monitoramento do Task WDT nesta tarefa
    float temp, pabs, umid, lux, rain;
    while(1)
    {
        if(xSemaphoreTake(conexaoWiFi, portMAX_DELAY))
        {
            mqtt_start();
        }
        if(xSemaphoreTake(conexaoMQTT, portMAX_DELAY))
        {
            while(1)
            {
                bme280_read(&temp, &pabs, &umid);
                bh1750_read(&lux);
                rainsensor_read(&rain);
                printf("Temperatura: %f\n", temp);
                printf("Pressão: %f\n", pabs);
                printf("Umidade: %f\n",umid);
                printf("Lux: %f\n",lux);
                printf("Rain: %f\n",rain);
                printf("Ok\n");
                sprintf(mensagem, "%d", (int)rain);
                mqtt_envia_mensagem("topic/chuva", mensagem);
                sprintf(mensagem, "%.2f", (float)temp);
                mqtt_envia_mensagem("topic/temperatura", mensagem);
                sprintf(mensagem, "%.2f", (float)umid);
                mqtt_envia_mensagem("topic/umidade", mensagem);
                sprintf(mensagem, "%.2f", (float)pabs);
                mqtt_envia_mensagem("topic/pressao", mensagem);
                sprintf(mensagem, "%.2f", (float)lux);
                mqtt_envia_mensagem("topic/luminosidade", mensagem);
                esp_task_wdt_reset(); // Alimenta o WDT
                vTaskDelay(60000 / portTICK_RATE_MS);
            }
        }
    }
}

void app_main(void)
{
    
    esp_task_wdt_init(180, true); //Inicia o Task WDT com 180seg

    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);
    
    conexaoWiFi = xSemaphoreCreateBinary();
    conexaoMQTT = xSemaphoreCreateBinary();

    wifi_start();

    xTaskCreate(&task1, "t1", 4096, NULL, 1, NULL);
}
