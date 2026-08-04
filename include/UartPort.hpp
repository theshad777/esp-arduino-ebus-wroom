#pragma once

#include <cstddef>
#include <cstdint>

#include <driver/uart.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

class UartPort {
 public:
  explicit UartPort(uart_port_t port);

  void begin(int baud, int rxPin = -1, int txPin = -1);
  void begin(int baud, uart_word_length_t dataBits, int rxPin, int txPin);
  void end();

  int available();
  int availableForWrite();
  int read();
  int peek();
  size_t write(uint8_t byte);

  void setRxBufferSize(size_t size);
  void setRxFIFOFull(int fullThreshold);
  void setDebugOutput(bool enable);
  void pollEvents();
  uint32_t framingErrors() const;
  uint32_t parityErrors() const;
  uint32_t fifoOverflows() const;
  uint32_t bufferFullErrors() const;

 private:
  void ensureInstalled(int baud, int rxPin, int txPin);

  uart_port_t port_;
  bool installed_ = false;
  size_t rxBufferSize_ = 1024;
  int cachedByte_ = -1;
  QueueHandle_t eventQueue_ = nullptr;

  uint32_t framingErrors_ = 0;
  uint32_t parityErrors_ = 0;
  uint32_t fifoOverflows_ = 0;
  uint32_t bufferFullErrors_ = 0;
};

extern UartPort BusSer;
extern UartPort DebugSer;
