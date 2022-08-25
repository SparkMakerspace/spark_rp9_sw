/*********************************************************************
 See main.cpp for full header-text.

 MIT license, check LICENSE for more information
 Copyright (c) 2022 John Scimone
 All text above must be included in any redistribution
*********************************************************************/

#ifndef KEYMAPPING_H

#define KEYMAPPING_H

#include "Adafruit_TinyUSB.h"
#include <map>

std::map<const char*, uint8_t> keymap;
void setupKeymap()
{
    keymap["a"] = HID_KEY_A;
    keymap["b"] = HID_KEY_B;
    keymap["c"] = HID_KEY_C;
    keymap["d"] = HID_KEY_D;
    keymap["e"] = HID_KEY_E;
    keymap["f"] = HID_KEY_F;
    keymap["g"] = HID_KEY_G;
    keymap["h"] = HID_KEY_H;
    keymap["i"] = HID_KEY_I;
    keymap["j"] = HID_KEY_J;
    keymap["k"] = HID_KEY_K;
    keymap["l"] = HID_KEY_L;
    keymap["m"] = HID_KEY_M;
    keymap["n"] = HID_KEY_N;
    keymap["o"] = HID_KEY_O;
    keymap["p"] = HID_KEY_P;
    keymap["q"] = HID_KEY_Q;
    keymap["r"] = HID_KEY_R;
    keymap["s"] = HID_KEY_S;
    keymap["t"] = HID_KEY_T;
    keymap["u"] = HID_KEY_U;
    keymap["v"] = HID_KEY_V;
    keymap["w"] = HID_KEY_W;
    keymap["x"] = HID_KEY_X;
    keymap["y"] = HID_KEY_Y;
    keymap["z"] = HID_KEY_Z;

    keymap["grave"] = HID_KEY_GRAVE;
    keymap["1"] = HID_KEY_1;
    keymap["2"] = HID_KEY_2;
    keymap["3"] = HID_KEY_3;
    keymap["4"] = HID_KEY_4;
    keymap["5"] = HID_KEY_5;
    keymap["6"] = HID_KEY_6;
    keymap["7"] = HID_KEY_7;
    keymap["8"] = HID_KEY_8;
    keymap["9"] = HID_KEY_9;
    keymap["0"] = HID_KEY_0;
    keymap["minus"] = HID_KEY_MINUS;
    keymap["equal"] = HID_KEY_EQUAL;
    keymap["backspace"] = HID_KEY_BACKSPACE;

    keymap["left_bracket"] = HID_KEY_BRACKET_LEFT;
    keymap["right_bracket"] = HID_KEY_BRACKET_RIGHT;
    keymap["backslash"] = HID_KEY_BACKSLASH;
    keymap["semicolon"] = HID_KEY_SEMICOLON;
    keymap["apostrophe"] = HID_KEY_APOSTROPHE;
    keymap["comma"] = HID_KEY_COMMA;
    keymap["period"] = HID_KEY_PERIOD;
    keymap["slash"] = HID_KEY_SLASH;

    keymap["F1"] = HID_KEY_F1;
    keymap["F2"] = HID_KEY_F2;
    keymap["F3"] = HID_KEY_F3;
    keymap["F4"] = HID_KEY_F4;
    keymap["F5"] = HID_KEY_F5;
    keymap["F6"] = HID_KEY_F6;
    keymap["F7"] = HID_KEY_F7;
    keymap["F8"] = HID_KEY_F8;
    keymap["F9"] = HID_KEY_F9;
    keymap["F10"] = HID_KEY_F10;
    keymap["F11"] = HID_KEY_F11;
    keymap["F12"] = HID_KEY_F12;
    keymap["F13"] = HID_KEY_F13;
    keymap["F14"] = HID_KEY_F14;
    keymap["F15"] = HID_KEY_F15;
    keymap["F16"] = HID_KEY_F16;
    keymap["F17"] = HID_KEY_F17;
    keymap["F18"] = HID_KEY_F18;
    keymap["F19"] = HID_KEY_F19;
    keymap["F20"] = HID_KEY_F20;
    keymap["F21"] = HID_KEY_F21;
    keymap["F22"] = HID_KEY_F22;
    keymap["F23"] = HID_KEY_F23;
    keymap["F24"] = HID_KEY_F24;

    keymap["escape"] = HID_KEY_ESCAPE;
    keymap["enter"] = HID_KEY_ENTER;
    keymap["space"] = HID_KEY_SPACE;

    keymap["print_screen"] = HID_KEY_PRINT_SCREEN;
    keymap["scroll_lock"] = HID_KEY_SCROLL_LOCK;
    keymap["pause"] = HID_KEY_PAUSE;

    keymap["insert"] = HID_KEY_INSERT;
    keymap["delete"] = HID_KEY_DELETE;
    keymap["home"] = HID_KEY_HOME;
    keymap["end"] = HID_KEY_END;
    keymap["page_up"] = HID_KEY_PAGE_UP;
    keymap["page_down"] = HID_KEY_PAGE_DOWN;
    keymap["num_lock"] = HID_KEY_NUM_LOCK;

    keymap["key_0"] = HID_KEY_KEYPAD_0;
    keymap["key_1"] = HID_KEY_KEYPAD_1;
    keymap["key_2"] = HID_KEY_KEYPAD_2;
    keymap["key_3"] = HID_KEY_KEYPAD_3;
    keymap["key_4"] = HID_KEY_KEYPAD_4;
    keymap["key_5"] = HID_KEY_KEYPAD_5;
    keymap["key_6"] = HID_KEY_KEYPAD_6;
    keymap["key_7"] = HID_KEY_KEYPAD_7;
    keymap["key_8"] = HID_KEY_KEYPAD_8;
    keymap["key_9"] = HID_KEY_KEYPAD_9;
    keymap["key_plus"] = HID_KEY_KEYPAD_ADD;
    keymap["key_comma"] = HID_KEY_KEYPAD_COMMA;
    keymap["key_period"] = HID_KEY_KEYPAD_DECIMAL;
    keymap["key_slash"] = HID_KEY_KEYPAD_DIVIDE;
    keymap["key_enter"] = HID_KEY_KEYPAD_ENTER;
    keymap["key_equal"] = HID_KEY_KEYPAD_EQUAL;
    keymap["key_equal_sign"] = HID_KEY_KEYPAD_EQUAL_SIGN;
    keymap["key_asterisk"] = HID_KEY_KEYPAD_MULTIPLY;
    keymap["key_minus"] = HID_KEY_KEYPAD_SUBTRACT;

    keymap["up"] = HID_KEY_ARROW_UP;
    keymap["down"] = HID_KEY_ARROW_DOWN;
    keymap["left"] = HID_KEY_ARROW_LEFT;
    keymap["right"] = HID_KEY_ARROW_RIGHT;

    keymap["undo"] = HID_KEY_UNDO;
    keymap["cut"] = HID_KEY_CUT;
    keymap["copy"] = HID_KEY_COPY;
    keymap["paste"] = HID_KEY_PASTE;
    keymap["find"] = HID_KEY_FIND;

    keymap["mute"] = HID_KEY_MUTE;
    keymap["vol_up"] = HID_KEY_VOLUME_UP;
    keymap["vol_down"] = HID_KEY_VOLUME_DOWN;

    keymap["power"] = HID_KEY_POWER;
    keymap["compose"] = HID_KEY_APPLICATION;
}

#endif