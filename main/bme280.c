/*
 * Copyright (c) 2022-present joaocarlosfr. 
 * 
 * SPDX-License-Identifier: MIT
 */

#include "bme280.h"

#include <stdint.h>
#include <math.h>

#include "esp_err.h"
#include "esp_system.h"
#include "driver/i2c.h"

#define BME280_ADDR 0x76
#define I2C_MASTER_PORT       0                  // Número do mestre
#define I2C_MASTER_FREQ_HZ    100000             // Frequência do Mestre 
#define WRITE_BIT             I2C_MASTER_WRITE   // Bit de escrita
#define READ_BIT              I2C_MASTER_READ    // Bit de leitura
#define ACK_CHECK_EN          0x1                // ACK Enable
#define ACK_CHECK_DIS         0x0                // ACK Disable
#define ACK_VAL               0x0                // ACK Check
#define NACK_VAL              0x1                // NACK Enable

int32_t t_fine;

/** 
 * @brief Instalação do Driver I2C
 *
 */
static esp_err_t i2c_master_init()
{
   i2c_config_t conf;
   conf.mode = I2C_MODE_MASTER;
   conf.sda_io_num = GPIO_NUM_18;
   conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
   conf.scl_io_num = GPIO_NUM_19;
   conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
   conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
   conf.clk_flags = 0;
   esp_err_t err = i2c_param_config(I2C_MASTER_PORT, &conf);
   if (err != ESP_OK)
   {             
      return err;
   }
   return i2c_driver_install(I2C_MASTER_PORT, conf.mode, 0, 0, 0);
}

/**
 * @brief Função de escrita I2C para o BME280
 * @param reg_adress endereço do registrador
 * @param data dados a serem registrados
 */
static esp_err_t i2c_write_bme280(uint8_t reg_adress, uint8_t data)
{
   int ret;
   i2c_cmd_handle_t cmd = i2c_cmd_link_create();                                 
   i2c_master_start(cmd);                                                       
   i2c_master_write_byte(cmd, BME280_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);   
   i2c_master_write_byte(cmd, reg_adress, ACK_CHECK_EN);
   i2c_master_write_byte(cmd, data, ACK_CHECK_EN);                          
   i2c_master_stop(cmd);                                                        
   ret = i2c_master_cmd_begin(I2C_MASTER_PORT, cmd, 1000 / portTICK_RATE_MS);            
   i2c_cmd_link_delete(cmd);                                                    
   return ret;
}

/**
 * @brief Função de compensação de temperatura
 * @param adc_T valor do registrador de temperatura
 * @param dig_T1 variável de compensação
 * @param dig_T2 variável de compensação
 * @param dig_T3 variável de compensação
 * Fonte: datasheet BME280, Bosch.
 */
static int temperatura(int32_t adc_T, uint16_t dig_T1, int16_t dig_T2, int16_t dig_T3)
{
   int32_t var1, var2, T;
   var1 = ((((adc_T>>3)-((int32_t)dig_T1<<1)))*((int32_t)dig_T2)) >> 11;
   var2 = (((((adc_T>>4)-((int32_t)dig_T1))*((adc_T>>4)-((int32_t)dig_T1)))>>12)*((int32_t) dig_T3))>>14;
   t_fine = var1 + var2;
   T = ((t_fine)*5+128)>>8;
   return T;
}

/**
 * @brief Função de compensação de pressão
 * @param adc_P valor do registrador de pressão
 * @param dig_P1 variável de compensação
 * @param dig_P2 variável de compensação
 * @param dig_P3 variável de compensação
 * @param dig_P4 variável de compensação
 * @param dig_P5 variável de compensação
 * @param dig_P6 variável de compensação
 * @param dig_P7 variável de compensação
 * @param dig_P8 variável de compensação
 * @param dig_P9 variável de compensação
 */
static int pressao(int32_t adc_P, uint16_t dig_P1, int16_t dig_P2, int16_t dig_P3, int16_t dig_P4, 
                   int16_t dig_P5, int16_t dig_P6, int16_t dig_P7, int16_t dig_P8, int16_t dig_P9)
{
   int32_t var1, var2;
   uint32_t p;
   var1 = (((int32_t)t_fine)>>1) - (int32_t)64000;
   var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((int32_t)dig_P6);
   var2 = var2 + ((var1*((int32_t)dig_P5))<<1);
   var2 = (var2>>2)+(((int32_t)dig_P4)<<16);
   var1 = (((dig_P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((int32_t)dig_P2) * var1)>>1))>>18;
   var1 =((((32768+var1))*((int32_t)dig_P1))>>15);
   if (var1 == 0) {
      return 0;
   }
   p = (((uint32_t)(((int32_t)1048576)-adc_P)-(var2>>12)))*3125;
   if (p < 0x80000000) {
      p = (p << 1) / ((uint32_t)var1);
   } else {
      p = (p / (uint32_t)var1) * 2;
   }
   var1 = (((int32_t)dig_P9) * ((int32_t)(((p>>3) * (p>>3))>>13)))>>12;
   var2 = (((int32_t)(p>>2)) * ((int32_t)dig_P8))>>13;
   p = (uint32_t)((int32_t)p + ((var1 + var2 + dig_P7) >> 4));
   return p;
}  

/**
 * @brief Função de compensação de umidade
 * 
 * @param adc_H valor do registrador de umidade
 * @param dig_H1 variável de compensação
 * @param dig_H2 variável de compensação
 * @param dig_H3 variável de compensação
 * @param dig_H4 variável de compensação
 * @param dig_H5 variável de compensação
 * @param dig_H6 variável de compensação
 * 
 * @return int valor da umidade
 * 
 * Fonte: datasheet BME280, Bosch.
 */
static int umidade(int32_t adc_H, uint16_t dig_H1, int16_t dig_H2, uint16_t dig_H3, int16_t dig_H4, int16_t dig_H5, int16_t dig_H6)
{
   int32_t v_x1_u32r;
   v_x1_u32r = (t_fine - ((int32_t)76800));
   v_x1_u32r = (((((adc_H << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)dig_H6)) >> 10) * (((v_x1_u32r * ((int32_t)dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)dig_H2) + 8192) >> 14));
   v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)dig_H1)) >> 4));
   v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
   v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
   return (int32_t)(v_x1_u32r >> 12);
}

/**
 * @brief Leitura dos valores dos registradores de pressão, temperatura e umidade.
 *
 */
static esp_err_t i2c_read_raw_bme280(uint8_t *press_msb, uint8_t *press_lsb, uint8_t *press_xlsb, 
                                     uint8_t *temp_msb, uint8_t *temp_lsb, uint8_t *temp_xlsb, 
                                     uint8_t *hum_msb, uint8_t *hum_lsb)
{
   int ret; 
   i2c_cmd_handle_t cmd = i2c_cmd_link_create();
   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, BME280_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);       
   i2c_master_write_byte(cmd, 0xF7, ACK_CHECK_EN);                      
   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, BME280_ADDR << 1 | READ_BIT, ACK_CHECK_EN);
   i2c_master_read_byte(cmd, press_msb, ACK_VAL);
   i2c_master_read_byte(cmd, press_lsb, ACK_VAL);                        
   i2c_master_read_byte(cmd, press_xlsb, ACK_VAL); 
   i2c_master_read_byte(cmd, temp_msb, ACK_VAL);
   i2c_master_read_byte(cmd, temp_lsb, ACK_VAL);                        
   i2c_master_read_byte(cmd, temp_xlsb, ACK_VAL);
   i2c_master_read_byte(cmd, hum_msb, ACK_VAL);
   i2c_master_read_byte(cmd, hum_lsb, NACK_VAL);                                                   
   i2c_master_stop(cmd);
   ret = i2c_master_cmd_begin(I2C_MASTER_PORT, cmd, 1000 / portTICK_RATE_MS);
   i2c_cmd_link_delete(cmd);
   return ret;
}

/**
 *  @brief Leitura do primeiro grupo de valores de calibração do sensor.
 *
 */
static esp_err_t i2c_read_calibration(uint8_t *dig_T1_1, uint8_t *dig_T1_2, uint8_t *dig_T2_1, uint8_t *dig_T2_2, uint8_t *dig_T3_1, uint8_t *dig_T3_2, 
                                      uint8_t *dig_P1_1, uint8_t *dig_P1_2, uint8_t *dig_P2_1, uint8_t *dig_P2_2, uint8_t *dig_P3_1, uint8_t *dig_P3_2, 
                                      uint8_t *dig_P4_1, uint8_t *dig_P4_2, uint8_t *dig_P5_1, uint8_t *dig_P5_2, uint8_t *dig_P6_1, uint8_t *dig_P6_2, 
                                      uint8_t *dig_P7_1, uint8_t *dig_P7_2, uint8_t *dig_P8_1, uint8_t *dig_P8_2, uint8_t *dig_P9_1, uint8_t *dig_P9_2, 
                                      uint8_t *dig_H1_1)
{
   int ret;
   i2c_cmd_handle_t cmd = i2c_cmd_link_create();
   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, BME280_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);       
   i2c_master_write_byte(cmd, 0x88, ACK_CHECK_EN);                      
   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, BME280_ADDR << 1 | READ_BIT, ACK_CHECK_EN);
   i2c_master_read_byte(cmd, dig_T1_1, ACK_VAL);
   i2c_master_read_byte(cmd, dig_T1_2, ACK_VAL);
   i2c_master_read_byte(cmd, dig_T2_1, ACK_VAL);
   i2c_master_read_byte(cmd, dig_T2_2, ACK_VAL);
   i2c_master_read_byte(cmd, dig_T3_1, ACK_VAL);
   i2c_master_read_byte(cmd, dig_T3_2, ACK_VAL);
   i2c_master_read_byte(cmd, dig_P1_1, ACK_VAL);
   i2c_master_read_byte(cmd, dig_P1_2, ACK_VAL);
   i2c_master_read_byte(cmd, dig_P2_1, ACK_VAL);
   i2c_master_read_byte(cmd, dig_P2_2, ACK_VAL);
   i2c_master_read_byte(cmd, dig_P3_1, ACK_VAL);
   i2c_master_read_byte(cmd, dig_P3_2, ACK_VAL);
   i2c_master_read_byte(cmd, dig_P4_1, ACK_VAL);
   i2c_master_read_byte(cmd, dig_P4_2, ACK_VAL);
   i2c_master_read_byte(cmd, dig_P5_1, ACK_VAL);
   i2c_master_read_byte(cmd, dig_P5_2, ACK_VAL);
   i2c_master_read_byte(cmd, dig_P6_1, ACK_VAL);
   i2c_master_read_byte(cmd, dig_P6_2, ACK_VAL);
   i2c_master_read_byte(cmd, dig_P7_1, ACK_VAL);
   i2c_master_read_byte(cmd, dig_P7_2, ACK_VAL);
   i2c_master_read_byte(cmd, dig_P8_1, ACK_VAL);
   i2c_master_read_byte(cmd, dig_P8_2, ACK_VAL);
   i2c_master_read_byte(cmd, dig_P9_1, ACK_VAL);
   i2c_master_read_byte(cmd, dig_P9_2, ACK_VAL);
   i2c_master_read_byte(cmd, dig_H1_1, ACK_VAL);
   i2c_master_read_byte(cmd, dig_H1_1, NACK_VAL);
   i2c_master_stop(cmd);
   ret = i2c_master_cmd_begin(I2C_MASTER_PORT, cmd, 1000 / portTICK_RATE_MS);
   i2c_cmd_link_delete(cmd);
   return ret;
}

/**
 *  @brief Leitura do segundo grupo de valores de calibração do sensor.
 *
 */
static esp_err_t i2c_read_calibration_2(uint8_t *dig_H2_1, uint8_t *dig_H2_2, uint8_t *dig_H3_1, uint8_t *dig_H4_1, uint8_t *dig_H4_2, uint8_t *dig_H5_2, uint8_t *dig_H6_1)
{
   int ret;
   i2c_cmd_handle_t cmd = i2c_cmd_link_create();
   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, BME280_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);       
   i2c_master_write_byte(cmd, 0xE1, ACK_CHECK_EN);                      
   i2c_master_start(cmd);
   i2c_master_write_byte(cmd, BME280_ADDR << 1 | READ_BIT, ACK_CHECK_EN);
   i2c_master_read_byte(cmd, dig_H2_1, ACK_VAL);
   i2c_master_read_byte(cmd, dig_H2_2, ACK_VAL);
   i2c_master_read_byte(cmd, dig_H3_1, ACK_VAL);
   i2c_master_read_byte(cmd, dig_H4_1, ACK_VAL);
   i2c_master_read_byte(cmd, dig_H4_2, ACK_VAL);
   i2c_master_read_byte(cmd, dig_H5_2, ACK_VAL);
   i2c_master_read_byte(cmd, dig_H6_1, NACK_VAL);
   i2c_master_stop(cmd);
   ret = i2c_master_cmd_begin(I2C_MASTER_PORT, cmd, 1000 / portTICK_RATE_MS);
   i2c_cmd_link_delete(cmd);
   return ret;
}

/**
 * @brief Função que faz a leitura e compensação dos registradores
 *
 * Etapas:
 *  _______________________      ______________________________________      ___________________________      ____________________________________
 * | Leitura Registradores | -> | Leitura dos parametros de calibração | -> | Soma de bits Registadores | -> | Funções de compesação, valor final |
 *  ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯      ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯      ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯      ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
 * @param temp temperatura compensada
 * @param pabs pressão compensada
 * @param umid umidade compensada
 *
 */
static void bme280_out(int32_t *temp, int32_t *pabs, int32_t *umid)
{
   uint8_t press_msb, press_lsb, press_xlsb, temp_msb, temp_lsb, temp_xlsb, hum_msb, hum_lsb;
   uint8_t dig_T1_1, dig_T1_2, dig_T2_1, dig_T2_2, dig_T3_1, dig_T3_2, 
           dig_P1_1, dig_P1_2, dig_P2_1, dig_P2_2, dig_P3_1, dig_P3_2, 
           dig_P4_1, dig_P4_2, dig_P5_1, dig_P5_2, dig_P6_1, dig_P6_2, 
           dig_P7_1, dig_P7_2, dig_P8_1, dig_P8_2, dig_P9_1, dig_P9_2, 
           dig_H1_1, dig_H2_1, dig_H2_2, dig_H3_1, dig_H4_1, dig_H4_2, 
           dig_H5_2, dig_H6_1;
   uint16_t dig_T1, dig_P1, dig_H1, dig_H3;
   int16_t dig_T2, dig_T3, dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, 
   dig_P8, dig_P9, dig_H2, dig_H4, dig_H5, dig_H6;
   int32_t adc_T, adc_P, adc_H;

   i2c_read_raw_bme280(&press_msb, &press_lsb, &press_xlsb, &temp_msb, &temp_lsb, &temp_xlsb, &hum_msb, &hum_lsb);

   i2c_read_calibration(&dig_T1_1, &dig_T1_2, &dig_T2_1, &dig_T2_2, &dig_T3_1, &dig_T3_2, 
                        &dig_P1_1, &dig_P1_2, &dig_P2_1, &dig_P2_2, &dig_P3_1, &dig_P3_2, 
                        &dig_P4_1, &dig_P4_2, &dig_P5_1, &dig_P5_2, &dig_P6_1, &dig_P6_2, 
                        &dig_P7_1, &dig_P7_2, &dig_P8_1, &dig_P8_2, &dig_P9_1, &dig_P9_2, 
                        &dig_H1_1);

   i2c_read_calibration_2(&dig_H2_1, &dig_H2_2, &dig_H3_1, &dig_H4_1, &dig_H4_2, &dig_H5_2, &dig_H6_1);

   dig_T1 = dig_T1_2 << 8 | dig_T1_1;
   dig_T2 = dig_T2_2 << 8 | dig_T2_1;
   dig_T3 = dig_T3_2 << 8 | dig_T3_1;
   dig_P1 = dig_P1_2 << 8 | dig_P1_1;
   dig_P2 = dig_P2_2 << 8 | dig_P2_1;
   dig_P3 = dig_P3_2 << 8 | dig_P3_1;
   dig_P4 = dig_P4_2 << 8 | dig_P4_1;
   dig_P5 = dig_P5_2 << 8 | dig_P5_1;
   dig_P6 = dig_P6_2 << 8 | dig_P6_1;
   dig_P7 = dig_P7_2 << 8 | dig_P7_1;
   dig_P8 = dig_P8_2 << 8 | dig_P8_1;
   dig_P9 = dig_P9_2 << 8 | dig_P9_1;
   dig_H1 = dig_H1_1;
   dig_H2 = dig_H2_2 << 8 | dig_H2_1;
   dig_H3 = dig_H3_1;
   dig_H4 = dig_H4_1 << 4 | ((dig_H4_2) ^ (((dig_H4_2 >> 4) << 4)));
   dig_H5 = dig_H5_2 << 4 | (dig_H4_2 >> 4);
   dig_H6 = dig_H6_1;

   adc_T = (temp_xlsb >> 4) | (temp_lsb << 4) | (temp_msb << 12);
   *temp = temperatura(adc_T, dig_T1, dig_T2, dig_T3);
   adc_P = (press_xlsb >> 4) | (press_lsb << 4) | (press_msb << 12);
   *pabs = pressao(adc_P, dig_P1, dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9);
   adc_H =  hum_lsb | (hum_msb << 8);
   *umid = umidade(adc_H, dig_H1, dig_H2, dig_H3, dig_H4, dig_H5, dig_H6);
}

/** 
 * @brief Inicialização do BME280
 * 0xF2 -> ______.__ 
 *         7,6,5,4,3,2: Nada.
 *         1,0: Controles de oversampling dos dados de umidade.
 *
 * 0xF4 -> ___.___.__
 *         7,6,5: Controles de oversampling dos dados de temperatura.
 *         4,3,2: Controles de oversampling dos dados de pressão.
 *         1,0:   Definição de modo do sensor.
 *
 * 0xF5 -> ___.___._._ 
 *         7,6,5: Controles de standby.
 *         4,3,2: Filtro IIR.
 *         1: Nada.
 *         0: SPI Enable.
 */
void bme280_start()
{
   i2c_master_init();
   i2c_write_bme280(0xF2, 0b00000001);
   i2c_write_bme280(0xF4, 0b00100110);
   i2c_write_bme280(0xF5, 0b10100000);
}

/**
 * @brief Leitura do Sensor BME280
 *
 * @param temp temperatura medida
 * @param pabs pressão absoluta medida
 * @param umid umidade medida
 */
void bme280_read(float *temp, float *pabs, float *umid)
{
   int32_t t, p, u;
   i2c_write_bme280(0xF4, 0b00100110);
   bme280_out(&t, &p, &u);
   *temp = (float)t/100;
   *pabs = (float)p/100;
   *umid = (float)u/1024;
}