# xkb2win
Library for translating XKB key codes to Windows ones

Today, there are at least four protocols for extended support for keyboard input in terminals (with such features as distinguishing between pressing and releasing a key, obtaining a key code regardless of the keyboard layout, and so on).

1. Windows Terminal's one
https://github.com/microsoft/terminal/blob/main/doc/specs/%234999%20-%20Improved%20keyboard%20handling%20in%20Conpty.md
2. far2l terminal extensions
https://github.com/elfmz/far2l/blob/master/WinPort/FarTTY.h
3. kitty's one
https://sw.kovidgoyal.net/kitty/keyboard-protocol/
4. iTerm2's one
https://gitlab.com/gnachman/iterm2/-/issues/7440#note_129307021

None of them has yet become widespread. Each has its own merits and demerits.

As for April 2023 only for the first two protocols there are terminals that support them available for all operating systems ([far2l](https://github.com/elfmz/far2l/) supports both protocols on Linux/MacOS/BSD platforms; [Windows Terminal](https://github.com/microsoft/terminal) supports first one on Windows platform; [putty4far2l](https://github.com/unxed/putty4far2l) supports second one on Windows platform). Why? Probably because the kovidgoyal's kitty protocol is quite complicated and not so easy to implement, and the iTerm2 protocol is heavily tied to macOS keyboard input and doesn't look like something that will be widely used on other platforms sometime in the future.

We come to the conclusion that one of the first two protocols (win32-input-mode and far2l terminal extensions) has a chance to become the de facto standard. Therefore, it makes sense to simplify their implementation in terminals.

Both of these standards use encoding of data from Windows [KEY_EVENT_RECORD](https://learn.microsoft.com/en-us/windows/console/key-event-record-str) structure to the form of escape sequences. The problem is that this structure uses Windows key codes that are not available on other platforms.

To solve such a problem, I wrote this library. It uses publicly available data (see comments in source code) to provide translation of XKB keycodes into Windows event keycodes. Also included is an example app showing how to properly use this library to process X11 input and generate win32-input-mode escape sequences.

Important note on Virtual Scan Code field. It is keyboard layout dependent, but we always set it as it would be for English keyboard layout. That can be probably be fixed using configuration file so user can override translation table, but I am not sure it is needed at all. Apps should not rely on Virtual Scan Code for char keys anyway as there is no way for app to know what keyboard layout is selected by terminal user (that problem is also noted in win32-input-mode spec). So for getting keyboard layout dependent input UnicodeChar field should be used instead, and for dealing with hot keys in keyboard layout independent mode Virtual Key Code should be used instead. Actually the only use case for Virtual Scan Code that I can see for now is distinguishing between left and right Shift key presses.
