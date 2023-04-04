// https://github.com/unxed/xkb2win
// License: CC0-1.0 license

#include <ctype.h>
#include <xkbcommon/xkbcommon.h>

// Auxiliary constants and functions necessary for the implementation
// of win32-input-protocol or far2l terminal extensions protocol on *nix systems.
// https://github.com/microsoft/terminal/blob/main/doc/specs/%234999%20-%20Improved%20keyboard%20handling%20in%20Conpty.md
// https://github.com/elfmz/far2l/blob/master/WinPort/FarTTY.h

// Modifier key state constants required to populate the dwControlKeyState field.
// Source: https://learn.microsoft.com/en-us/windows/console/key-event-record-str
#define RIGHT_ALT_PRESSED     0x0001 // the right alt key is pressed.
#define LEFT_ALT_PRESSED      0x0002 // the left alt key is pressed.
#define RIGHT_CTRL_PRESSED    0x0004 // the right ctrl key is pressed.
#define LEFT_CTRL_PRESSED     0x0008 // the left ctrl key is pressed.
#define SHIFT_PRESSED         0x0010 // the shift key is pressed.
#define NUMLOCK_ON            0x0020 // the numlock light is on.
#define SCROLLLOCK_ON         0x0040 // the scrolllock light is on.
#define CAPSLOCK_ON           0x0080 // the capslock light is on.
#define ENHANCED_KEY          0x0100 // the key is enhanced.

// Additional useful codes requested for standardization.
// See https://github.com/microsoft/terminal/issues/337
//     https://github.com/cyd01/KiTTY/pull/435/files
#define LEFT_SHIFT_PRESSED    0x0200 // the left shift key is pressed.
#define RIGHT_SHIFT_PRESSED   0x0400 // the right shift key is pressed.

// Windows Virtual Key Codes list:
// https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
// Windows Virtual Scan Codes list:
// https://docs.vmware.com/en/VMware-Workstation-Player-for-Windows/17.0/com.vmware.player.win.using.doc/GUID-D2C43B86-32EF-44EA-A2ED-D890483D70BD.html
// More info on Windows Key and Scan codes:
// https://chromium.googlesource.com/chromium/src/+/refs/heads/main/ui/events/keycodes/keyboard_code_conversion_x.cc
// https://chromium.googlesource.com/chromium/src/+/refs/heads/main/ui/events/keycodes/dom/dom_code_data.inc

// This function translates XKB KeySym value (as defined /usr/include/X11/keysymdef.h)
// to array containing three values needed for win32-like terminal input protocols:
// 1. wVirtualKeyCode value
// 2. wVirtualScanCode value
// 3. Boolean ENHANCED_KEY flag state
// Boolean ENHANCED_KEY flag state should be applied to dwControlKeyState field as follows:
// dwControlKeyState |= (enhanced_flag_state ? ENHANCED_KEY : 0);
static unsigned char *xkb_to_winkey(int code)
{
	if ((code < 128) && isalpha(code)) { code = toupper(code); }

	switch (code) {

		case XKB_KEY_BackSpace:    { static unsigned char arr[3] = {  8,  14,   0}; return arr; } // VK_BACK
		case XKB_KEY_Tab:          { static unsigned char arr[3] = {  9,  15,   0}; return arr; } // VK_TAB
		case XKB_KEY_KP_Begin:     { static unsigned char arr[3] = { 12,  76,   0}; return arr; } // VK_CLEAR
		case XKB_KEY_Return:       { static unsigned char arr[3] = { 13,  28,   0}; return arr; } // VK_RETURN
		case XKB_KEY_KP_Enter:     { static unsigned char arr[3] = { 13,  28,   1}; return arr; } // VK_RETURN
		case XKB_KEY_Shift_L:      { static unsigned char arr[3] = { 16,  42,   0}; return arr; } // VK_SHIFT
		case XKB_KEY_Shift_R:      { static unsigned char arr[3] = { 16,  54,   0}; return arr; } // VK_SHIFT
		case XKB_KEY_Control_L:    { static unsigned char arr[3] = { 17,  29,   0}; return arr; } // VK_CONTROL
		case XKB_KEY_Control_R:    { static unsigned char arr[3] = { 17,  29,   1}; return arr; } // VK_CONTROL
		case XKB_KEY_Alt_L:        { static unsigned char arr[3] = { 18,  56,   0}; return arr; } // VK_MENU
		case XKB_KEY_Alt_R:        { static unsigned char arr[3] = { 18,  56,   1}; return arr; } // VK_MENU
		case XKB_KEY_Caps_Lock:    { static unsigned char arr[3] = { 20,  58,   0}; return arr; } // VK_CAPITAL
		case XKB_KEY_Escape:       { static unsigned char arr[3] = { 27,   1,   0}; return arr; } // VK_ESCAPE
		case XKB_KEY_space:        { static unsigned char arr[3] = { 32,  57,   0}; return arr; } // VK_SPACE
		case XKB_KEY_Page_Up:      { static unsigned char arr[3] = { 33,  73,   1}; return arr; } // VK_PRIOR
		case XKB_KEY_KP_Page_Up:   { static unsigned char arr[3] = { 33,  73,   0}; return arr; } // VK_PRIOR
		case XKB_KEY_Page_Down:    { static unsigned char arr[3] = { 34,  81,   1}; return arr; } // VK_NEXT
		case XKB_KEY_KP_Page_Down: { static unsigned char arr[3] = { 34,  81,   0}; return arr; } // VK_NEXT
		case XKB_KEY_End:          { static unsigned char arr[3] = { 35,  79,   1}; return arr; } // VK_END
		case XKB_KEY_KP_End:       { static unsigned char arr[3] = { 35,  79,   0}; return arr; } // VK_END
		case XKB_KEY_Home:         { static unsigned char arr[3] = { 36,  71,   1}; return arr; } // VK_HOME
		case XKB_KEY_KP_Home:      { static unsigned char arr[3] = { 36,  71,   0}; return arr; } // VK_HOME
		case XKB_KEY_Left:         { static unsigned char arr[3] = { 37,  75,   1}; return arr; } // VK_LEFT
		case XKB_KEY_KP_Left:      { static unsigned char arr[3] = { 37,  75,   0}; return arr; } // VK_LEFT
		case XKB_KEY_Up:           { static unsigned char arr[3] = { 38,  72,   1}; return arr; } // VK_UP
		case XKB_KEY_KP_Up:        { static unsigned char arr[3] = { 38,  72,   0}; return arr; } // VK_UP
		case XKB_KEY_Right:        { static unsigned char arr[3] = { 39,  77,   1}; return arr; } // VK_RIGHT
		case XKB_KEY_KP_Right:     { static unsigned char arr[3] = { 39,  77,   0}; return arr; } // VK_RIGHT
		case XKB_KEY_Down:         { static unsigned char arr[3] = { 40,  80,   1}; return arr; } // VK_DOWN
		case XKB_KEY_KP_Down:      { static unsigned char arr[3] = { 40,  80,   0}; return arr; } // VK_DOWN
		case XKB_KEY_Print:        { static unsigned char arr[3] = { 44,  55,   1}; return arr; } // VK_SNAPSHOT
		case XKB_KEY_Insert:       { static unsigned char arr[3] = { 45,  82,   1}; return arr; } // VK_INSERT
		case XKB_KEY_KP_Insert:    { static unsigned char arr[3] = { 45,  82,   0}; return arr; } // VK_INSERT
		case XKB_KEY_Delete:       { static unsigned char arr[3] = { 46,  83,   1}; return arr; } // VK_DELETE
		case XKB_KEY_KP_Delete:    { static unsigned char arr[3] = { 46,  83,   0}; return arr; } // VK_DELETE
		case XKB_KEY_0:            { static unsigned char arr[3] = { 48,  11,   0}; return arr; } // 0
		case XKB_KEY_1:            { static unsigned char arr[3] = { 49,   2,   0}; return arr; } // 1
		case XKB_KEY_2:            { static unsigned char arr[3] = { 50,   3,   0}; return arr; } // 2
		case XKB_KEY_3:            { static unsigned char arr[3] = { 51,   4,   0}; return arr; } // 3
		case XKB_KEY_4:            { static unsigned char arr[3] = { 52,   5,   0}; return arr; } // 4
		case XKB_KEY_5:            { static unsigned char arr[3] = { 53,   6,   0}; return arr; } // 5
		case XKB_KEY_6:            { static unsigned char arr[3] = { 54,   7,   0}; return arr; } // 6
		case XKB_KEY_7:            { static unsigned char arr[3] = { 55,   8,   0}; return arr; } // 7
		case XKB_KEY_8:            { static unsigned char arr[3] = { 56,   9,   0}; return arr; } // 8
		case XKB_KEY_9:            { static unsigned char arr[3] = { 57,  10,   0}; return arr; } // 9
		case XKB_KEY_A:            { static unsigned char arr[3] = { 65, 108,   0}; return arr; } // A
		case XKB_KEY_B:            { static unsigned char arr[3] = { 66, 124,   0}; return arr; } // B
		case XKB_KEY_C:            { static unsigned char arr[3] = { 67, 122,   0}; return arr; } // C
		case XKB_KEY_D:            { static unsigned char arr[3] = { 68, 110,   0}; return arr; } // D
		case XKB_KEY_E:            { static unsigned char arr[3] = { 69,  98,   0}; return arr; } // E
		case XKB_KEY_F:            { static unsigned char arr[3] = { 70, 111,   0}; return arr; } // F
		case XKB_KEY_G:            { static unsigned char arr[3] = { 71, 112,   0}; return arr; } // G
		case XKB_KEY_H:            { static unsigned char arr[3] = { 72, 113,   0}; return arr; } // H
		case XKB_KEY_I:            { static unsigned char arr[3] = { 73, 103,   0}; return arr; } // I
		case XKB_KEY_J:            { static unsigned char arr[3] = { 74, 114,   0}; return arr; } // J
		case XKB_KEY_K:            { static unsigned char arr[3] = { 75, 115,   0}; return arr; } // K
		case XKB_KEY_L:            { static unsigned char arr[3] = { 76, 116,   0}; return arr; } // L
		case XKB_KEY_M:            { static unsigned char arr[3] = { 77, 126,   0}; return arr; } // M
		case XKB_KEY_N:            { static unsigned char arr[3] = { 78, 125,   0}; return arr; } // N
		case XKB_KEY_O:            { static unsigned char arr[3] = { 79, 104,   0}; return arr; } // O
		case XKB_KEY_P:            { static unsigned char arr[3] = { 80, 105,   0}; return arr; } // P
		case XKB_KEY_Q:            { static unsigned char arr[3] = { 81,  96,   0}; return arr; } // Q
		case XKB_KEY_R:            { static unsigned char arr[3] = { 82,  99,   0}; return arr; } // R
		case XKB_KEY_S:            { static unsigned char arr[3] = { 83, 109,   0}; return arr; } // S
		case XKB_KEY_T:            { static unsigned char arr[3] = { 84, 100,   0}; return arr; } // T
		case XKB_KEY_U:            { static unsigned char arr[3] = { 85, 102,   0}; return arr; } // U
		case XKB_KEY_V:            { static unsigned char arr[3] = { 86, 123,   0}; return arr; } // V
		case XKB_KEY_W:            { static unsigned char arr[3] = { 87,  97,   0}; return arr; } // W
		case XKB_KEY_X:            { static unsigned char arr[3] = { 88, 121,   0}; return arr; } // X
		case XKB_KEY_Y:            { static unsigned char arr[3] = { 89, 101,   0}; return arr; } // Y
		case XKB_KEY_Z:            { static unsigned char arr[3] = { 90, 120,   0}; return arr; } // Z
		case XKB_KEY_Super_L:      { static unsigned char arr[3] = { 91,  91,   1}; return arr; } // VK_LWIN
		case XKB_KEY_Super_R:      { static unsigned char arr[3] = { 92,  92,   1}; return arr; } // VK_RWIN
		case XKB_KEY_Menu:         { static unsigned char arr[3] = { 93,  93,   1}; return arr; } // VK_APPS
		case XKB_KEY_KP_0:         { static unsigned char arr[3] = { 96,  82,   0}; return arr; } // VK_NUMPAD0
		case XKB_KEY_KP_1:         { static unsigned char arr[3] = { 97,  79,   0}; return arr; } // VK_NUMPAD1
		case XKB_KEY_KP_2:         { static unsigned char arr[3] = { 98,  80,   0}; return arr; } // VK_NUMPAD2
		case XKB_KEY_KP_3:         { static unsigned char arr[3] = { 99,  81,   0}; return arr; } // VK_NUMPAD3
		case XKB_KEY_KP_4:         { static unsigned char arr[3] = {100,  75,   0}; return arr; } // VK_NUMPAD4
		case XKB_KEY_KP_5:         { static unsigned char arr[3] = {101,  76,   0}; return arr; } // VK_NUMPAD5
		case XKB_KEY_KP_6:         { static unsigned char arr[3] = {102,  77,   0}; return arr; } // VK_NUMPAD6
		case XKB_KEY_KP_7:         { static unsigned char arr[3] = {103,  71,   0}; return arr; } // VK_NUMPAD7
		case XKB_KEY_KP_8:         { static unsigned char arr[3] = {104,  72,   0}; return arr; } // VK_NUMPAD8
		case XKB_KEY_KP_9:         { static unsigned char arr[3] = {105,  73,   0}; return arr; } // VK_NUMPAD9
		case XKB_KEY_KP_Multiply:  { static unsigned char arr[3] = {106,  55,   0}; return arr; } // VK_MULTIPLY
		case XKB_KEY_KP_Add:       { static unsigned char arr[3] = {107,  78,   0}; return arr; } // VK_ADD
		case XKB_KEY_KP_Subtract:  { static unsigned char arr[3] = {109,  74,   0}; return arr; } // VK_SUBTRACT
		case XKB_KEY_KP_Decimal:   { static unsigned char arr[3] = {110,  83,   0}; return arr; } // VK_DECIMAL
		case XKB_KEY_KP_Divide:    { static unsigned char arr[3] = {111,  53,   1}; return arr; } // VK_DIVIDE
		case XKB_KEY_F1:           { static unsigned char arr[3] = {112,  59,   0}; return arr; } // VK_F1
		case XKB_KEY_F2:           { static unsigned char arr[3] = {113,  60,   0}; return arr; } // VK_F2
		case XKB_KEY_F3:           { static unsigned char arr[3] = {114,  61,   0}; return arr; } // VK_F3
		case XKB_KEY_F4:           { static unsigned char arr[3] = {115,  62,   0}; return arr; } // VK_F4
		case XKB_KEY_F5:           { static unsigned char arr[3] = {116,  63,   0}; return arr; } // VK_F5
		case XKB_KEY_F6:           { static unsigned char arr[3] = {117,  64,   0}; return arr; } // VK_F6
		case XKB_KEY_F7:           { static unsigned char arr[3] = {118,  65,   0}; return arr; } // VK_F7
		case XKB_KEY_F8:           { static unsigned char arr[3] = {119,  66,   0}; return arr; } // VK_F8
		case XKB_KEY_F9:           { static unsigned char arr[3] = {120,  67,   0}; return arr; } // VK_F9
		case XKB_KEY_F10:          { static unsigned char arr[3] = {121,  68,   0}; return arr; } // VK_F10
		case XKB_KEY_F11:          { static unsigned char arr[3] = {122,  87,   0}; return arr; } // VK_F11
		case XKB_KEY_F12:          { static unsigned char arr[3] = {123,  88,   0}; return arr; } // VK_F12
		case XKB_KEY_Num_Lock:     { static unsigned char arr[3] = {144,  69,   1}; return arr; } // VK_NUMLOCK
		case XKB_KEY_semicolon:    { static unsigned char arr[3] = {186, 117,   0}; return arr; } // VK_OEM_1
		case XKB_KEY_equal:        { static unsigned char arr[3] = {187,  13,   0}; return arr; } // VK_OEM_PLUS
		case XKB_KEY_comma:        { static unsigned char arr[3] = {188, 127,   0}; return arr; } // VK_OEM_COMMA
		case XKB_KEY_minus:        { static unsigned char arr[3] = {189,  12,   0}; return arr; } // VK_OEM_MINUS
		case XKB_KEY_period:       { static unsigned char arr[3] = {190, 128,   0}; return arr; } // VK_OEM_PERIOD
		case XKB_KEY_slash:        { static unsigned char arr[3] = {191,  53,   0}; return arr; } // VK_OEM_2
		case XKB_KEY_grave:        { static unsigned char arr[3] = {192, 119,   0}; return arr; } // VK_OEM_3
		case XKB_KEY_bracketleft:  { static unsigned char arr[3] = {219, 106,   0}; return arr; } // VK_OEM_4
		case XKB_KEY_backslash:    { static unsigned char arr[3] = {220,  43,   0}; return arr; } // VK_OEM_5
		case XKB_KEY_bracketright: { static unsigned char arr[3] = {221, 107,   0}; return arr; } // VK_OEM_6
		case XKB_KEY_apostrophe:   { static unsigned char arr[3] = {222, 118,   0}; return arr; } // VK_OEM_7
	}

	static unsigned char arr[3] = {0, 0, 0};
	return arr;
}

// Helper function to translate UTF8 char to its integer value
// input:
//   utf8 - pointer to string buffer
// outout:
//   out - integer value of unicode char
// return
//   count of utf8 bytes decoded
int utf8_char_to_ucs2(char *utf8, wchar_t *out)
{
    if (!utf8[0]) { *out = 0; return 0; }
	if(!(utf8[0] & 0x80))              // 0xxxxxxx
		{ *out = (wchar_t)utf8[0]; return 1; }
	else if((utf8[0] & 0xE0) == 0xC0)  // 110xxxxx
		{ *out = (wchar_t)(((utf8[0] & 0x1F) << 6) | (utf8[1] & 0x3F)); return 2; }
	else if((utf8[0] & 0xF0) == 0xE0)  // 1110xxxx
		{ *out = (wchar_t)(((utf8[0] & 0x0F) << 12) | ((utf8[1] & 0x3F) << 6) | (utf8[2] & 0x3F)); return 3; }
	else
		{ *out = 0; return 0; };       // UCS-2 can't handle code points this high; broken input?
}
