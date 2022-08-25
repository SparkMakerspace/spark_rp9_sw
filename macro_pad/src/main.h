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
#include "keymapping.h"
#include "ArduinoJson.h"
#include <Adafruit_NeoPixel.h>
#include <array>
#include <regex>

//--------------------------------------------------------------------+
// MSC External Flash Config
//--------------------------------------------------------------------+

#if defined(ARDUINO_ARCH_RP2040)
// RP2040 use same flash device that store code.
// Therefore there is no need to specify the SPI and SS
// Use default (no-args) constructor to be compatible with CircuitPython partition scheme
Adafruit_FlashTransport_RP2040 flashTransport;

// For generic usage:
//    Adafruit_FlashTransport_RP2040 flashTransport(start_address, size)
// If start_address and size are both 0, value that match filesystem setting in
// 'Tools->Flash Size' menu selection will be used
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
void removeSpace(char *);
void handleKeypress(int);
bool parseConfig(FatFile);
void blinkGreen(int);
void blinkRed(int);

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

//--------------------------------------------------------------------+
// Keypage Class
//--------------------------------------------------------------------+
class Keypage
{
public:
  int page;                        // page number
  std::array<int, 9> pagechange;   // which page each key changes to. 69 if don't change.
  std::array<uint8_t, 9> hidcode;  // hidcode for each key on the page. 0 for no output
  std::array<uint8_t, 9> modcode;  // modifier for each key on the page
  std::array<bool, 3> leds;        // board LEDs
  std::array<bool, 3> builtinleds; // builtin RGB LEDs
  uint32_t neopixel;               // Neopixel value

  Keypage(int page,
          std::array<int, 9> pagechange,
          std::array<uint8_t, 9> hidcode,
          std::array<uint8_t, 9> modcode,
          std::array<bool, 3> leds,
          std::array<bool, 3> builtinleds,
          uint32_t neopixel)
  {
    this->page = page;
    this->pagechange = pagechange;
    this->hidcode = hidcode;
    this->modcode = modcode;
    this->leds = leds;
    this->builtinleds = builtinleds;
    this->neopixel = neopixel;
  }
  Keypage() {
    this->page = -1;
    this->pagechange = {69};
    this->hidcode = {0};
    this->modcode = {0};
    this->leds = {false};
    this->builtinleds = {false};
    this->neopixel = 0;
  }

  void fill(int page,
            std::array<int, 9> pagechange,
            std::array<uint8_t, 9> hidcode,
            std::array<uint8_t, 9> modcode,
            std::array<bool, 3> leds,
            std::array<bool, 3> builtinleds,
            uint32_t neopixel)
  {
    this->page = page;
    this->pagechange = pagechange;
    this->hidcode = hidcode;
    this->leds = leds;
    this->modcode = modcode;
    this->builtinleds = builtinleds;
    this->neopixel = neopixel;
  }

  void print()
  {
    Serial.print("Page: ");
    Serial.println(this->page);
    Serial.println("Pagechange: ");
    for (int i = 0; i < 9; i++){
      Serial.println(this->pagechange[i]);
    }
        Serial.println("hidcode: ");
    for (int i = 0; i < 9; i++){
      Serial.println(this->hidcode[i]);
    }
        Serial.println("leds: ");
    for (int i = 0; i < 3; i++){
      Serial.println(this->leds[i]);
    }
        Serial.println("modcode: ");
    for (int i = 0; i < 9; i++){
      Serial.println(this->modcode[i]);
    }
        Serial.println("builtinleds: ");
    for (int i = 0; i < 3; i++){
      Serial.println(this->builtinleds[i]);
    }
    Serial.print("Neopixel: ");
    Serial.println(this->neopixel);
  }
};

// tracks if a good config is loaded
bool invalidConfig;

// stores the keypages parsed by core 0
std::array<Keypage *, 9> keypages;

Adafruit_NeoPixel np(1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

// the current page number
int currpage;
// the last page number
bool pagechanged;

// the list of keycodes to send - never more than 6 per the spec!
std::array<uint8_t, 6> keycode;
// how many keys were pressed
uint8_t count;
// modifier key collector
uint8_t modifier;

// whether we've allocated memory for keypages
bool memallocated;

#endif
