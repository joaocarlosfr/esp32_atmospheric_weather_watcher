# ESP32 Atmospheric Weather Watcher
![Espressif](https://img.shields.io/badge/espressif-E7352C.svg?style=for-the-badge&logo=espressif&logoColor=white)
![Home Assistant](https://img.shields.io/badge/home%20assistant-%2341BDF5.svg?style=for-the-badge&logo=home-assistant&logoColor=white)

## Introduction

Welcome aboard! My name is João, which is the Brazilian equivalent of "John". I'm a brazilian eletronic engineer graduated from **Federal University of Technology – Paraná**. In this repository i make available my final course project, it constructed a watcher of some characteristics of the atmosphere around a house. 

> [!NOTE]  
> 💡 Just for informational purposes regarding the motivations for building this, i've always wondered how devices are able to tell us what the weather is like. Nothing better than try to do this by myself, so i choose sensors such as the BME280 and BH1750FVI to capture key environmental variables, while Wi-Fi connectivity and the MQTT protocol ensured seamless integration with remote platforms. The result was an efficient and connected solution designed for IoT applications.

## The Document

If you understand portuguese, i invite you to read the project in the following link: [Desenvolvimento de Sistema para Análise do Tempo Atmosférico com o ESP32](https://repositorio.utfpr.edu.br/jspui/bitstream/1/36416/1/sistemaanalisetempoatmosferico.pdf).

## Technical Information

This project uses the **ESP-IDF** framework for embedded systems development with connectivity features.

The following components were integrated:

* 🌡️ **BME280**: sensor for temperature, humidity, and pressure
* ☀️ **BH1750FVI**: sensor for light measurement
* 🌧️ **YL-83**: analogic sensor for rain measuremennt
* 🌐 **Wi-Fi**: communication 
* 🗄️ **MQTT**: protocol for data transmission
* 🏠 **Home Assistant** and **Node-RED**: broker and data analyzes


## BME280 and BH1750

When i started this project i decided to implement things like the way the sensors comunicate, for educational purposes, the **BME280 library** was one of that things. This library uses I2C driver, and the magnificent formulas provided by bosch documentation to transform raw data into real information. The same for BH1750FVI sensor.

## WiFi and MQTT

I would like to thank **teacher Renato Sampaio (University of Brasília)**, i watched some of his classes on YouTube to write WiFi and MQTT drivers for this project. If you be interested to learn how i did that please [click on this link](https://www.youtube.com/watch?v=2toRLL_S6Yo) and enjoy his knowledge. He also teached how to use **Kconfig.projbuild** to abstract things like Keys, URIs, Passwords with ESP-IDF. 

## Home Assistant and Node-RED

I used this wonderfull solution called Home Assistant as MQTT broker and Node-RED plugin with some javescript code to analyse the information. The analyzes are based on NOAA (US), INMET (BR) and some technical documents that i found during this research, you can find all information in that portuguese documentation in "The Document" section.

## Conclusion

I would like to thank my advisor teacher Dr. Luís Fernando Caparroz Duarte, he showed the way and cleared a lot of doubts. I think to develop this project built a lot of knowledge for me and hopefully it can help new sailors to exploring this ocean called **Embedded Systems**.
