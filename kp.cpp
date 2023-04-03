// https://github.com/unxed/xkb2win
// License: CC0-1.0 license

#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>

#include <X11/Xutil.h>

#include <xkbcommon/xkbcommon.h>
#include "xkb2win.c"

int main()
{

	Display *display; // X11 display
	Window window;    // X11 window
	XEvent event;     // X11 event for keypresses we want to process
	int screen;       // X11 screen

	int cks = 0;      // Value for dwControlKeyState field
	
	// Open connection with the server
	display = XOpenDisplay(NULL);
	if (display == NULL)
	{
		fprintf(stderr, "Cannot open display\n");
		exit(1);
	}

	fprintf(stderr, "\n\nPress ESC to exit\n\n\n");

	// Create window
	screen = DefaultScreen(display);
	window = XCreateSimpleWindow(display, RootWindow(display, screen), 10, 10, 400, 100, 1,
		BlackPixel(display, screen), WhitePixel(display, screen));

	XStoreName(display, window, "XKB to WinKey translation demo");

	// Select kind of events we are interested in
	XSelectInput(display, window, KeyPressMask | KeyReleaseMask);

	// Map (show) the window
	XMapWindow(display, window);

	// Prepare keyboard state object for us keyboard layout.
	// We need X11 keycodes for English keyboard layout, no matter what layout is actually used,
	// to get the corresponding Windows key codes
	xkb_context *ctx = nullptr;
	xkb_keymap *keymap = nullptr;
	xkb_state *state = nullptr;
	ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	struct xkb_rule_names names = {0};
	names.layout = "us";
	keymap = xkb_keymap_new_from_names(ctx, &names, XKB_KEYMAP_COMPILE_NO_FLAGS);
	state = xkb_state_new(keymap);

	// Read keyboard state of physical keyboard to detect initial num lock state
	XKeyboardState x;
	XGetKeyboardControl(display, &x);
	if (x.led_mask & 2) { // If NumLock is on
		// Change our virtual english keyboard state so it has NumLock on also.
		// NumLock key has X11 scan code 77, as defined in
		// /usr/share/X11/xkb/keycodes/xfree86
		xkb_state_update_key(state, 77, XKB_KEY_DOWN); // simulate numlock key down
		xkb_state_update_key(state, 77, XKB_KEY_UP);   // simulate numlock key up
	}

	// Create an input method
	XIM im = XOpenIM(display, NULL, NULL, NULL);

	// Create an input context
	XIC ic = XCreateIC(im, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, window, NULL);

	// Event loop
	while (1)
	{
		XNextEvent(display, &event);

		// Update our virtual keyboard state so it has NumLock state equal to actual physical NumLock state.
		// But do not pass Shift key presses here as we want KeySyms for non-alphabetic char keys
		// to be in lower case for X11-to-WinKey translations.
		// Shift keys have X11 scan codes 50 and 62, as defined in
		// /usr/share/X11/xkb/keycodes/xfree86
		if ((event.xkey.keycode != 50) && (event.xkey.keycode != 62)) {
			xkb_state_update_key(state, event.xkey.keycode, (event.type == KeyPress) ? XKB_KEY_DOWN : XKB_KEY_UP);
		}

		// Get X11 KeySym for the pressed key
		xkb_keysym_t sym = xkb_state_key_get_one_sym(state, event.xkey.keycode);

		// Reset Windows control key state in case modifier key release event is lost (due to window focus lost, etc)
		if (!(event.xkey.state & ShiftMask))
			{ cks &= ~LEFT_SHIFT_PRESSED; cks &= ~RIGHT_SHIFT_PRESSED; cks &= ~SHIFT_PRESSED; }
		if (!(event.xkey.state & ControlMask))
			{ cks &= ~LEFT_CTRL_PRESSED; cks &= ~RIGHT_CTRL_PRESSED; }
		if (!(event.xkey.state & Mod1Mask) && !(event.xkey.state & Mod5Mask)) // sometimes AltGr is mapped to Mod5
			{ cks &= ~LEFT_ALT_PRESSED; cks &= ~RIGHT_ALT_PRESSED; }

		// Update Windows control key state
		if ((sym == XKB_KEY_Shift_L) && (event.type == KeyPress  )) { cks |=  LEFT_SHIFT_PRESSED;  cks |=  SHIFT_PRESSED; }
		if ((sym == XKB_KEY_Shift_L) && (event.type == KeyRelease)) { cks &= ~LEFT_SHIFT_PRESSED;  cks &= ~SHIFT_PRESSED; }
		if ((sym == XKB_KEY_Shift_R) && (event.type == KeyPress  )) { cks |=  RIGHT_SHIFT_PRESSED; cks |=  SHIFT_PRESSED; }
		if ((sym == XKB_KEY_Shift_R) && (event.type == KeyRelease)) { cks &= ~RIGHT_SHIFT_PRESSED; cks &= ~SHIFT_PRESSED; }

		if ((sym == XKB_KEY_Control_L) && (event.type == KeyPress  )) { cks |=  LEFT_CTRL_PRESSED;  }
		if ((sym == XKB_KEY_Control_L) && (event.type == KeyRelease)) { cks &= ~LEFT_CTRL_PRESSED;  }
		if ((sym == XKB_KEY_Control_R) && (event.type == KeyPress  )) { cks |=  RIGHT_CTRL_PRESSED; }
		if ((sym == XKB_KEY_Control_R) && (event.type == KeyRelease)) { cks &= ~RIGHT_CTRL_PRESSED; }

		if ((sym == XKB_KEY_Alt_L) && (event.type == KeyPress  )) { cks |=  LEFT_ALT_PRESSED;  }
		if ((sym == XKB_KEY_Alt_L) && (event.type == KeyRelease)) { cks &= ~LEFT_ALT_PRESSED;  }
		if ((sym == XKB_KEY_Alt_R) && (event.type == KeyPress  )) { cks |=  RIGHT_ALT_PRESSED; }
		if ((sym == XKB_KEY_Alt_R) && (event.type == KeyRelease)) { cks &= ~RIGHT_ALT_PRESSED; }

		// Get X11 KeySym name (it is not needed for translation to win key codes, just for debug output)
		char buf[64];
		xkb_keysym_get_name(sym, buf, 64);

		if (event.type == KeyPress)
			printf ("KeyPress, KeyCode: %i, KeySym: %i %s\n", event.xkey.keycode, sym, buf );
		else if (event.type == KeyRelease)
		{
			printf( "KeyRelease, KeyCode: %i, KeySym: %i %s\n", event.xkey.keycode, sym, buf );
		}

		// Get UTF-8 string corresponding to key event
		int r;
		Status s;
		r = Xutf8LookupString(ic, (XKeyPressedEvent*)&event, buf, sizeof(buf), 0, 0);
		buf[r] = 0;
		printf ("utf8 string from X11: %s\n", buf);

		// Translate X11 KeySym to Windows key codes
		unsigned char* win_key_data = xkb_to_winkey(sym);

		printf ("Windows VirtualKeyCode: %i %c\n",
			win_key_data[0], isalpha(win_key_data[0]) ? win_key_data[0] : ' ');

		// Update Windows control keys state to actual num/caps/scroll lock state
		int cks_current = cks | (win_key_data[2] ? ENHANCED_KEY : 0);
		if (event.xkey.state & LockMask)    cks_current |= CAPSLOCK_ON;
		if (event.xkey.state & Mod2Mask)    cks_current |= NUMLOCK_ON;
		if (event.xkey.state & Mod3Mask)    cks_current |= SCROLLLOCK_ON;

		// Generate win32-input-mode ESC sequence[s]
		// If X11 gives us more than 1 unicode char, we should generate
		// separate sequence for each char
		int numread = 0; // number of utf8 chars parsed
		int offset = 0;  // offset of current utf8 char
		wchar_t ch;      // integet value of current unicode char
		bool first = 1;  // first iteration
		while(1) { 
			numread = utf8_char_to_ucs2(&buf[offset], &ch);
			if (numread || first) {
				// zero read utf8 bytes on first iteration means no unicode value for that key event
				// still esc seq should be displayed
				printf ("ESC sequence as in win32-input-mode: ^[[%i;%i;%i;%i;%i;%i_\n",
					win_key_data[0],                   // VirtualKeyCode
					win_key_data[1],                   // VirtualScanCode
					ch,				                   // Unicode Char as integer value
					(event.type == KeyPress) ? 1 : 0,  // KeyDown or KeyUp flag
					cks_current,                       // dwControlKeyState
					1                                  // RepeatCount
				);
			} else { break; }
			offset += numread;
			first = 0;
		}

		printf ("\n");

		// Exit on ESC key press
		if ( event.xkey.keycode == 0x09 )
			break;
	}

	if (state) { xkb_state_unref(state); }
	if (keymap) { xkb_keymap_unref(keymap); }
	if (ctx) { xkb_context_unref(ctx); }

	// Close connection to server
	if (display) {
		XCloseDisplay(display);
	}

	return 0;
}
