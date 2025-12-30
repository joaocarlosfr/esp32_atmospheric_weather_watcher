#ifndef WIFI_H
#define WIFI_H

/**
*  @brief Iniciar Wi-Fi: inicia o driver de wi-fi do esp.
*  Créditos: Professor Renato Sampaio (UNB) + Documentação Espressif ESP32
*  Documentação disponível em: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/index.html
*  Para configurar seu Wi-Fi por favor entre em menuconfig da sua IDF e insira ssid e senha.
*/
void wifi_start();

#endif