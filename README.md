# ESP32 Atmospheric Weather Watcher

This repository contain ESP32 embedded code (ESP-IDF) for my final course project (Eletronic Engineering - Federal University of Technology – Paraná), this project constructed a watcher of some characteristics of the atmosphere around a house. The code contain relevant stuff like BME280, BH1750FVI, Wi-Fi, MQTT lib for ESP-IDF. (You can read the project in the following link: https://repositorio.utfpr.edu.br/jspui/bitstream/1/36416/1/sistemaanalisetempoatmosferico.pdf)  
  
The project was developed using ESP-IDF, you can create a project and after that import the code inside main folder, hope you enjoy.

### Project Tree Structure

esp32_atmospheric_weather_watcher/  
├── .gitignore  
├── main/  
│   ├── bh1750.c  
│   ├── bh1750.h  
│   ├── bme280.c  
│   ├── bme280.h  
│   ├── Kconfig.projbuild  
│   ├── main.c  
│   ├── mqtt.c  
│   ├── mqtt.h  
│   ├── rainsensor.c  
│   ├── rainsensor.h  
│   ├── wifi.c  
│   └── wifi.h  
├── LICENSE  
└── README.md  
  
Kconfig.projbuild: nice stuff to configure keys, uris, passwords in your ESP-IDF project, then you just put the information in a KCONFIG menu;  
bh1750: library to read luminosity sensor usign a ADC properly configured with ESP-IDF;  
bme280: library that i wrote using i2c driver of ESP-IDF to read BME280 sensor (pressure, temperature, humidity);  
mqtt: library to comunicate with a MQTT BROKER and send messages, using MQTT driver of ESP-IDF;  
rainsensor: library to read rain sensor using a ADC properly configured with ESP-IDF;  
wifi: library wrote using WiFi driver of ESP-IDF based in Professor Renato Sampaio (UNB) class, to connect ESP32 to a wifi access point. (https://www.youtube.com/watch?v=2toRLL_S6Yo)  