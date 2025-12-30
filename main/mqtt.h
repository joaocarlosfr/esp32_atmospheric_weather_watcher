#ifndef MQTT_H
#define MQTT_H
/**
 * @brief Configura MQTT e inicia comunicação.
 * 
 */
void mqtt_start();

/**
 * @brief Faz o envio de uma mensagem via MQTT
 * 
 * @param topico String que descreve o topico que sera enviado
 * @param mensagem String do valor que será enviada ao broker
 */
void mqtt_envia_mensagem(char *topico, char *mensagem);

#endif