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

#include "main.h"

// Setup and Loop initialize the device and watch for config file changes. When the config file changes, core1 is restarted
void setup()
{
  flash.begin();

  pinMode(LED_BUILTIN, OUTPUT);

  // Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
  usb_msc.setID("Adafruit", "External Flash", "1.0");

  // Set callback
  usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);

  // Set disk size, block size should be 512 regardless of spi flash page size
  usb_msc.setCapacity(flash.size() / 512, 512);

  // MSC is ready for read/write
  usb_msc.setUnitReady(true);

  usb_msc.begin();

  // Set up rows and columns
  for (auto col : cols)
  {
    pinMode(col, OUTPUT);
    digitalWrite(col, 0);
  }

  for (auto row : rows)
  {
    pinMode(row, INPUT_PULLDOWN);
  }

  // Set up LEDs
  for (auto led : leds)
  {
    pinMode(led, OUTPUT);
    digitalWrite(led, 1);
  }

  for (auto led : rgbLeds)
  {
    pinMode(led, OUTPUT);
    digitalWrite(led, 1);
  }
  pinMode(PIN_NEOPIXEL, OUTPUT);
  digitalWrite(PIN_NEOPIXEL, 0);
  pinMode(NEOPIXEL_POWER, OUTPUT);
  digitalWrite(NEOPIXEL_POWER, 1);

  // Init file system on the flash
  fs_formatted = fatfs.begin(&flash);

  // Notes: following commented-out functions has no affect on ESP32
  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));

  usb_hid.begin();

  Serial.begin(115200);
  while (!Serial)
    delay(10); // wait for native usb

  if (!fs_formatted)
  {
    Serial.println("Failed to init files system, flash may not be formatted");
  }

  // Serial.println("Spark_RP9 Macropad");
  // Serial.print("JEDEC ID: 0x"); Serial.println(flash.getJEDECID(), HEX);
  // Serial.print("Flash size: "); Serial.print(flash.size() / 1024); Serial.println(" KB");

  fs_changed = true; // to print contents initially

  // read the config file and populate the keypages
  // TODO: actually do this.

  std::array<int, 9> page0Changes;
  page0Changes.fill(69);
  page0Changes[1] = 1;
  std::array<uint8_t, 9> page0Keycodes = {0, 0, HID_KEY_1,
                                          HID_KEY_2, HID_KEY_3, HID_KEY_4,
                                          HID_KEY_5, HID_KEY_6, HID_KEY_7};
  page0.page = 0;
  page0.pagechange = page0Changes;
  page0.hidcode = page0Keycodes;
  keypages[0] = &page0;

  std::array<int, 9> page1Changes;
  page1Changes.fill(69);
  page1Changes[0] = 0;
  std::array<uint8_t, 9> page1Keycodes = {0, 0, HID_KEY_A,
                                          HID_KEY_B, HID_KEY_C, HID_KEY_D,
                                          HID_KEY_E, HID_KEY_F, HID_KEY_G};
  page1.page = 1;
  page1.pagechange = page1Changes;
  page1.hidcode = page1Keycodes;
  keypages[1] = &page1;

  // send the initial keypages over to core1
  rp2040.fifo.push((uint32_t)&keypages);
  builtinLED_RState = 0;
}

void loop()
{
  builtinLED_RState = (builtinLED_RState >= 1) ? 0 : 1;
  digitalWrite(PIN_LED_R, builtinLED_RState);
  if (fs_changed)
  {
    fs_changed = false;

    // check if host formatted disk
    if (!fs_formatted)
    {
      fs_formatted = fatfs.begin(&flash);
    }

    // skip if still not formatted
    if (!fs_formatted)
      return;

    // Serial.println("Opening root");

    if (!root.open("/"))
    {
      // Serial.println("open root failed");
      return;
    }

    // Serial.println("Flash contents:");

    // Open next file in root.
    // Warning, openNext starts at the current directory position
    // so a rewind of the directory may be required.
    while (file.openNext(&root, O_RDONLY))
    {
      // file.printFileSize(&Serial);
      // Serial.write(' ');
      // file.printName(&Serial);
      if (file.isDir())
      {
        // Indicate a directory.
        // Serial.write('/');
      }
      // Serial.println();
      file.close();
    }

    root.close();

    // Serial.println();
    delay(500); // refresh every .5 second
  }
}

// Setup 1 and Loop 1 support key polling and keyboard output
void setup1()
{
  // wait until the keypages are sent over and then get 'em
  core1Keypages = (std::array<Keypage *, 9> *)rp2040.fifo.pop();
  builtinLED_BState = 0;
  currpage = 0;
  for (Keypage *kp : *core1Keypages)
  {
    if (kp != nullptr)
    {
      Serial.print("Page ->  ");
      Serial.println((*kp).page);
    }
  }
}

void loop1()
{
  builtinLED_BState = (builtinLED_BState >= 1) ? 0 : 1;
  digitalWrite(PIN_LED_B, builtinLED_BState);

  // poll gpio once each 2 ms
  delay(2);

  // used to avoid send multiple consecutive zero report for keyboard
  static bool keyPressedPreviously = false;

  // clear keycode buffer
  keycode.fill(0);
  count = 0;

  // scol = selected column
  for (auto scol : cols)
  {
    // set each column to on or off
    for (auto col : cols)
    {
      if (col == scol)
      {
        digitalWrite(col, true);
      }
      else
      {
        digitalWrite(col, false);
      }
    }
    // check each row
    for (auto row : rows)
    {
      // needed to prevent inaccurate key reading
      delay(2);
      // and now - I present to you the big ugly if else block!
      if (digitalRead(row))
      {
        if (row == ROW1 && scol == COL1)
        {
          handleKeypress(0);
        }
        else if (row == ROW1 && scol == COL2)
        {
          handleKeypress(1);
        }
        else if (row == ROW1 && scol == COL3)
        {
          handleKeypress(2);
        }
        else if (row == ROW2 && scol == COL1)
        {
          handleKeypress(3);
        }
        else if (row == ROW2 && scol == COL2)
        {
          handleKeypress(4);
        }
        else if (row == ROW2 && scol == COL3)
        {
          handleKeypress(5);
        }
        else if (row == ROW3 && scol == COL1)
        {
          handleKeypress(6);
        }
        else if (row == ROW3 && scol == COL2)
        {
          handleKeypress(7);
        }
        else if (row == ROW3 && scol == COL3)
        {
          handleKeypress(8);
        }
        // 6 is max keycode per report per the HID specification
        if (count == 6)
          break;
      }
    }
  }

  // Remote wakeup
  if (TinyUSBDevice.suspended() && count)
  {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    TinyUSBDevice.remoteWakeup();
  }

  // skip if hid is not ready e.g still transferring previous report
  if (!usb_hid.ready())
    return;

  if (count)
  {
    // Send report if there is key pressed
    uint8_t const report_id = 0;
    uint8_t const modifier = 0;

    keyPressedPreviously = true;
    usb_hid.keyboardReport(report_id, modifier, keycode.data());
  }
  else
  {
    // Send All-zero report to indicate there are no keys pressed
    // Most of the time this is the case, however we don't need
    // to send zero report every loop(), only a key is pressed
    // in previous loop()
    if (keyPressedPreviously)
    {
      keyPressedPreviously = false;
      usb_hid.keyboardRelease(0);
    }
  }
}

//------------------------------------------------------------------+
// Mass Storage Class callback functions
//------------------------------------------------------------------+

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and
// return number of copied bytes (must be multiple of block size)
int32_t msc_read_cb(uint32_t lba, void *buffer, uint32_t bufsize)
{
  // Note: SPIFLash Block API: readBlocks/writeBlocks/syncBlocks
  // already include 4K sector caching internally. We don't need to cache it, yahhhh!!
  return flash.readBlocks(lba, (uint8_t *)buffer, bufsize / 512) ? bufsize : -1;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and
// return number of written bytes (must be multiple of block size)
int32_t msc_write_cb(uint32_t lba, uint8_t *buffer, uint32_t bufsize)
{
  digitalWrite(LED_BUILTIN, HIGH);
  // Note: SPIFLash Block API: readBlocks/writeBlocks/syncBlocks
  // already include 4K sector caching internally. We don't need to cache it, yahhhh!!
  return flash.writeBlocks(lba, buffer, bufsize / 512) ? bufsize : -1;
}

// Callback invoked when WRITE10 command is completed (status received and accepted by host).
// used to flush any pending cache.
void msc_flush_cb(void)
{
  flash.syncBlocks();

  // clear file system's cache to force refresh
  fatfs.cacheClear();

  fs_changed = true;

  digitalWrite(LED_BUILTIN, LOW);
}

// i is the key that was pressed
void handleKeypress(int i)
{
  // if we're not supposed to change pages, get the HID code
  if ((*core1Keypages)[currpage]->pagechange[i] == 69)
  {
    keycode[count++] = (*core1Keypages)[currpage]->hidcode[i];
  }
  // otherwise, change pages :)
  else
  {
    currpage = (*core1Keypages)[currpage]->pagechange[i];
  }
}