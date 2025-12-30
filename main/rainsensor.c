/*
 * Copyright (c) 2022-present joaocarlosfr. 
 * 
 * SPDX-License-Identifier: MIT
 */

#include "rainsensor.h"

#include <stdint.h>

#include "esp_adc_cal.h"
#include "driver/gpio.h"
#include "driver/adc.h"

#define VREF 3200  // Tensão de referência em Volts    
#define SAMPLES 64 // Amostras 

static const adc_atten_t atten = ADC_ATTEN_DB_11;       // Atenuação ideal para medições até 3.9V .
static const adc_unit_t unit = ADC_UNIT_1;              // SAR(Successive Approximation Register) ADC1.
static const adc_bits_width_t width = ADC_WIDTH_BIT_12; // Tamanho da leitura.
static const adc_channel_t channel = ADC1_CHANNEL_0;    // Canal da leitura.
static esp_adc_cal_characteristics_t *adc_chars;        // Criar estrutura de caracteristicas em adc_chars.

/**
 * @brief Inicializador do conversor ADC para aquisição de informações de chuva.
 * 
 */
void rainsensor_start()
{
    adc1_config_width(width); adc1_config_channel_atten(channel,atten);
    // Caracterização para a conversão RAW - mV
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t)); // Cria um vetor de tamanho dinâmico com ponteiro.
    esp_adc_cal_characterize(unit, atten, width, VREF, adc_chars); 
}

/**
 * @brief Rotina de leitura do sensor de chuva.
 * 
 * @param analograin variável que transporta a informação de chuva.
 */
void rainsensor_read(float *analograin)
{
    uint32_t reading = 0, voltage;
    
    // Faz aquisição de amostras
    for (int i = 0; i < SAMPLES; i++){
        reading += adc1_get_raw(ADC1_CHANNEL_0);
    }

    // Divide a leitura pelo numero de amostras
    reading /= SAMPLES;
    // Conversão RAW -> mV
    voltage = esp_adc_cal_raw_to_voltage(reading, adc_chars);
    // Regra de três para medir a quantidade de chuva em 10bits
    *analograin = ((voltage)*1023)/3250;
}