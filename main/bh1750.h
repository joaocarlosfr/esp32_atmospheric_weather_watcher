#ifndef BH1750_H
#define BH1750_H

/**
 * @brief Inicia o sensor BH1750
 */
void bh1750_start();

/**
 * @brief Leitura do sensor BH1750
 *
 * @param lux valor de luminância
 */
void bh1750_read(float *lux);

#endif