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
  usb_msc.setID("Spark", "RP9 Macropad", "1.0");

  // Set callback
  usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);

  // Set disk size, block size should be 512 regardless of spi flash page size
  usb_msc.setCapacity(flash.size() / 512, 512);

  // MSC is ready for read/write
  usb_msc.setUnitReady(true);

  usb_msc.begin();

  setupKeymap();

  // Set up rows and columns
  for (auto col : cols)
  {
    pinMode(col, OUTPUT);
    digitalWrite(col, false);
  }

  for (auto row : rows)
  {
    pinMode(row, INPUT_PULLDOWN);
  }

  // Set up LEDs
  for (auto led : leds)
  {
    pinMode(led, OUTPUT);
    digitalWrite(led, true);
  }

  for (auto led : rgbLeds)
  {
    pinMode(led, OUTPUT);
    digitalWrite(led, true);
  }
  pinMode(PIN_NEOPIXEL, OUTPUT);
  digitalWrite(PIN_NEOPIXEL, false);
  pinMode(NEOPIXEL_POWER, OUTPUT);
  digitalWrite(NEOPIXEL_POWER, true);

  // Init file system on the flash
  fs_formatted = fatfs.begin(&flash);

  // Notes: following commented-out functions has no affect on ESP32
  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));

  usb_hid.begin();

  Serial.begin(9600);
  if (!fs_formatted)
  {
    Serial.println("Failed to init files system, flash may not be formatted");
  }

  Serial.println("Spark_RP9 Macropad");
  Serial.print("JEDEC ID: 0x");
  Serial.println(flash.getJEDECID(), HEX);
  Serial.print("Flash size: ");
  Serial.print(flash.size() / 1024);
  Serial.println(" KB");
  fs_changed = true; // to print contents initially
  invalidConfig = true;

  currpage = 0;
  pagechanged = true;
  modifier = 0;

  np.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  np.show();            // Turn OFF all pixels ASAP
  np.setBrightness(20); // Set BRIGHTNESS to about 1/5 (max = 255)
}

void loop()
{
  if (fs_changed || invalidConfig)
  {
    delay(500); // refresh every 5 seconds
    fs_changed = false;

    // check if host formatted disk
    if (!fs_formatted)
    {
      fs_formatted = fatfs.begin(&flash);
    }

    // skip if still not formatted
    if (!fs_formatted)
      return;

    if (!root.open("/"))
    {
      Serial.println("open root failed");
      return;
    }

    Serial.println("Trying to open config.json");

    // Serial.println("Flash contents:");
    // root.ls(LS_R | LS_DATE | LS_SIZE);
    if (file.open("/config.json"))
    {
      if (parseConfig(file))
      {
        invalidConfig = false;
        currpage = 0;
        blinkGreen(8);
        pagechanged = true;
      }
      else
      {
        invalidConfig = true;
        blinkRed(1);
      }
      file.close();
    }

    root.close();
  }
  else
  {
    ////////////////////////////////////
    // THE KEYPAD PORTION OF THE LOOP //
    ////////////////////////////////////

    // used to avoid send multiple consecutive zero report for keyboard
    static bool keyPressedPreviously = false;

    // clear keycode buffer
    keycode.fill(0);
    count = 0;

    // if we changed keypages, adjust LEDs and stuff
    if (pagechanged)
    {
      for (int i = 0; i < sizeof(leds); i++)
      {
        digitalWrite(leds[i], !keypages.at(currpage)->leds[i]); // note I invert logic because LEDs are active low
      }

      for (int i = 0; i < sizeof(rgbLeds); i++)
      {
        digitalWrite(rgbLeds[i], !keypages.at(currpage)->builtinleds[i]); // note I invert logic because LEDs are active low
      }
      np.setPixelColor(0, keypages.at(currpage)->neopixel);
      np.show();
      pagechanged = false;
    }

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

      keyPressedPreviously = true;
      usb_hid.keyboardReport(report_id, modifier, keycode.data());
      modifier = 0;
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
}

//------------------------------------------------------------------+
// Functions
//------------------------------------------------------------------+

void removeSpace(char *s)
{
  for (char *s2 = s; *s2; ++s2)
  {
    if (*s2 != ' ')
      *s++ = *s2;
  }
  *s = 0;
}

// i is the key that was pressed
void handleKeypress(int i)
{
  // if we're not supposed to change pages, get the HID code
  if (keypages[currpage]->pagechange[i] == 69) // 69 means no page change
  {
    keycode[count++] = keypages[currpage]->hidcode[i];
    modifier = keypages[currpage]->modcode[i];
  }
  // otherwise, change pages :)
  else
  {
    int lastpage = currpage;
    currpage = keypages[currpage]->pagechange[i];
    pagechanged = (lastpage == currpage) ? false : true;
  }
}

// returns true is successful, false if failed
bool parseConfig(FatFile configfile)
{
  // tracks if we've allocated memory for keypages
  static bool memAllocated = false;

  // these are some basic config checks we can do
  bool allPagesHaveAKeysElement = true;
  bool allPagesHaveAPageElement = true;
  bool allPagesHaveALedsElement = true;
  bool allPagesHaveNineKeys = true;
  bool allPagesHaveAllLeds = true;
  bool noPagesBiggerThanEight = true;
  bool zeropageExists = false;

  // buffer for reading the config file (one byte per character)
  char rdbuf[5000];
  char *rdbuf_p = &rdbuf[0];
  // keep track of how many bytes we actually read
  int bytesRead = 0;
  // Json Document for parsing
  DynamicJsonDocument doc(3072);
  // read the file!
  Serial.println("Trying to read config.json");
  Serial.flush();
  while (configfile.read(&rdbuf[bytesRead], 1) == 1)
  {
    bytesRead++;
  }

  // check that we read the whole thing!
  if (bytesRead < configfile.fileSize())
  {
    Serial.println("could not read entire config.json file");
    Serial.flush();
    delay(100);
    return false;
  }

  // parsing time
  Serial.println("Trying to parse data");
  Serial.flush();
  DeserializationError error = deserializeJson(doc, rdbuf, bytesRead);

  // detect parsing error
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return false;
  }
  Serial.println("No problem parsing file");
  // allocate memory for all the keypages - ONLY ONCE EVER
  if (!memAllocated)
  {
    for (int y = 0; y < 9; y++)
    {
      keypages[y] = new Keypage();
    }
    memAllocated = true;
    Serial.println("memory allocated for keypages");
  }
  else
  {
    Serial.println("skipping memory allocation");
  }

  // Config checks!
  if (doc["pages"].isNull())
  {
    Serial.println("config.json has no top level 'pages' array");
    return false;
  }
  if (doc["pages"].as<JsonArray>().size() < 1)
  {
    Serial.println("'pages' array must have at least one element");
    return false;
  }
  for (auto a : doc["pages"].as<JsonArray>())
  {
    if (a["keys"].isNull())
    {
      Serial.println("All pages must have a 'keys' element");
      return false;
    }
    if (a["keys"]["1"].isNull() || a["keys"]["2"].isNull() || a["keys"]["3"].isNull() ||
        a["keys"]["4"].isNull() || a["keys"]["5"].isNull() || a["keys"]["6"].isNull() ||
        a["keys"]["7"].isNull() || a["keys"]["8"].isNull() || a["keys"]["9"].isNull() || a["keys"].size() != 9)
    {
      Serial.println("All pages must have only elements '1' through '9' under 'keys'");
      return false;
    }
    if (a["page"].isNull())
    {
      Serial.println("All pages must have a 'page' element");
      return false;
    }
    if (a["page"] > 8)
    {
      Serial.println("Page element values must be less than 9");
      return false;
    }
    if (a["leds"].isNull())
    {
      Serial.println("All pages must have a 'leds' element");
      return false;
    }
    if (a["leds"]["led1"].isNull() || a["leds"]["led2"].isNull() || a["leds"]["led3"].isNull() ||
        a["leds"]["ledR"].isNull() || a["leds"]["ledG"].isNull() || a["leds"]["ledB"].isNull() ||
        a["leds"]["neopixel"].isNull())
    {
      Serial.println("All leds must be included in each page under the 'leds' key");
      return false;
    }
    if (a["page"] == 0)
    {
      zeropageExists = true;
    }
  }
  if (!zeropageExists)
  {
    Serial.println("A page numbered '0' must exist");
    return false;
  }

  // Fun fact: keypages are 0-indexed and everything else isn't
  for (JsonObject page : doc["pages"].as<JsonArray>())
  {
    // regex used to determine if a key should switch pages
    std::string pagePattern = "^page [0-9]$";
    std::regex pageRegex(pagePattern);

    // regex to identify modifier keys
    std::string altPattern = "(.*)(alt)(.*)";
    std::regex altRegex(altPattern.c_str());
    std::string ctlPattern = "(.*)(ctl)(.*)";
    std::regex ctlRegex(ctlPattern.c_str());
    std::string shiftPattern = "(.*)(shift)(.*)";
    std::regex shiftRegex(shiftPattern.c_str());
    std::string guiPattern = "(.*)(gui)(.*)";
    std::regex guiRegex(guiPattern.c_str());

    int page_page = page["page"]; // the current page number

    JsonObject page_keys = page["keys"];
    const char *page_key_array[] = {page_keys["1"], page_keys["2"], page_keys["3"],
                                    page_keys["4"], page_keys["5"], page_keys["6"],
                                    page_keys["7"], page_keys["8"], page_keys["9"]}; // all keys

    JsonObject page_leds = page["leds"];
    const char *page_leds_neopixel = page_leds["neopixel"];                             // Xiao RP2040 builtin Neopixel
    bool page_leds_array[] = {page_leds["led1"], page_leds["led2"], page_leds["led3"]}; // all leds
    bool page_rgb_array[] = {page_leds["ledR"], page_leds["ledG"], page_leds["ledB"]};  // all builtin RGBs

    keypages[page_page]->page = page_page;
    // assign all the things in each keypage
    for (int j = 0; j < (sizeof(page_key_array) / sizeof(page_key_array[0])); j++)
    {
      // check if it's a page change assignment
      if (std::regex_match(page_key_array[j], pageRegex))
      {
        keypages[page_page]->pagechange[j] = page_key_array[j][5] - 0x30; // ascii to integer
      }
      // determine what hidcode or page change applies
      else
      {
        keypages[page_page]->pagechange[j] = 69; // no page change :)

        // copy the page_key so we can strip the spaces
        char this_page_key[50];
        strncpy(this_page_key, page_key_array[j], 49);
        removeSpace(this_page_key);

        // check for modifier keys
        if (std::regex_match(this_page_key, altRegex))
        {
          keypages[page_page]->modcode[j] += KEYBOARD_MODIFIER_LEFTALT;
        }
        if (std::regex_match(this_page_key, ctlRegex))
        {
          keypages[page_page]->modcode[j] += KEYBOARD_MODIFIER_LEFTCTRL;
        }
        if (std::regex_match(this_page_key, shiftRegex))
        {
          keypages[page_page]->modcode[j] += KEYBOARD_MODIFIER_LEFTSHIFT;
        }
        if (std::regex_match(this_page_key, guiRegex))
        {
          keypages[page_page]->modcode[j] += KEYBOARD_MODIFIER_LEFTGUI;
        }

        // find the position of the rightmost + if there is one
        char *last_plus = std::strrchr(this_page_key, '+');
        char *keycode;
        // if not NULL, then there was a +. Check starting from next char
        if (last_plus != NULL)
        {
          keycode = &last_plus[1];
        }
        else
        {
          keycode = this_page_key;
        }
        // check each keycode to see if it matches
        for (auto k : keymap)
        {
          if (!strcmp(keycode, k.first))
          {
            keypages[page_page]->hidcode[j] = k.second;
            break;
          }
        }
        // by default if nothing matches, the default hidcode of 0 applies

        // copy the led config in
        std::copy(std::begin(page_leds_array),
                  std::end(page_leds_array),
                  keypages[page_page]->leds.begin());
        // copy the builtin led config in
        std::copy(std::begin(page_rgb_array),
                  std::end(page_rgb_array),
                  keypages[page_page]->builtinleds.begin());
        // copy the neopixel config in
        keypages[page_page]->neopixel = std::stoi(page_leds_neopixel, nullptr, 16);
      }
    }
  }
  Serial.println("config.json parsed successfully");
  return true;
}

// n is how many times.
void blinkGreen(int n)
{
  for (int i = 0; i < 3; i++)
  {
    digitalWrite(rgbLeds[i], true);
  }
  for (int i = 0; i < n; i++)
  {
    digitalWrite(rgbLeds[1], false);
    delay(100);
    digitalWrite(rgbLeds[1], true);
    delay(100);
  }
}

// n is how many times.
void blinkRed(int n)
{
  for (int i = 0; i < 3; i++)
  {
    digitalWrite(rgbLeds[i], true);
  }
  for (int i = 0; i < n; i++)
  {
    digitalWrite(rgbLeds[0], false);
    delay(200);
    digitalWrite(rgbLeds[0], true);
    delay(250);
  }
}