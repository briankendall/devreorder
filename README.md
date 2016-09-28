# devreorder
A utility for reordering and hiding controllers for games using DirectInput 8. It's implemented using a wrapper DLL and can be used to change the behavior of a single game or your entire system.

The main use of this tool is to bring some sanity to older DirectInput games that rely on the enumeration order of devices to determine the controller order in game (even though they are not supposed to do that). After Windows XP, and especially starting with Windows 8, the enumeration order of controllers is quite arbitrary and will change after rebooting Windows or unplugging and replugging in your devices. This can wreck havoc on games where you have carefully set up controller bindings for players 1-4, only to find that they are totally ruined the next time you boot up Windows.

There is no way supported way to change this order, and the only method I found before writing devreorder was to physically unplug all of your devices and plug them back in the order that you want. This technique, however, falls apart when dealing with wireless and virtual devices. This tool finally allows the order to be defined explicitly.

Tested in Windows 8.1, but should in theory work in any version of Windows. Note that this currently only works for games that use DirectInput 8. Any older games that make use of an earlier version of DirectInput will not be affected, nor will games that use a different API for reading controller input, including Xinput, the old joystick API in winmm.dll, raw input, and the low level Windows HID API.

## How to use:

### Apply to single game or program:

In the release zip file, or in the release directory if you cloned the repo, there is an x86 and x64 folder, each containing dinput8.dll. Depending on whether the game is 32-bit or 64-bit copy dinput8.dll in x86 or x64 respectively to the same folder containing the game's .exe file.

Also copy devreorder.ini and edit it so that its `[order]` section contains a list of controllers in the order that you want them to appear, and the `[hidden]` section contains a list of controllers you want to be hidden.

Any controllers listed in the `[order]` section will always be sorted ahead of any controller not listed in this section. Controllers with the same name will be grouped together, though their relative other between each other will remain the same.

You need to type in their name exactly as it appears in the Game Controllers control panel, matching any punctuation, spaces, and capital letters. To open the Game Controllers control panel, type Win+R, type joy.cpl into the dialog box that appears, and then press enter. It will list any controllers that you currently have connected to your system in the order that they will appear to most games that use DirectInput.

Alternatively, if you want to have a single devreorder.ini file that applies to all games, copy it to `C:\ProgramData\devreorder\devreorder.ini` (or wherever. your program-data folder is) The wrapper DLL will always check the game's current directory for devreorder.ini and, failing that, then check `C:\ProgramData\devreorder\devreorder.ini` so you can always change the settings on a per-game basis if you prefer.

### Apply to your entire system

(Warning! The following method involves changing DLL files in your Windows directory. Do not use this method unless you are comfortable making potentially breaking changes to your system and you understand the consequences, including the security implications. Proceed at your own peril!)

If you want to affect the order of controllers for your entire system, that can be accomplished with the following:

0. Create a System Restore Point, just to be safe
1. Fully quit any applications that make use of DirectInput. (They cannot be running in the system tray.)
2. Open your system32 directory, usually `C:\Windows\system32`
3. Take ownership of the file dinput8.dll and change its permissions so that the Administrators group has full control of the file. I'm not going to provide instructions here as you can use your favorite search engine to figure out how to do this. If you're not comfortable taking ownership of a file then you probably shouldn't be tampering with your Windows directory!
4. Rename dinput8.dll to dinput8org.dll
5. Copy x64/dinput8.dll from the release directory or release zip file into system32
6. Open your sysWOW64 directory, usually `C:\Windows\sysWOW64`
7. Take ownership of dinput8.dll and change its permissions so that the Administrators group has full control
8. Rename dinput8.dll to dinput8org.dll
9. Copy x86/dinput8.dll from the release directory or release zip file into system32
10. Copy devreorder.ini to a folder named devreorder located in your program-data directory, usually C:\ProgramData\devreorder
11. Edit devreorder.ini as per the instructions in the above section

## Possible Future work

- A GUI
- Wrap other game input APIs

I make no guarantees that I will ever get around to implementing any of these!

## Some credits

This project makes use of both [MinHook](https://github.com/TsudaKageyu/minhook) by @TsudaKageyu and [SimpleIni](https://github.com/brofield/simpleini) by @brofield.

The technique and a good chunk of source code for the DirectInput wrapper was adapted from the [x360ce](https://github.com/x360ce/x360ce) project. Special thanks to the contributors of that project!
