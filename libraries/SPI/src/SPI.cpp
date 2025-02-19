/*
 * Copyright (c) 2010 by Cristian Maglie <c.maglie@arduino.cc>
 * Copyright (c) 2014 by Paul Stoffregen <paul@pjrc.com> (Transaction API)
 * SPI Master library for arduino.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#include "SPI.h"

SPIClass SPI;

/**
  * @brief  Default constructor. Uses pin configuration of variant.h.
  */
SPIClass::SPIClass()
{
  _spi.pin_miso = digitalPinToPinName(MISO);
  _spi.pin_mosi = digitalPinToPinName(MOSI);
  _spi.pin_sclk = digitalPinToPinName(SCK);
  _spi.pin_ssel = NC;
}

/**
  * @brief  Constructor to create another SPI instance attached to another SPI
  *         peripheral different of the default SPI. All pins must be attached to
  *         the same SPI peripheral. See datasheet of the microcontroller.
  * @param  mosi: SPI mosi pin. Accepted format: number or Arduino format (Dx)
  *         or ST format (Pxy).
  * @param  miso: SPI miso pin. Accepted format: number or Arduino format (Dx)
  *         or ST format (Pxy).
  * @param  sclk: SPI clock pin. Accepted format: number or Arduino format (Dx)
  *         or ST format (Pxy).
  * @param  ssel: SPI ssel pin (optional). Accepted format: number or
  *         Arduino format (Dx) or ST format (Pxy). By default is set to NC.
  *         This pin must correspond to a hardware CS pin which can be managed
  *         by the SPI peripheral itself. See the datasheet of the microcontroller
  *         or look at PinMap_SPI_SSEL[] inside the file PeripheralPins.c
  *         corresponding to the board. If you configure this pin you can't use
  *         another CS pin and don't pass a CS pin as parameter to any functions
  *         of the class.
  */
SPIClass::SPIClass(uint32_t mosi, uint32_t miso, uint32_t sclk, uint32_t ssel)
{
  _spi.pin_miso = digitalPinToPinName(miso);
  _spi.pin_mosi = digitalPinToPinName(mosi);
  _spi.pin_sclk = digitalPinToPinName(sclk);
  _spi.pin_ssel = digitalPinToPinName(ssel);
}

/**
  * @brief  Initialize the SPI instance.
  */
void SPIClass::begin(void)
{
  _spi.handle.State = HAL_SPI_STATE_RESET;
  _spiSettings = DEFAULT_SPI_SETTINGS;
  spi_init(&_spi, _spiSettings.getClockFreq(),
           _spiSettings.getDataMode(),
           _spiSettings.getBitOrder());
}

/**
  * @brief  This function should be used to configure the SPI instance in case you
  *         don't use the default parameters set by the begin() function.
  * @param  settings: SPI settings(clock speed, bit order, data mode).
  */
void SPIClass::beginTransaction(SPISettings settings)
{
  if (_spiSettings != settings) {
    _spiSettings = settings;
    spi_init(&_spi, _spiSettings.getClockFreq(),
             _spiSettings.getDataMode(),
             _spiSettings.getBitOrder());
  }
}

/**
  * @brief  End the transaction after beginTransaction usage
  */
void SPIClass::endTransaction(void)
{

}

/**
  * @brief  Deinitialize the SPI instance and stop it.
  */
void SPIClass::end()
{
  spi_deinit(&_spi);
}

/**
  * @brief  Deprecated function.
  *         Configure the bit order: MSB first or LSB first.
  * @param  _bitOrder: MSBFIRST or LSBFIRST
  */
void SPIClass::setBitOrder(BitOrder bitOrder)
{
  _spiSettings.setBitOrder(bitOrder);

  spi_init(&_spi, _spiSettings.getClockFreq(),
           _spiSettings.getDataMode(),
           _spiSettings.getBitOrder());
}

/**
  * @brief  Deprecated function.
  *         Configure the data mode (clock polarity and clock phase)
  * @param  mode: SPI_MODE0, SPI_MODE1, SPI_MODE2 or SPI_MODE3
  * @note
  *         Mode          Clock Polarity (CPOL)   Clock Phase (CPHA)
  *         SPI_MODE0             0                     0
  *         SPI_MODE1             0                     1
  *         SPI_MODE2             1                     0
  *         SPI_MODE3             1                     1
  */

void SPIClass::setDataMode(uint8_t mode)
{
  setDataMode((SPIMode)mode);
}

void SPIClass::setDataMode(SPIMode mode)
{
  _spiSettings.setDataMode(mode);

  spi_init(&_spi, _spiSettings.getClockFreq(),
           _spiSettings.getDataMode(),
           _spiSettings.getBitOrder());
}

/**
  * @brief  Deprecated function.
  *         Configure the clock speed
  * @param  _divider: the SPI clock can be divided by values from 1 to 255.
  *         If 0, default SPI speed is used.
  */
void SPIClass::setClockDivider(uint8_t _divider)
{
  if (_divider == 0) {
    _spiSettings.setClockFreq(SPI_SPEED_CLOCK_DEFAULT);
  } else {
    /* Get clock freq of the SPI instance and compute it */
    _spiSettings.setClockFreq(spi_getClkFreq(&_spi) / _divider);
  }

  spi_init(&_spi, _spiSettings.getClockFreq(),
           _spiSettings.getDataMode(),
           _spiSettings.getBitOrder());
}

/**
  * @brief  Transfer one byte on the SPI bus.
  *         begin() or beginTransaction() must be called at least once before.
  * @param  data: byte to send.
  * @return byte received from the slave.
  */
uint8_t SPIClass::transfer(uint8_t data)
{
  spi_transfer(&_spi, &data, sizeof(uint8_t), SPI_TRANSFER_TIMEOUT, _spiSettings.getSkipRecv());
  return data;
}

/**
  * @brief  Transfer two bytes on the SPI bus in 16 bits format.
  *         begin() or beginTransaction() must be called at least once before.
  * @param  data: bytes to send.
  * @return bytes received from the slave in 16 bits format.
  */
uint16_t SPIClass::transfer16(uint16_t data)
{
  uint16_t tmp;

  if (_spiSettings.getBitOrder()) {
    tmp = ((data & 0xff00) >> 8) | ((data & 0xff) << 8);
    data = tmp;
  }
  spi_transfer(&_spi, (uint8_t *)&data, sizeof(uint16_t),
               SPI_TRANSFER_TIMEOUT, _spiSettings.getSkipRecv());

  if (_spiSettings.getBitOrder()) {
    tmp = ((data & 0xff00) >> 8) | ((data & 0xff) << 8);
    data = tmp;
  }

  return data;
}

/**
  * @brief  Transfer several bytes. Only one buffer used to send and receive data.
  *         begin() or beginTransaction() must be called at least once before.
  * @param  buf: pointer to the bytes to send. The bytes received are copy in
  *         this buffer.
  * @param  count: number of bytes to send/receive.
  */
void SPIClass::transfer(void *buf, size_t count)
{
  if ((count != 0) && (buf != NULL)) {
    spi_transfer(&_spi, ((uint8_t *)buf), count,
                 SPI_TRANSFER_TIMEOUT, _spiSettings.getSkipRecv());
  }
}

/**
  * @brief  Not implemented.
  */
void SPIClass::usingInterrupt(int interruptNumber)
{
  UNUSED(interruptNumber);
}

/**
  * @brief  Not implemented.
  */
void SPIClass::notUsingInterrupt(int interruptNumber)
{
  UNUSED(interruptNumber);
}

/**
  * @brief  Not implemented.
  */
void SPIClass::attachInterrupt(void)
{
  // Should be enableInterrupt()
}

/**
  * @brief  Not implemented.
  */
void SPIClass::detachInterrupt(void)
{
  // Should be disableInterrupt()
}

#if defined(SUBGHZSPI_BASE)
void SUBGHZSPIClass::begin()
{
  SPIClass::begin();
}

void SUBGHZSPIClass::beginTransaction(SPISettings settings)
{
  SPIClass::beginTransaction(settings);
}

byte SUBGHZSPIClass::transfer(uint8_t _data)
{
  byte res;
  res = SPIClass::transfer(_data);
  return res;
}

uint16_t SUBGHZSPIClass::transfer16(uint16_t _data)
{
  uint16_t rx_buffer = 0;
  rx_buffer = SPIClass::transfer16(_data);
  return rx_buffer;
}

void SUBGHZSPIClass::transfer(void *_buf, size_t _count)
{
  SPIClass::transfer(_buf, _count);
}

void SUBGHZSPIClass::enableDebugPins(uint32_t mosi, uint32_t miso, uint32_t sclk, uint32_t ssel)
{
  /* Configure SPI GPIO pins */
  pinmap_pinout(digitalPinToPinName(mosi), PinMap_SPI_MOSI);
  pinmap_pinout(digitalPinToPinName(miso), PinMap_SPI_MISO);
  pinmap_pinout(digitalPinToPinName(sclk), PinMap_SPI_SCLK);
  pinmap_pinout(digitalPinToPinName(ssel), PinMap_SPI_SSEL);
}
#endif
