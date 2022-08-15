/*********************************************************************
 Author: John Scimone
 Organization: Spark Markerspace Co

 This software is for the Spark_RP9 - a simple 9-key macropad built 
 around the RP2040 that an electronics beginner can assemble and 
 configure.

 MIT license, check LICENSE for more information
 Copyright (c) 2022 John Scimone
 All text above must be included in any redistribution
*********************************************************************/

#include <Arduino.h>
#include <xiaorp2040.h>
#include <Adafruit_NeoPixel.h>

//------------- Neopixel -------------//
#ifdef PIN_NEOPIXEL
// How many NeoPixels are attached to the Arduino?
// use on-board defined NEOPIXEL_NUM if existed
#ifndef NEOPIXEL_NUM
#define NEOPIXEL_NUM 1
#endif
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NEOPIXEL_NUM, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
#endif

uint8_t cols[] = {D8, D9, D10};
uint8_t rows[] = {D1, D2, D3};

uint8_t led_blink_counter;
uint8_t led_index = 0;

uint8_t builtin_leds[] = {LED_BUILTIN_R, LED_BUILTIN_G, LED_BUILTIN_B};

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                SETUP                                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void setup()
{
  led_blink_counter = 0;
#if defined(PIN_NEOPIXEL_POWER)
  pinMode(PIN_NEOPIXEL_POWER, OUTPUT);
  digitalWrite(PIN_NEOPIXEL_POWER, 1);
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
  pixels.setBrightness(20); // out of 255

  for (uint8_t i = 0; i < 3; i++)
  {
    pinMode(cols[i], OUTPUT);
    pinMode(rows[i], INPUT_PULLDOWN);
  }
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

  // Count loops
  led_blink_counter += 1U;
  // reset the count at 32
  led_blink_counter = (led_blink_counter < 32)? led_blink_counter : 0;
  // when the counter is 0, change color
  if (led_blink_counter == 0)
  {
    // loop over all builtin LEDs turning them all off - this could be much more efficient
    for (uint8_t i = 0; i < sizeof(builtin_leds); i++)
    {
      digitalWrite(builtin_leds[i], 1); // turn off all builtin LEDS
    }
    // the value of led_index (0, 1, or 2) tells us which buildin LED to turn on
    digitalWrite(builtin_leds[led_index], 0); // turn on the one we want
    // increment the led index for next time
    led_index += 1U;
    // wrap the led index back to 0 after 2 since there are only three of them
    led_index = (led_index < 3)? led_index : 0;
  }

  for (uint8_t i = 0; i < 3; i++)
  {
    // go through each column
    for (uint8_t h = 0; h < 3; h++)
    {
      // set the active column to 1, otherwise 0.
      if (h == i)
      {
        digitalWrite(cols[i], 1);
      }
      else
      {
        digitalWrite(cols[h], 0);
      }
      delay(2);
    }

    for (uint8_t j = 0; j < 3; j++)
    {
      // check each row
      if (digitalRead(rows[j]))
      {
        if (j == 0) // top row
        {
          if (i == 0) // left column 
            digitalWrite(LED0, 0); // turn on
          else
            digitalWrite(LED0, 1); // turn off
          if (i == 1) // middle column
            digitalWrite(LED1, 0); // turn on
          else
            digitalWrite(LED1, 1); // turn off
          if (i == 2) // right column
            digitalWrite(LED2, 0); // turn on
          else
            digitalWrite(LED2, 1); // turn off
        }
        else if (j == 1) // middle row
        {
          if (i == 0) // left column
            pixels.fill(0xff0000);
          if (i == 1) // middle column
            pixels.fill(0x00ff00);
          if (i == 2) // right column
            pixels.fill(0x0000ff);
          pixels.show();
        }
        else if (j == 2) // bottom row
        {
          if (i == 0) // left column
            pixels.fill(0xffff00);
          if (i == 1) // middle column
            pixels.fill(0x00ffff);
          if (i == 2) // right column
            pixels.fill(0xff00ff);
          pixels.show();
        }
      }
    }
  }

}