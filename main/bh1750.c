/*
 * Copyright (c) 2022-present joaocarlosfr. 
 * 
 * SPDX-License-Identifier: MIT
 */

#include "bh1750.h"

#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_system.h"
#include "driver/i2c.h"

#define BH1750_ADDR           0x23               // Address do Sensor     
#define I2C_MASTER_PORT       0                  // Número do mestre
#define WRITE_BIT             I2C_MASTER_WRITE   // Bit de escrita
#define READ_BIT              I2C_MASTER_READ    // Bit de leitura
#define ACK_CHECK_EN          0x1                // ACK Enable
#define ACK_CHECK_DIS         0x0                // ACK Disable
#define ACK_VAL               0x0                // ACK Check
#define NACK_VAL              0x1                // NACK Enable

/**
 * @brief Escrita I2C
 * 
 * @param data byte para escrita
 * 
 * @return esp_err_t 
 */
static esp_err_t i2c_write_bh1750(uint8_t data)
{
   int ret;
   i2c_cmd_handle_t cmd = i2c_cmd_link_create();
   i2c_master_start(cmd);       
   i2c_master_write_byte(cmd, BH1750_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);
   i2c_master_write_byte(cmd, data, ACK_CHECK_EN);
   i2c_master_stop(cmd);
   ret = i2c_master_cmd_begin(I2C_MASTER_PORT, cmd, 1000 / portTICK_RATE_MS);
   i2c_cmd_link_delete(cmd);
   return ret;
}

/**
 * @brief  Leitura i2c BH1750
 * 
 * @param lux_msb most significant byte lux
 * @param lux_lsb less significant byte lux
 * 
 */
static esp_err_t i2c_read_bh1750(uint8_t *lux_msb, uint8_t *lux_lsb)
{
   int ret; 
   i2c_cmd_handle_t cmd = i2c_cmd_link_create();
   i2c_master_start(cmd);
   i2c_master_write_byte(cmd,  BH1750_ADDR << 1 | READ_BIT, ACK_CHECK_EN); 
   i2c_master_read_byte(cmd, lux_msb, ACK_VAL);
   i2c_master_read_byte(cmd, lux_lsb, NACK_VAL);
   i2c_master_stop(cmd);
   ret = i2c_master_cmd_begin(I2C_MASTER_PORT, cmd, 1000 / portTICK_RATE_MS);
   i2c_cmd_link_delete(cmd);
   return ret;
}

/**
 * @brief Inicialização do I2C
 * 
 * Escrever (Power Down) > Escrever (Start)
 * 
 */
void bh1750_start(void)
{
   i2c_write_bh1750(0x00);
   i2c_write_bh1750(0x01);
}

/**
 * @brief Leitura de iluminancia
 *
 * Escrever (Start) > Escrever(One Time HRes Mode) > Ler Iluminância
 * 
 * @param lux valor de iluminancia
 */
void bh1750_read(float *lux)
{
   uint8_t lux_msb, lux_lsb;
   i2c_write_bh1750(0b00000001);
   i2c_write_bh1750(0b00100000);
   vTaskDelay(120 / portTICK_RATE_MS);
   i2c_read_bh1750(&lux_msb, &lux_lsb);
   *lux = ((lux_msb << 8 | lux_lsb)/1.2);
}
