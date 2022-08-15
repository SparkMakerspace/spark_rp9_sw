/*********************************************************************
 See main.cpp for full header-text.

 MIT license, check LICENSE for more information
 Copyright (c) 2022 John Scimone
 All text above must be included in any redistribution
*********************************************************************/

#ifndef MAIN_H

#define MAIN_H

#include "SPI.h"
#include "SdFat.h"
#include "Adafruit_SPIFlash.h"
#include "Adafruit_TinyUSB.h"
#include <Adafruit_NeoPixel.h>
#include <array>

//--------------------------------------------------------------------+
// MSC External Flash Config
//--------------------------------------------------------------------+

// Un-comment to run example with custom SPI SPI and SS e.g with FRAM breakout
// #define CUSTOM_CS   A5
// #define CUSTOM_SPI  SPI

#if defined(CUSTOM_CS) && defined(CUSTOM_SPI)
Adafruit_FlashTransport_SPI flashTransport(CUSTOM_CS, CUSTOM_SPI);

#elif defined(ARDUINO_ARCH_ESP32)
// ESP32 use same flash device that store code.
// Therefore there is no need to specify the SPI and SS
Adafruit_FlashTransport_ESP32 flashTransport;

#elif defined(ARDUINO_ARCH_RP2040)
// RP2040 use same flash device that store code.
// Therefore there is no need to specify the SPI and SS
// Use default (no-args) constructor to be compatible with CircuitPython partition scheme
Adafruit_FlashTransport_RP2040 flashTransport;

// For generic usage:
//    Adafruit_FlashTransport_RP2040 flashTransport(start_address, size)
// If start_address and size are both 0, value that match filesystem setting in
// 'Tools->Flash Size' menu selection will be used

#else
// On-board external flash (QSPI or SPI) macros should already
// defined in your board variant if supported
// - EXTERNAL_FLASH_USE_QSPI
// - EXTERNAL_FLASH_USE_CS/EXTERNAL_FLASH_USE_SPI
#if defined(EXTERNAL_FLASH_USE_QSPI)
Adafruit_FlashTransport_QSPI flashTransport;

#elif defined(EXTERNAL_FLASH_USE_SPI)
Adafruit_FlashTransport_SPI flashTransport(EXTERNAL_FLASH_USE_CS, EXTERNAL_FLASH_USE_SPI);

#else
#error No QSPI/SPI flash are defined on your board variant.h !
#endif
#endif

Adafruit_SPIFlash flash(&flashTransport);

// file system object from SdFat
FatFileSystem fatfs;

FatFile root;
FatFile file;

// USB Mass Storage object
Adafruit_USBD_MSC usb_msc;

// Check if flash is formatted
bool fs_formatted;

// Set to true when PC write to flash
bool fs_changed;

//--------------------------------------------------------------------+
// HID Config
//--------------------------------------------------------------------+

// HID report descriptor using TinyUSB's template
// Single Report (no ID) descriptor
uint8_t const desc_hid_report[] =
    {
        TUD_HID_REPORT_DESC_KEYBOARD()};

// USB HID object. For ESP32 these values cannot be changed after this declaration
// desc report, desc len, protocol, interval, use out endpoint
Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_NONE, 2, false);

//--------------------------------------------------------------------+
// Prototypes
//--------------------------------------------------------------------+
int32_t msc_read_cb(uint32_t, void *, uint32_t);
int32_t msc_write_cb(uint32_t, uint8_t *, uint32_t);
void msc_flush_cb(void);
void handleKeypress(int);

//--------------------------------------------------------------------+
// Other Stuff
//--------------------------------------------------------------------+

#define COL1 D8
#define COL2 D9
#define COL3 D10
#define ROW1 D1
#define ROW2 D2
#define ROW3 D3
#define LED1 D0
#define LED2 D6
#define LED3 D7
// PIN_LED_R
// PIN_LED_G
// PIN_LED_B
// PIN_NEOPIXEL
// NEOPIXEL_POWER

std::array<uint8_t, 3> cols = {COL1, COL2, COL3};
std::array<uint8_t, 3> rows = {ROW1, ROW2, ROW3};
std::array<uint8_t, 3> leds = {LED1, LED2, LED3};
std::array<uint8_t, 3> rgbLeds = {PIN_LED_R, PIN_LED_G, PIN_LED_B};

class Keypage
{
public:
  int page;                       // page number
  std::array<int, 9> pagechange;  // which page each key changes to. 69 if don't change.
  std::array<uint8_t, 9> hidcode; // hidcode for each key on the page. 0 for no output

  Keypage(int page, std::array<int, 9> pagechange, std::array<uint8_t, 9> hidcode)
  {
    this->page = page;
    this->pagechange = pagechange;
    this->hidcode = hidcode;
  }
  Keypage() {}

  void fill(int page, std::array<int, 9> pagechange, std::array<uint8_t, 9> hidcode)
  {
    this->page = page;
    this->pagechange = pagechange;
    this->hidcode = hidcode;
  }
};

// TODO: these pages are temporary and should be deleted.
Keypage page0;
Keypage page1;

// stores the keypages parsed by core 0
std::array<Keypage *, 9> keypages;

// used to blink an LED
int builtinLED_BState;
int builtinLED_RState;

// stores the address of the keypages for core 1
std::array<Keypage *, 9> *core1Keypages;

// the current page number
int currpage;

// the list of keycodes to send - never more than 9!
std::array<uint8_t,6> keycode;
uint8_t count;

#endif
