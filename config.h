#ifndef CONFIG_H
#define CONFIG_H

#if defined(ARDUINO_BLUEPILL_F103C8)
#define BOARD_TYPE F("Bluepill STM32F103C8")
#define CS_PIN PA4
#define SCK_PIN PA5
#define MISO_PIN PA6
#define MOSI_PIN PA7
#define CE_PIN PB0
#define SCL_PIN PB6
#define SDA_PIN PB7
#elif defined(ARDUINO_AVR_NANO)
#define BOARD_TYPE F("Arduino Nano")
#define NODE_ID 1
#define CS_PIN 10
#define SCK_PIN 13
#define MISO_PIN 12
#define MOSI_PIN 11
#define CE_PIN 9
#define SCL_PIN A5
#define SDA_PIN A4
#else
#error Unsupported microprocessor type selected
#endif

#endif
