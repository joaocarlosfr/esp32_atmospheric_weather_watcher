#ifndef BME280_H
#define BME280_H

#include <stdint.h>

/**
 * @brief Inicialização do Sensor BME280
 */
void bme280_start();

/**
 * @brief Função para ler o sensor BME280.
 * 
 * @param temp ponteiro da variável de temperatura.
 * @param pabs ponteiro da variável de pressão.
 * @param umid ponteiro da variável de umidade.
 * @param alt ponteiro da variável de altitude.
 */
void bme280_read(float *temp, float *pabs, float *umid);

#endif