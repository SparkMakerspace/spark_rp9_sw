/*********************************************************************
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 Copyright (c) 2019 Ha Thach for Adafruit Industries
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

/* This sketch demonstrates USB Mass Storage and HID mouse (and CDC)
 * - Enumerated as disk using on-board external flash
 * - Press button pin will move mouse toward bottom right of monitor
 */

#include "main.h"

// the setup function runs once when you press reset or power the board
void setup()
{
  flash.begin();

  pinMode(LED_BUILTIN, OUTPUT);

  // Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
  usb_msc.setID("Adafruit", "External Flash", "1.0");

  // Set callback
  usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);

  // Set disk size, block size should be 512 regardless of spi flash page size
  usb_msc.setCapacity(flash.size()/512, 512);

  // MSC is ready for read/write
  usb_msc.setUnitReady(true);
  
  usb_msc.begin();

  // Set up rows and columns
  pinMode(COL1, OUTPUT);
  digitalWrite(COL1,0);
  pinMode(COL2, OUTPUT);
  digitalWrite(COL2,0);
  pinMode(COL3, OUTPUT);
  digitalWrite(COL3,0);
  pinMode(ROW1, INPUT_PULLDOWN);
  pinMode(ROW2, INPUT_PULLDOWN);
  pinMode(ROW3, INPUT_PULLDOWN);
  pinMode(LED1, OUTPUT);
  digitalWrite(LED1,1);
  pinMode(LED2, OUTPUT);
  digitalWrite(LED2,1);
  pinMode(LED3, OUTPUT);
  digitalWrite(LED3,1);
  pinMode(PIN_LED_R, OUTPUT);
  digitalWrite(PIN_LED_R,1);
  pinMode(PIN_LED_G, OUTPUT);
  digitalWrite(PIN_LED_G,1);
  pinMode(PIN_LED_B, OUTPUT);
  digitalWrite(PIN_LED_B,1);


  // Init file system on the flash
  fs_formatted = fatfs.begin(&flash);


  // Notes: following commented-out functions has no affect on ESP32
  // usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));

  usb_hid.begin();

  Serial.begin(115200);
  while ( !Serial ) delay(10);   // wait for native usb

   if ( !fs_formatted )
  {
    Serial.println("Failed to init files system, flash may not be formatted");
  }

  Serial.println("Spark_RP9 Macropad");
  Serial.print("JEDEC ID: 0x"); Serial.println(flash.getJEDECID(), HEX);
  Serial.print("Flash size: "); Serial.print(flash.size() / 1024); Serial.println(" KB");

  fs_changed = true; // to print contents initially

  rp2040.fifo.push((uint32_t)&defpage);
}

void loop()
{


  if ( fs_changed )
  {
    fs_changed = false;

    // check if host formatted disk
    if (!fs_formatted)
    {
      fs_formatted = fatfs.begin(&flash);
    }

    // skip if still not formatted
    if (!fs_formatted) return;

    Serial.println("Opening root");

    if ( !root.open("/") )
    {
      Serial.println("open root failed");
      return;
    }

    Serial.println("Flash contents:");

    // Open next file in root.
    // Warning, openNext starts at the current directory position
    // so a rewind of the directory may be required.
    while ( file.openNext(&root, O_RDONLY) )
    {
      file.printFileSize(&Serial);
      Serial.write(' ');
      file.printName(&Serial);
      if ( file.isDir() )
      {
        // Indicate a directory.
        Serial.write('/');
      }
      Serial.println();
      file.close();
    }

    root.close();

    Serial.println();
    delay(1000); // refresh every 1 second
  }
}

keypage* p;

void setup1(){
  while(rp2040.fifo.available() < 1){
    delay(100); // sit and spin
  }
  p = (keypage*)(rp2040.fifo.pop());

}

void loop1(){
  bool button_pressed = false;
  // Remote wakeup
  if ( TinyUSBDevice.suspended() && button_pressed)
  {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    tud_remote_wakeup();
  }
}

//------------------------------------------------------------------+
// Mass Storage Class callback functions
//------------------------------------------------------------------+

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and 
// return number of copied bytes (must be multiple of block size) 
int32_t msc_read_cb (uint32_t lba, void* buffer, uint32_t bufsize)
{
  // Note: SPIFLash Block API: readBlocks/writeBlocks/syncBlocks
  // already include 4K sector caching internally. We don't need to cache it, yahhhh!!
  return flash.readBlocks(lba, (uint8_t*) buffer, bufsize/512) ? bufsize : -1;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and 
// return number of written bytes (must be multiple of block size)
int32_t msc_write_cb (uint32_t lba, uint8_t* buffer, uint32_t bufsize)
{
  digitalWrite(LED_BUILTIN, HIGH);
  // Note: SPIFLash Block API: readBlocks/writeBlocks/syncBlocks
  // already include 4K sector caching internally. We don't need to cache it, yahhhh!!
  return flash.writeBlocks(lba, buffer, bufsize/512) ? bufsize : -1;
}

// Callback invoked when WRITE10 command is completed (status received and accepted by host).
// used to flush any pending cache.
void msc_flush_cb (void)
{
  flash.syncBlocks();
  
  // clear file system's cache to force refresh
  fatfs.cacheClear();

  fs_changed = true;

  digitalWrite(LED_BUILTIN, LOW);
}