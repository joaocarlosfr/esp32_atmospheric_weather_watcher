#ifndef RAINSENSOR_H
#define RAINSENSOR_H

#include <stdint.h>
/**
 * @brief Pré configurações ADC.
 * 
 */
void rainsensor_start(void);

/**
 * @brief Leitura analógica do rain module.
 * 
 * @param analograin variável que transporta a informação de chuva.
 */
void rainsensor_read(float *analograin);

#endif