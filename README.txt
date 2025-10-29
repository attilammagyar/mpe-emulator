MPE Emulator
============

A VST® plugin for enhancing non-MPE MIDI streams with MPE (MIDI Polyphonic
Expression) capabilities based on user-defined mappings and rules, for Linux
and Windows.

MPE Emulator is a MIDI proxy: an intermediary plugin which turns
non-polyphonic, single channel aftertouch (channel pressure), pitch bend, or
any MIDI Control Change (CC) message into polyphonic by applying them
selectively to the lowest, highest, oldest, or newest note, either across the
whole keyboard or restricted to a range of keys based on a configurable
keyboard split point. It can also remap and reshape controller data, and do
various other tricks to increase the expressiveness of a musical performance.

To download MPE Emulator, visit https://attilammagyar.github.io/mpe-emulator ,
or look for the "Releases" section at its GitHub page at
https://github.com/attilammagyar/mpe-emulator .

(The source code is also available on GitHub under the terms of the GNU General
Public License Version 3.)

VST is a registered trademark of Steinberg Media Technologies GmbH.

MPE Emulator Lite
-----------------

MPE Emulator Lite is a platform-independent JS plugin for REAPER
(see https://http://reaper.fm/) v6.74 and above which provides the most
fundamental of MPE Emulator.

System Requirements
-------------------

 * Operating System: Windows 7 or newer, or Linux (e.g. Ubuntu 22.04)
 * CPU: SSE2 support, 32 bit (i686) or 64 bit (x86-64)
 * RAM: around 30 MB per instance

The plugin can also be compiled for RISC-V 64 from its source code.

MPE Emulator Lite runs on any operating system and CPU where REAPER runs. The
required minimum version of REAPER is v6.74.

Dependencies on Windows
-----------------------

Typical Windows systems usually have the MSVC library already installed, but in
case you need it, you can download it from Microsoft's website at
https://learn.microsoft.com/en-US/cpp/windows/latest-supported-vc-redist .

(Most people need the "X64" version of this library. To use the 32 bit version
of the plugin, you will need the "X86" version of the library. See the
Before Installing: Choosing a Distribution section below for more information.)

Dependencies on Linux
---------------------

On Linux, the "libxcb", "libxcb-render", and "libcairo" libraries, and either
the "kdialog" or the "zenity" application are required to run MPE Emulator. To
install them on Debian based distributions (e.g. Ubuntu), you can use the
following command:

    sudo apt-get install libxcb1 libxcb-render0 libcairo2 zenity kdialog

Note: if you want to run the 32 bit version of MPE Emulator on a 64 bit system,
then you will have to install the 32 bit version of the libraries, for example:

    sudo apt-get install libxcb1:i386 libxcb-render0:i386 libcairo2:i386 zenity kdialog

Before Installing: Choosing a Distribution
------------------------------------------

If your plugin host application supports VST 2.4, then you should download and
install the FST version of MPE Emulator.

If your plugin host only supports VST 3, or if you can live with a few quirks
and edge case bugs due to the incomplete MIDI support in the VST 3 protocol,
then you should try the VST 3 bundle on both Windows and Linux.

If you are using an older VST 3 host, or if you are running a 32 bit (also
known as "i686" or "x86") VST 3 host on a 64 bit Linux system, then it might
not be able to load the VST 3 bundle, so you will have to go with a VST 3
single file MPE Emulator package that matches the architecture of your host
application.

The 32 bit versions are usually only needed by those who deliberately use a 32
bit plugin host application, e.g. because they want to keep using some really
old plugins which are not available for 64 bit systems.

If you are in doubt, then try the 64 bit FST version, and if your plugin host
application doesn't recognize it, then try the VST 3 bundle, and if that still
fails to load, then try the 64 bit VST 3 single file version, then the 32 bit
FST version, then the 32 bit VST 3 single file version, and so on.

Packages
~~~~~~~~

These are the file names that you will find on GitHub on the "Releases" page:

 * FST editions for Windows:

    * "mpe-emulator-X_Y_Z-windows-x86_64-sse2-fst.zip": for 64 bit CPUs with
      SSE2 support.

    * "mpe-emulator-X_Y_Z-windows-x86-sse2-fst.zip": for 32 bit CPUs with
      SSE2 support.

 * FST editions for Linux:

    * "mpe-emulator-X_Y_Z-linux-x86_64-sse2-fst.zip": for 64 bit CPUs with SSE2
      support.

    * "mpe-emulator-X_Y_Z-linux-x86-sse2-fst.zip": for 32 bit CPUs with SSE2
      support.

 * VST 3 bundles for both Windows and Linux:

    * "mpe-emulator-X_Y_Z-sse2-vst3_bundle.zip": for 64 or 32 bit CPUs with
      SSE2 support.

 * VST 3 single file editions for Windows:

    * "mpe-emulator-X_Y_Z-windows-x86_64-sse2-vst3_single_file.zip": for
      64 bit CPUs with SSE2 support.

    * "mpe-emulator-X_Y_Z-windows-x86-sse2-vst3_single_file.zip": for
      32 bit CPUs with SSE2 support.

 * VST 3 single file editions for Linux:

    * "mpe-emulator-X_Y_Z-linux-x86_64-sse2-vst3_single_file.zip": for
      64 bit CPUs with SSE2 support.

    * "mpe-emulator-X_Y_Z-linux-x86-sse2-vst3_single_file.zip": for
      32 bit CPUs with SSE2 support.

 * MPE Emulator Lite for any platform that is supported by REAPER:

    * "MPE_Emulator_Lite-vX_Y_Z.jsfx"

 * Source:

    * "mpe-emulator-X_Y_Z-src.zip": the source code of the plugin which you can
      compile for any CPU architecture. (See the "Development" section in the
      documentation.)

Installing the FST (VST 2.4) Version on Windows
-----------------------------------------------

1. Download MPE Emulator from https://attilammagyar.github.io/mpe-emulator
2. Extract the ZIP archive.
3. Copy the "mpe-emulator.dll" file to the folder where you keep your VST 2.4
   plugins.
4. Optionally, if your host application can load ".vstxml" files, it is
   recommended to copy the "mpe-emulator.vstxml" file as well to the folder
   where you keep your VST 2.4 plugins.

Note: VST 2.4 plugins are usually put in the "C:\Program Files\VstPlugins"
folder.

Installing the FST (VST 2.4) Version on Linux
---------------------------------------------

1. Download MPE Emulator from https://attilammagyar.github.io/mpe-emulator
2. Extract the ZIP archive.
3. Copy the "mpe-emulator.so" file to the directory where you keep your VST 2.4
   plugins.
4. Optionally, if your host application can load ".vstxml" files, it is
   recommended to copy the "mpe-emulator.vstxml" file as well to the directory
   where you keep your VST 2.4 plugins.

Note: VST 2.4 plugins are usually put in the "~/.vst" directory.

Installing the VST 3 Bundle on Windows
--------------------------------------

1. Download MPE Emulator from https://attilammagyar.github.io/mpe-emulator
2. Extract the ZIP archive.
3. Copy the entire "mpe-emulator.vst3" folder to your VST 3 folder which is
   usually "C:\Users\YourUserName\AppData\Local\Programs\Common\VST3".

Installing the VST 3 Bundle on Linux
------------------------------------

1. Download MPE Emulator from https://attilammagyar.github.io/mpe-emulator
2. Extract the ZIP archive.
3. Copy the entire "mpe-emulator.vst3" directory to your VST 3 directory which
   is usually "~/.vst3".

Installing the VST 3 Single File Version on Windows
---------------------------------------------------

1. Download MPE Emulator from https://attilammagyar.github.io/mpe-emulator
2. Extract the ZIP archive.
3. Copy the "mpe-emulator.vst3" file to your VST 3 folder which is usually
   "C:\Users\YourUserName\AppData\Local\Programs\Common\VST3".

Installing the VST 3 Single File Version on Linux
-------------------------------------------------

1. Download MPE Emulator from https://attilammagyar.github.io/mpe-emulator
2. Extract the ZIP archive.
3. Copy the "mpe-emulator.vst3" file to your VST 3 directory which is usually
   "~/.vst3".

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
documentation of your one.

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

Open the "README.html" file from the ZIP archive in a browser for a detailed
explanation of each parameter.

Bugs
----

If you find bugs that are not mentioned in the documentation in the
"Known Issues and Quirks" section, please report them at
https://github.com/attilammagyar/mpe-emulator/issues

Documentation
-------------

Open the "README.html" file from the ZIP archive in a browser for more
information.

