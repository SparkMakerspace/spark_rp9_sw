#ifndef MAIN_H

#define MAIN_H

#include "SPI.h"
#include "SdFat.h"
#include "Adafruit_SPIFlash.h"
#include "Adafruit_TinyUSB.h"
#include <Adafruit_NeoPixel.h>

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
  TUD_HID_REPORT_DESC_KEYBOARD()
};

// USB HID object. For ESP32 these values cannot be changed after this declaration
// desc report, desc len, protocol, interval, use out endpoint
Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_NONE, 2, false);

//--------------------------------------------------------------------+
// Prototypes
//--------------------------------------------------------------------+
int32_t msc_read_cb (uint32_t, void*, uint32_t);
int32_t msc_write_cb (uint32_t, uint8_t*, uint32_t);
void msc_flush_cb (void);

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
//PIN_LED_R
//PIN_LED_G
//PIN_LED_B
//PIN_NEOPIXEL
//NEOPIXEL_POWER

uint8_t cols[] = {COL1, COL2, COL3};
uint8_t rows[] = {ROW1, ROW2, ROW3};

struct keypage {
  uint8_t page; // page number
  uint8_t pagechange[9]; // which page each key changes to. 0 if don't change.
  uint8_t keycode[9]; // keycode for each key on the page
};

keypage defpage = {0,{0,0,0,0,0,0,0,0,0},{HID_KEY_1, HID_KEY_2, HID_KEY_3,
                                          HID_KEY_4, HID_KEY_5, HID_KEY_6,
                                          HID_KEY_7, HID_KEY_8, HID_KEY_9}};

#endif
