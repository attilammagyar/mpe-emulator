MPE Emulator
============

A VST® plugin for enhancing ordinary MIDI controllers with MPE (MIDI Polyphonic
Expression) capabilities based on configurable mappings and rules, for Linux,
Windows, and macOS.

MPE Emulator is a MIDI proxy: an intermediary plugin which turns
non-polyphonic, single channel aftertouch (channel pressure), pitch bend, or
any MIDI control change (CC) message into polyphonic by applying them
selectively to the lowest, highest, oldest, or newest note, either across the
whole keyboard or restricted to a range of keys based on a configurable
keyboard split point. It can also remap and reshape controller data, and do
various other tricks to increase the expressiveness of a musical performance.

To download MPE Emulator, visit https://attilammagyar.github.io/mpe-emulator ,
or look for the "Releases" section at its GitHub page at
https://github.com/attilammagyar/mpe-emulator .

See the "Before Installing: Choosing a Distribution" section
below to find out which package you need. TL;DR: if your host application
supports VST 2.4 plugins, then it's most probably the x86_64 FST edition for
Linux and Windows, and the universal FST edition for macOS.

(The source code is also available on GitHub under the terms of the GNU General
Public License Version 3.)

VST is a registered trademark of Steinberg Media Technologies GmbH.

MPE Emulator Lite
-----------------

MPE Emulator Lite is a platform-independent JS plugin for REAPER
(see https://http://reaper.fm/) v6.74 and above which provides the most
fundamental features of MPE Emulator.

Before Installing: Choosing a Distribution
------------------------------------------

If your plugin host application supports VST 2.4, then you should download and
install the FST version of MPE Emulator.

If your plugin host only supports VST 3, or if you can live with a few quirks
and edge case bugs due to the incomplete MIDI support in the VST 3 protocol
itself, then you should try the VST 3 bundle.

The source code distribution can be compiled for various CPU architectures and
operating systems. Ready-to-use binary distributions are available for
Linux and Windows "`x86_64`" and "`x86`" compatible systems (like most desktop
PCs and laptops), and universal binaries are available for macOS.

Choosing a Distribution for Linux or Windows
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If the FST (VST 2.4) version is not an option for you, and you are using an
older VST 3 host, or if you are running a 32 bit (also known as "`i686`" or
"`x86`") VST 3 host on a 64 bit Linux system, then it might not be able to load
the VST 3 bundle, so you will have to go with th VST 3 single file MPE Emulator
package that matches the architecture of your host application.

The 32 bit versions are usually only needed by those who deliberately use a 32
bit plugin host application, e.g. because they want to keep using some really
old plugins which are not available for 64 bit systems.

If you are in doubt, then try the 64 bit FST version, and if your plugin host
application doesn't recognize it, then try the VST 3 bundle, and if that still
fails to load, then try the 64 bit VST 3 single file version, then the 32 bit
FST version, then the 32 bit VST 3 single file version, and so on.

Packages for Linux and Windows
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

These are the file names that you will find on GitHub on the
"Releases" page (https://github.com/attilammagyar/mpe-emulator/releases):

 * FST editions for Windows:

    * "mpe-emulator-X_Y_Z-windows-x86_64-sse2-fst.zip": for 64 bit CPUs with
      SSE2 support.

    * "mpe-emulator-X_Y_Z-windows-x86-sse2-fst.zip": for 32 bit CPUs with SSE2
      support.

 * FST editions for Linux:

    * "mpe-emulator-X_Y_Z-linux-x86_64-sse2-fst.zip": for 64 bit CPUs with SSE2
      support.

    * "mpe-emulator-X_Y_Z-linux-x86-sse2-fst.zip": for 32 bit CPUs with SSE2
      support.

 * VST 3 bundle for both Windows and Linux:

    * "mpe-emulator-X_Y_Z-sse2-vst3_bundle.zip": for 64 or 32 bit CPUs with
      SSE2 support.

 * VST 3 single file editions for Windows:

    * "mpe-emulator-X_Y_Z-windows-x86_64-sse2-vst3_single.zip": for 64 bit CPUs
      with SSE2 support.

    * "mpe-emulator-X_Y_Z-windows-x86-sse2-vst3_single.zip": for 32 bit CPUs
      with SSE2 support.

 * VST 3 single file editions for Linux:

    * "mpe-emulator-X_Y_Z-linux-x86_64-sse2-vst3_single.zip": for 64 bit CPUs
      with SSE2 support.

    * "mpe-emulator-X_Y_Z-linux-x86-sse2-vst3_single.zip": for 32 bit CPUs
      with SSE2 support.

 * MPE Emulator Lite for any platform that is supported by REAPER:

    * "MPE_Emulator_Lite-vX_Y_Z.jsfx"

 * Source code:

    * "mpe-emulator-X_Y_Z-src.zip": the source code of the plugin which you can
      compile for any CPU architecture. (See the "[Development](#dev)" section
      for the details.)

Choosing a Distribution for macOS
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The universal packages are built for macOS 11 Big Sur for M1, M2, etc. and
newer CPUs (arm64) and for macOS 10.12 Sierra for Intel CPUs `x86_64 with
SSE2 instructions, see https://en.wikipedia.org/wiki/SSE2),
and were tested on macOS 26 Tahoe.

Compiling from source code for older macOS versions and different CPUs might be
possible, but it may require some technical knowledge and a few tweaks in the
"Makefile" and "make/macos-gpp.mk".

Packages for macOS
~~~~~~~~~~~~~~~~~~

These are the file names that you will find on GitHub on the
"Releases" page (https://github.com/attilammagyar/mpe-emulator/releases):

 * FST bundle: "mpe-emulator-X_Y_Z-macos-universal-fst_bundle-signed.pkg".

 * VST 3 bundle: "mpe-emulator-X_Y_Z-macos-universal-vst3_bundle-signed.pkg".

 * MPE Emulator Lite for any platform that is supported by REAPER:
   "MPE_Emulator_Lite-vX_Y_Z.jsfx"

 * Source code: "mpe-emulator-X_Y_Z-src.zip".

System Requirements
-------------------

General Information
~~~~~~~~~~~~~~~~~~~

Tested with REAPER 7.14 (https://www.reaper.fm).

RAM: around 30 MB is used per instance, depending on settings like
sample rate, buffer sizes, etc.

MPE Emulator Lite runs on any operating system and CPU where REAPER runs. The
required minimum version of REAPER is v6.74.

Note: the source code can be compiled for RISC-V 64 as well.

Requirements on Linux
~~~~~~~~~~~~~~~~~~~~~

 * CPU: SSE2 support, 32 bit (i686) or 64 bit (x86_64).

Dependencies on Linux
~~~~~~~~~~~~~~~~~~~~~

Required packages:

 * libxcb,
 * libxcb-render,
 * libcairo,
 * either kdialog or zenity.

These are usually already installed on most desktop systems, but to install
them on Debian based distributions (e.g. Ubuntu), you can use the following
command:

    sudo apt-get install libxcb1 libxcb-render0 libcairo2 zenity kdialog

Note: if you want to run the 32 bit version of MPE Emulator on a 64 bit system,
then you will have to install the 32 bit version of the packages, for example:

    sudo apt-get install libxcb1:i386 libxcb-render0:i386 libcairo2:i386 zenity kdialog

Requirements on Windows
~~~~~~~~~~~~~~~~~~~~~~~

 * Operating System: Windows 7 or newer.
 * CPU: SSE2 support, 32 bit (i686) or 64 bit (x86_64).

Dependencies on Windows
~~~~~~~~~~~~~~~~~~~~~~~

Typical Windows systems usually have the MSVC library already installed, but in
case you need it, you can download it from Microsoft's website at
https://learn.microsoft.com/en-US/cpp/windows/latest-supported-vc-redist .

(Most people need the X64 version of this library. To use the 32 bit version
of the plugin, you will need the X86 version of the library. See the
Before Installing: Choosing a Distribution section for more information.)

Requirements on macOS
~~~~~~~~~~~~~~~~~~~~~

 * Operating System:
    * M1, M2, etc. and newer CPUs: macOS 11 Big Sur or newer.
    * Intel CPUs: macOS 10.12 Sierra or newer.
 * CPU: either M1, M2, or newer, or Intel-based x86_64 CPUs.

Installing on Linux
-------------------

FST (VST 2.4)
~~~~~~~~~~~~~

1. Download MPE Emulator from https://attilammagyar.github.io/mpe-emulator .
2. Extract the ZIP archive.
3. Copy the "mpe-emulator.so" file to the directory where you keep your VST 2.4
   plugins.
4. Optionally, if your host application can load ".vstxml" files, it is
   recommended to copy the "mpe-emulator.vstxml" file as well to the directory
   where you keep your VST 2.4 plugins.

Note: VST 2.4 plugins are usually put in the "~/.vst" directory.

VST 3 Bundle
~~~~~~~~~~~~

1. Download MPE Emulator from https://attilammagyar.github.io/mpe-emulator .
2. Extract the ZIP archive.
3. Copy the entire "mpe-emulator.vst3" directory to your VST 3 directory which
   is usually "~/.vst3".

VST 3 Single File
~~~~~~~~~~~~~~~~~

1. Download MPE Emulator from https://attilammagyar.github.io/mpe-emulator .
2. Extract the ZIP archive.
3. Copy the "mpe-emulator.vst3" file to your VST 3 directory which is usually
   "~/.vst3".

Uninstalling
~~~~~~~~~~~~

1. Use a file manager application to locate MPE Emulator in your VST 2.4 or
   VST 3 folder where you installed it.
2. Delete it or drag it to the Trash.

Installing on Windows
---------------------

FST (VST 2.4)
~~~~~~~~~~~~~

1. Download MPE Emulator from https://attilammagyar.github.io/mpe-emulator .
2. Extract the ZIP archive.
3. Copy the "mpe-emulator.dll" file to the folder where you keep your VST 2.4
   plugins.
4. Optionally, if your host application can load ".vstxml" files, it is
   recommended to copy the "mpe-emulator.vstxml" file as well to the folder
   where you keep your VST 2.4 plugins.

Note: VST 2.4 plugins are usually put in the "C:\Program Files\VstPlugins"
folder.

VST 3 Bundle
~~~~~~~~~~~~

1. Download MPE Emulator from https://attilammagyar.github.io/mpe-emulator .
2. Extract the ZIP archive.
3. Copy the entire "mpe-emulator.vst3" folder to your VST 3 folder which is
   usually "C:\Users\YourUserName\AppData\Local\Programs\Common\VST3".

VST 3 Single File
~~~~~~~~~~~~~~~~~

1. Download MPE Emulator from https://attilammagyar.github.io/mpe-emulator .
2. Extract the ZIP archive.
3. Copy the "mpe-emulator.vst3" file to your VST 3 folder which is usually
   "C:\Users\YourUserName\AppData\Local\Programs\Common\VST3".

Uninstalling
~~~~~~~~~~~~

1. Use File Explorer to locate MPE Emulator in your VST 2.4 or VST 3 folder
   where you installed it.
2. Delete it or drag it to the Recycle Bin.

Installing on macOS
-------------------

FST (VST 2.4) Bundle
~~~~~~~~~~~~~~~~~~~~

1. Download MPE Emulator from https://attilammagyar.github.io/mpe-emulator .
2. Locate the downloaded ".pkg" file in Finder and double click on it.
3. Follow the on-screen installation instructions.
4. MPE Emulator will be installed in the "/Library/Audio/Plug-Ins/VST" folder.

VST 3 Bundle
~~~~~~~~~~~~

1. Download MPE Emulator from https://attilammagyar.github.io/mpe-emulator .
2. Locate the downloaded ".pkg" file in Finder and double click on it.
3. Follow the on-screen installation instructions.
4. MPE Emulator will be installed in the "/Library/Audio/Plug-Ins/VST3" folder.

Uninstalling
~~~~~~~~~~~~

1. Use Finder to locate the "mpe-emulator.vst" or "mpe-emulator.vst3" package
   in the "/Library/Audio/Plug-Ins/VST" or "/Library/Audio/Plug-Ins/VST3"
   folder.
2. Drag it to the Trash.

Installing MPE Emulator Lite
----------------------------

1. Download MPE Emulator Lite from https://attilammagyar.github.io/mpe-emulator
2. Copy "MPE_Emulator_Lite-vX_Y_Z.jsfx" to the "Effects" folder of REAPER.

Refer to REAPER's User Guide at https://reaper.fm/userguide.php for more
information on installing JS plugins.

Usage
-----

Place MPE Emulator in the signal chain between the raw MIDI input and a
synthesizer plugin in the plugin host application. The exact steps to set up
MIDI routing vary between different plugin host applications, so consult the
documentation of the one you use.

    +---------------+ single channel +--------------+           +-------------+
    |               | raw MIDI data  |              | MPE data  |             |
    | MIDI keyboard |--------------->| MPE Emulator |---------->| Synthesizer |
    |               |                |              |           |             |
    +---------------+                +--------------+           +-------------+

Most of the parameters that control the settings of the MIDI signal
transformations can be adjusted via virtual knobs on the screen:

 * Move the mouse cursor over a knob, and use the mouse wheel or move the mouse
   while holding down the left mouse button for adjusting the value.

 * Double click on a knob to reset it to its default value.

Open the "README.html" file from the package in a web browser for more
information.

Bugs
----

If you find bugs that are not mentioned in the documentation in the
"Known Issues and Quirks" section, please report them at
https://github.com/attilammagyar/mpe-emulator/issues

Documentation
-------------

Open the "README.html" file from the package in a web browser for more
information.
