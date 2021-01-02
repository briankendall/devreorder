# devreorder
A utility for reordering and hiding controllers for games using DirectInput 8. It's implemented using a wrapper DLL and can be used to change the behavior of a single game or your entire system.

The main use of this tool is to bring some sanity to older DirectInput games that rely on the enumeration order of devices to determine the controller order in game (even though they are not supposed to do that). After Windows XP, and especially starting with Windows 8, the enumeration order of controllers is quite arbitrary and will change after rebooting Windows or unplugging and replugging in your devices. This can wreck havoc on games where you have carefully set up controller bindings for players 1-4, only to find that they are totally ruined the next time you boot up Windows.

There is no supported way to change this order, and the only method I found before writing devreorder was to physically unplug all of your devices and plug them back in the order that you want. This technique, however, falls apart when dealing with wireless and virtual devices. This tool finally allows the order to be defined explicitly.

Tested in Windows 8.1, but should in theory work in any version of Windows. Note that this currently only works for games that use DirectInput 8. Any older games that make use of an earlier version of DirectInput will not be affected, nor will games that use a different API for reading controller input, including Xinput, the old joystick API in winmm.dll, raw input, and the low level Windows HID API. (Although games that use older versions DirectInput, i.e. they use dinput.dll rather than dinput8.dll, may work with devreorder when combined with [dinputto8](https://github.com/elishacloud/dinputto8).)

## How to use:

### Apply to single game or program

In the release zip file, or in the release directory if you cloned the repo, there is an x86 and x64 folder, each containing dinput8.dll. Depending on whether the game is 32-bit or 64-bit copy dinput8.dll in x86 or x64 respectively to the same folder containing the game's .exe file.

Also copy devreorder.ini and edit it so that its `[order]` section contains a list of controllers in the order that you want them to appear. You can also optionally decide to hide certain controllers by either adding all the controllers you want to be hidden to the `[hidden]` section, or adding all the controllers you _don't_ want to be hidden to the `[visible]` section.

Any controllers listed in the `[order]` section will always be sorted ahead of any controller not listed in this section. Controllers with the same name will be grouped together, though their relative other between each other will remain the same. You may also use the *instance GUID* of a particular device in case you need to change the order of devices that have the same name.

When specifying names, you need to have it match exactly as it appears in the Game Controllers control panel or the included DeviceLister program, matching any punctuation, spaces, and capital letters. To open the Game Controllers control panel, type Win+R, type joy.cpl into the dialog box that appears, and then press enter. It will list any controllers that you currently have connected to your system in the order that they will appear to most games that use DirectInput. The DeviceLister application can also be used for this, and allows selecting the text so that it can be copied and pasted, probably making it a better, more convenient option.

You can also use DeviceLister.exe to find the GUIDs of each of your connected devices. You can use a GUID instead of a device name in the `[order]`, `[hidden]`, or `[visible]` sections if you need to specify a specific controller when multiple controllers have the same name. GUIDs may also work better in cases where devreorder doesn't match a controller by name for some reason (such as a bug). The GUID must be enclosed in curly braces and match the format in DeviceLister.exe, e.g. `{01234567-89ab-cdef-0123-456789abcdef}` Unfortunately, when there's more than one device with the same name, DeviceLister currently doesn't have a convenient way of determining which listing corresponds to which physical device. However, the order it uses *should* be the same as in the Game Controllers control panel, and that can display which buttons on a particular device are pressed, so you can use that to help figure out which GUID corresponds with which device. (I hope to improve this situation in the future.)

Please note that while these GUIDs are supposed to remain consistent for any one device, they are specific to a particular Windows installation and are therefore not transferable to another system. Also, people have reported that due to bugs in Windows, the GUIDs might be different between different user accounts. In that case you can specify all of the GUIDs that Windows reports for a particular device to ensure it's sorted or hidden consistently.

Finally, if you install devreorder system-wide, you can disable it for specific applications by adding their executable's filename to the `[ignored processes]` section. Be sure to include the file's extension too (which is usually `.exe`).

**NOTE:** This method of using devreorder will not work for games that initialize DirectInput via the COM interface. If you follow these directions to apply devreorder to a single game and it is not having any effect, it is likely that the game is accessing the DirectInput COM interface. In that case, you will need to follow the directions in the [Apply to your entire system](#apply-to-your-entire-system) section.

**NOTE #2:** If you get an error when running DeviceLister.exe that says something like `System.IO.FileNotFoundException: Could not load file or assembly 'Microsoft.DirectX.DirectInput'` then you probably need to install the DirectX 9 runtime. [Here's a link to Microsoft's installer.](https://www.microsoft.com/en-us/download/details.aspx?id=8109)

### Use one settings file for all games

If you want to have a single devreorder.ini file that applies to all games, copy it to `C:\ProgramData\devreorder\devreorder.ini` (or wherever. your program-data folder is) The wrapper DLL will always check the game's current directory for devreorder.ini and, failing that, then check `C:\ProgramData\devreorder\devreorder.ini` so you can always change the settings on a per-game basis if you prefer.:

### Apply to your entire system

(Warning! The following method involves changing DLL files in your Windows directory. Do not use this method unless you are comfortable making potentially breaking changes to your system and you understand the consequences, including the security implications. Proceed at your own peril!)

If you want to affect the order of controllers for your entire system, that can be accomplished with the following:

1. Create a System Restore Point, just to be safe
2. Fully quit any applications that make use of DirectInput. (They cannot be running in the system tray.)
3. Open your `system32` directory, usually `C:\Windows\system32`
4. Take ownership of the file `dinput8.dll` and change its permissions so that the Administrators group has full control of the file. I'm not going to provide instructions here as you can use your favorite search engine to figure out how to do this. If you're not comfortable taking ownership of a file then you probably shouldn't be tampering with your Windows directory!
5. Rename `dinput8.dll` to `dinput8org.dll`
6. If your Windows installation is 64-bit, copy `x64/dinput8.dll` from the release directory or release zip file into `system32`. If your system is 32-bit, copy `x86/dinput8.dll` into `system32` and skip to step 11.
7. Open your `sysWOW64` directory, usually `C:\Windows\sysWOW64`
8. *If you're using TrackIR,* to prevent it from crashing, copy the original `dinput8.dll` from `sysWOW64` to the TrackIR directory. If you're not using TrackIR then you can ignore this step.
9. Take ownership of `dinput8.dll` and change its permissions so that the Administrators group has full control
10. Rename `dinput8.dll` to `dinput8org.dll`
11. Copy `x86/dinput8.dll` from the release directory or release zip file into `sysWOW64`
12. Copy `devreorder.ini` to a folder named `devreorder` located in your program-data directory, usually `C:\ProgramData\devreorder\devreorder.ini`
13. Edit that copy of `devreorder.ini` as per the instructions in the above sections

*NB: It is extremely important that you rename the original copy of dinput8.dll to dinput8org.dll. devreorder will specifically look for a dll named that in the system32 / sysWOW64 directory, so if you use a different name than dinput8org.dll, then DirectInput will stop working and your game may crash!*

## Antivirus false positives

It's been reported to me that some antivirus software may flag devreorder's versions of dinput8.dll as a virus or other kind of malware. These are false positives. If this happens to you then you will need to configure your antivirus software to whitelist these files so that it doesn't prevent devreorder from working.

## Possible future work

- A GUI that streamlines installing the devreorder DLLs, and allows just dragging controllers into the order you want. (Wouldn't that be nice!)
- Wrap other game input APIs

I make no guarantees that I will ever get around to implementing any of these!

## Some credits

This project makes use of both [MinHook](https://github.com/TsudaKageyu/minhook) by @TsudaKageyu and [SimpleIni](https://github.com/brofield/simpleini) by @brofield.

The technique and a good chunk of source code for the DirectInput wrapper was adapted from the [x360ce](https://github.com/x360ce/x360ce) project. Special thanks to the contributors of that project!
