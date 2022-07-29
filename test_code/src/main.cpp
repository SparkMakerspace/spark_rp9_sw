#include <Arduino.h>
#include <xiaorp2040.h>
#include <Adafruit_TinyUSB.h>
#include <Adafruit_NeoPixel.h>

void hid_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize);

//------------- Neopixel -------------//
#ifdef PIN_NEOPIXEL
// How many NeoPixels are attached to the Arduino?
// use on-board defined NEOPIXEL_NUM if existed
#ifndef NEOPIXEL_NUM
#define NEOPIXEL_NUM 1
#endif
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NEOPIXEL_NUM, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
#endif

uint8_t const desc_hid_report[] =
    {
        TUD_HID_REPORT_DESC_KEYBOARD()};

uint8_t cols[] = {D8, D9, D10};
uint8_t rows[] = {D1, D2, D3};

// Key definitions!
uint8_t hidcode[] = {HID_KEY_7, HID_KEY_8, HID_KEY_9,
                     HID_KEY_4, HID_KEY_5, HID_KEY_6,
                     HID_KEY_1, HID_KEY_2, HID_KEY_3};

// Init Keyboard
Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_KEYBOARD, 2, false);

bool activeState = false;

uint8_t led_blink_divider;
uint8_t led_index = 0;

uint8_t builtin_leds[] = {LED_BUILTIN_R, LED_BUILTIN_G, LED_BUILTIN_B};

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                SETUP                                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void setup()
{
  led_blink_divider = 0;
#if defined(PIN_NEOPIXEL_POWER)
  pinMode(PIN_NEOPIXEL_POWER, OUTPUT);
  digitalWrite(PIN_NEOPIXEL_POWER, 1);
#endif
#if defined(ARDUINO_ARCH_MBED) && defined(ARDUINO_ARCH_RP2040)
  // Manual begin() is required on core without built-in support for TinyUSB such as mbed rp2040
  TinyUSB_Device_Init(0);
#endif

  // Setup LEDs
  pinMode(LED0, OUTPUT);
  digitalWrite(LED0, 0); // turn on
  pinMode(LED1, OUTPUT);
  digitalWrite(LED1, 1); // turn off
  pinMode(LED2, OUTPUT);
  digitalWrite(LED2, 1); // turn off
  pinMode(LED_BUILTIN_R, OUTPUT);
  digitalWrite(LED_BUILTIN_R, 1); // turn off
  pinMode(LED_BUILTIN_G, OUTPUT);
  digitalWrite(LED_BUILTIN_G, 1); // turn off
  pinMode(LED_BUILTIN_B, OUTPUT);
  digitalWrite(LED_BUILTIN_B, 1); // turn off

  // Neopixel
  pixels.begin();
  pixels.setBrightness(50); // out of 255

  usb_hid.setBootProtocol(HID_ITF_PROTOCOL_KEYBOARD);
  usb_hid.setPollInterval(2);
  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  usb_hid.setStringDescriptor("TinyUSB Keyboard");

  // Set up output report (on control endpoint) for Capslock indicator
  // usb_hid.setReportCallback(NULL, hid_report_callback);

  usb_hid.begin();

  for (uint8_t i = 0; i < 3; i++)
  {
    pinMode(cols[i], OUTPUT);
    pinMode(rows[i], INPUT_PULLDOWN);
  }

  // wait until device mounted
  while (!TinyUSBDevice.mounted())
    delay(1);
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                 LOOP                                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

void loop()
{
  // poll gpio once each 2 ms
  delay(2);

  led_blink_divider += 1U;
  led_blink_divider = (led_blink_divider < 128)? led_blink_divider : 0;
  if (led_blink_divider == 0)
  {
    for (uint8_t i = 0; i < sizeof(builtin_leds); i++)
    {
      digitalWrite(builtin_leds[i], 1); // turn off all builtin LEDS
    }
    digitalWrite(builtin_leds[led_index], 0); // turn on the one we want
    led_index += 1U;
    led_index = (led_index < 3)? led_index : 0;
  }

  // used to avoid send multiple consecutive zero report for keyboard
  // static bool keyPressedPreviously = false;
  uint8_t count = 0;
  // uint8_t keycode[9] = {0};

  for (uint8_t i = 0; i < 3; i++)
  {
    // go through each column
    for (uint8_t h = 0; h < 3; h++)
    {
      // set all non-current columns to 0
      if (h == i)
      {
        digitalWrite(cols[i], 1);
      }
      else
      {
        // set current column to 1
        digitalWrite(cols[h], 0);
      }
    }

    delay(1); // prevents accidental reads while prev column was "on"

    for (uint8_t j = 0; j < 3; j++)
    {
      // check each row
      if (digitalRead(rows[j]))
      {
        // keycode[count++] = hidcode[(j * 3) + i]; // put the key press in the buffer
        if (j == 0)
        {
          if (i == 0)
            digitalWrite(LED0, 0); // turn on
          else
            digitalWrite(LED0, 1); // turn off
          if (i == 1)
            digitalWrite(LED1, 0); // turn on
          else
            digitalWrite(LED1, 1); // turn off
          if (i == 2)
            digitalWrite(LED2, 0); // turn on
          else
            digitalWrite(LED2, 1); // turn off
        }
        else if (j == 1)
        {
          if (i == 0)
            pixels.fill(0xff0000);
          if (i == 1)
            pixels.fill(0x00ff00);
          if (i == 2)
            pixels.fill(0x0000ff);
          pixels.show();
        }
        else if (j == 2)
        {
          if (i == 0)
            pixels.fill(0xffff00);
          if (i == 1)
            pixels.fill(0x00ffff);
          if (i == 2)
            pixels.fill(0xff00ff);
          pixels.show();
        }
      }
      if (count == 9)
        break;
    }
  }

  if (TinyUSBDevice.suspended() && count)
  {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    TinyUSBDevice.remoteWakeup();
  }

  // skip if hid is not ready e.g still transferring previous report
  if (!usb_hid.ready())
    return;

  // if (count)
  // {
  //   // Send report if there is key pressed
  //   uint8_t const report_id = 0;
  //   uint8_t const modifier = 0;

  //   keyPressedPreviously = true;
  //   usb_hid.keyboardReport(report_id, modifier, keycode);
  // }
  // else
  // {
  //   // Send All-zero report to indicate there is no keys pressed
  //   // Most of the time, it is, though we don't need to send zero report
  //   // every loop(), only a key is pressed in previous loop()
  //   if (keyPressedPreviously)
  //   {
  //     keyPressedPreviously = false;
  //     usb_hid.keyboardRelease(0);
  //   }
  // }
}