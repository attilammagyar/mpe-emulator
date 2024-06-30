MPE Emulator
============

A [VST plugin](https://en.wikipedia.org/wiki/Virtual_Studio_Technology) for
enhancing non-MPE MIDI streams with MPE
([MIDI Polyphonic Expression](https://en.wikipedia.org/wiki/MIDI#MIDI_Polyphonic_Expression))
capabilities based on user-defined mappings and rules, for Linux and Windows.

MPE Emulator is a MIDI proxy: an intermediary plugin which turns
non-polyphonic, single channel aftertouch (channel pressure), pitch bend, or
any MIDI Control Change (CC) message into polyphonic by applying them
selectively to the lowest, highest, oldest, or newest note, either across the
whole keyboard or restricted to a range of keys based on a configurable
keyboard split point. It can also remap and reshape controller data, and do
various other tricks to increase the expressiveness of a musical performance.

<img src="https://raw.githubusercontent.com/attilammagyar/mpe-emulator/master/mpe-emulator.png" alt="Screenshot of MPE Emulator" />

To download MPE Emulator, visit its website at
[https://attilammagyar.github.io/mpe-emulator](https://attilammagyar.github.io/mpe-emulator),
or look for the "[Releases](https://github.com/attilammagyar/mpe-emulator/releases)"
section at its GitHub page at
[https://github.com/attilammagyar/mpe-emulator](https://github.com/attilammagyar/mpe-emulator).

(The source code is also available on GitHub under the terms of the GNU General
Public License Version 3.)

See the "[Before Installing: Choosing a Distribution](#install-dist)" section
below to find out which package you need.

> VST® is a trademark of Steinberg Media Technologies GmbH, registered in
> Europe and other countries.

<a id="toc"></a>

Table of Contents
-----------------

 * [Table of Contents](#toc)
 * [Features](#features)
 * [Installation](#install)
    * [Before Installing: Choosing a Distribution](#install-dist)
    * [System Requirements](#system-reqs)
       * [Dependencies on Windows](#windows-deps)
       * [Dependencies on Linux](#linux-deps)
    * [FST (VST 2.4) on Windows](#fst-windows)
    * [FST (VST 2.4) on Linux](#fst-linux)
    * [VST 3 Bundle on Windows](#vst3-bundle-windows)
    * [VST 3 Bundle on Linux](#vst3-bundle-linux)
    * [VST 3 Single File on Windows](#vst3-single-windows)
    * [VST 3 Single File on Linux](#vst3-single-linux)
 * [Usage](#usage)
    * [Knobs](#usage-knobs)
    * [Zone Configuration](#usage-zone)
    * [Rules](#usage-rule)
 * [Bugs](#bugs)
    * [Known Issues and Quirks](#bugs-known)
       * [REAPER 7.17 lets raw MIDI control data pass through the VST 3 version](#bugs-known-reaper-7-17)
       * [Ardour 8.6 ignores Channel Pressure (Aftertouch) events emitted by plugins](#bugs-known-ardour)
       * [FL Studio does not support MPE at all (as of July 2024)](#bugs-known-fl-studio)
       * [VST 3 Note On and CC event ordering issues](#bugs-known-vst3)
       * [The mod wheel or a CC event is not handled with polyphony despite matching one of the rules](#bugs-known-ccpoly)
 * [Frequently Asked Questions](#faq)
    * [Mac version?](#faq-mac)
    * [FL Studio: How to ...?](#faq-flstudio)
    * [Why do you say FST instead of VST 2?](#faq-fst)
 * [Development](#dev)
    * [Tools](#dev-tools)
    * [Dependencies](#dev-dep)
    * [Compiling](#dev-compile)

<a id="features"></a>

Features
--------

 * Retroactively add MPE functionality to non-MPE-capable MIDI input devices.
 * Remap controllers, e.g. turn modulation wheel data into MPE aftertouch and
   vice-versa.
 * Invert MIDI expression and controller data, and apply various other
   distortions.
 * Use the trigger velocity of notes as release velocity for synths that
   utilize it.
 * Split the keyboard and route polyphonic expressions to the two halves with
   different rules.
 * Route various expressions and controllers to the lowest, highest, oldest, or
   newest note globally, or across the split halves of the keyboard.
 * MIDI Learn.

<a href="#toc">Table of Contents</a>

<a id="install"></a>

Installation
------------

<a id="install-dist"></a>

### Before Installing: Choosing a Distribution

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

#### Packages

These are the file names that you will find on GitHub on the "Releases" page:

 * FST editions for Windows:

    * `mpe-emulator-X_Y_Z-windows-x86_64-sse2-fst.zip`: for 64 bit CPUs with
      SSE2 support.

    * `mpe-emulator-X_Y_Z-windows-x86-sse2-fst.zip`: for 32 bit CPUs with
      SSE2 support.

 * FST editions for Linux:

    * `mpe-emulator-X_Y_Z-linux-x86_64-sse2-fst.zip`: for 64 bit CPUs with SSE2
      support.

    * `mpe-emulator-X_Y_Z-linux-x86-sse2-fst.zip`: for 32 bit CPUs with SSE2
      support.

 * VST 3 bundles for both Windows and Linux:

    * `mpe-emulator-X_Y_Z-sse2-vst3_bundle.zip`: for 64 or 32 bit CPUs with
      SSE2 support.

 * VST 3 single file editions for Windows:

    * `mpe-emulator-X_Y_Z-windows-x86_64-sse2-vst3_single_file.zip`: for
      64 bit CPUs with SSE2 support.

    * `mpe-emulator-X_Y_Z-windows-x86-sse2-vst3_single_file.zip`: for
      32 bit CPUs with SSE2 support.

 * VST 3 single file editions for Linux:

    * `mpe-emulator-X_Y_Z-linux-x86_64-sse2-vst3_single_file.zip`: for
      64 bit CPUs with SSE2 support.

    * `mpe-emulator-X_Y_Z-linux-x86-sse2-vst3_single_file.zip`: for
      32 bit CPUs with SSE2 support.

 * Source:

    * `mpe-emulator-X_Y_Z-src.zip`: the source code of the plugin which you can
      compile for any CPU architecture. (See the "Development" section below.)

<a href="#toc">Table of Contents</a>

<a id="system-reqs"></a>

### System Requirements

 * Operating System: Windows 7 or newer, or Linux (e.g. Ubuntu 22.04)
 * CPU: SSE2 support, 32 bit (i686) or 64 bit (x86-64)
 * RAM: around 30 MB per instance

The plugin can also be compiled for RISC-V 64 from its source code.

Tested with [REAPER](https://www.reaper.fm/) 7.17.

<a id="windows-deps"></a>

#### Dependencies on Windows

Typical Windows systems usually have the MSVC library already installed, but in
case you need it, you can download it from
[Microsoft's website](https://learn.microsoft.com/en-US/cpp/windows/latest-supported-vc-redist).

(Most people need the `X64` version of this library. To use the 32 bit version
of the plugin, you will need the `X86` version of the library. See the
[Before Installing: Choosing a Distribution](#install-dist) section below for
more information.)

<a id="linux-deps"></a>

#### Dependencies on Linux

On Linux, the `libxcb`, `libxcb-render`, and `libcairo` libraries, and either
the `kdialog` or the `zenity` application are required to run MPE Emulator. To
install them on Debian based distributions (e.g. Ubuntu), you can use the
following command:

    sudo apt-get install libxcb1 libxcb-render0 libcairo2 zenity kdialog

Note: if you want to run the 32 bit version of MPE Emulator on a 64 bit system,
then you will have to install the 32 bit version of the libraries, for example:

    sudo apt-get install libxcb1:i386 libxcb-render0:i386 libcairo2:i386 zenity kdialog

<a id="fst-windows"></a>

### FST (VST 2.4) on Windows

1. [Download MPE Emulator](https://attilammagyar.github.io/mpe-emulator).
2. Extract the ZIP archive.
3. Copy the `mpe-emulator.dll` file to the folder where you keep your VST 2.4
   plugins.
4. Optionally, if your host application can load `.vstxml` files, it is
   recommended to copy the `mpe-emulator.vstxml` file as well to the folder
   where you keep your VST 2.4 plugins.

Note: VST 2.4 plugins are usually put in the `C:\Program Files\VstPlugins`
folder.

<a id="fst-linux"></a>

### FST (VST 2.4) on Linux

1. [Download MPE Emulator](https://attilammagyar.github.io/mpe-emulator).
2. Extract the ZIP archive.
3. Copy the `mpe-emulator.so` file to the directory where you keep your VST 2.4
   plugins.
4. Optionally, if your host application can load `.vstxml` files, it is
   recommended to copy the `mpe-emulator.vstxml` file as well to the directory
   where you keep your VST 2.4 plugins.

Note: VST 2.4 plugins are usually put in the `~/.vst` directory.

<a id="vst3-bundle-windows"></a>

### VST 3 Bundle on Windows

1. [Download MPE Emulator](https://attilammagyar.github.io/mpe-emulator).
2. Extract the ZIP archive.
3. Copy the entire `mpe-emulator.vst3` folder to your VST 3 folder which is
   usually `C:\Users\YourUserName\AppData\Local\Programs\Common\VST3`.

<a id="vst3-bundle-linux"></a>

### VST 3 Bundle on Linux

1. [Download MPE Emulator](https://attilammagyar.github.io/mpe-emulator).
2. Extract the ZIP archive.
3. Copy the entire `mpe-emulator.vst3` directory to your VST 3 directory which
   is usually `~/.vst3`.

<a id="vst3-single-windows"></a>

### VST 3 Single File on Windows

1. [Download MPE Emulator](https://attilammagyar.github.io/mpe-emulator).
2. Extract the ZIP archive.
3. Copy the `mpe-emulator.vst3` file to your VST 3 folder which is usually
   `C:\Users\YourUserName\AppData\Local\Programs\Common\VST3`.

<a id="vst3-single-linux"></a>

### VST 3 Single File on Linux

1. [Download MPE Emulator](https://attilammagyar.github.io/mpe-emulator).
2. Extract the ZIP archive.
3. Copy the `mpe-emulator.vst3` file to your VST 3 directory which is usually
   `~/.vst3`.

<a href="#toc">Table of Contents</a>

<a id="usage"></a>

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

<a id="usage-knobs"></a>

### Knobs

Most of the parameters that control the settings of the MIDI signal
transformations can be adjusted via virtual knobs on the screen:

 * Move the mouse cursor over a knob, and use the mouse wheel or move the mouse
   while holding down the left mouse button for adjusting the value.

 * Double click on a knob to reset it to its default value.

<a href="#toc">Table of Contents</a>

<a id="usage-zone"></a>

### Zone Configuration

An MPE zone is a group of MIDI channels where one channel is used for sending
global MIDI events and controller changes which apply to all active notes
(manager channel), and the other channels are allocated for individual
polyphonic notes and controller events (member channels). Each controller event
that is sent on a member channel is applied only to the note (or notes) that is
played on that MIDI channel. One of the possible zones is called the "lower"
zone and the other is called "upper". This has nothing to do with note pitches,
these names actually refer to the layout of MIDI channel numbers.

Currently, MPE Emulator uses only one zone which can be either the lower or the
upper zone. The exact details of these are not important, what's important is
that MPE Emulator and the MPE-capable synthesizer must agree on the manager and
the member channel layout. This can be achieved either by manually setting up
the same MPE configuration in both the synthesizer and in MPE Emulator, or
alternatively, MPE Emulator can send an MPE Configuration Message (MCM), and if
the synthesizer implements MPE properly, then it will automatically configure
itself to match the settings in MPE Emulator. (Though in practice, even some
widely used commercial synthesizers ignore MCM messages, so you will likely
have to manually verify that the zone settings of MPE Emulator and the
synthesizer match up.)

#### Import Settings, Export Settings

The two icons at the top left corner of the "Main" configuration section allow
saving and loading settings as ordinary files, e.g. for transferring them
across projects, different host applications, computers, etc.

#### Override Release Velocity With Triggered Velocity (RelVel=TrigVel, Z1ORV)

Most MIDI keyboards emit Note Off events with note velocity set to either 0%,
50%, or 100%, regardless of how slow or fast the corresponding key is released.
However, some synthesizers can utilize note release velocity, for example, to
set the amplitude envelope release time accordingly. MPE Emulator remembers the
note velocity for each triggered note, and can override the release velocity
that is sent by the MIDI device to match the trigger velocity, so that softer
notes are also release softly, and high velocity notes are also released with a
high velocity.

#### Emit MCM on Reset (MCM)

This toggle switch tells MPE Emulator to emit MPE Configuration Messages (MCM),
or don't bother, because the synthesizer would just ignore them anyways.

#### Zone Type (ZONE, Z1TYP)

Select the channel layout used by MPE Emulator:

 * **Lower**: channel 1 is the manager channel, channel 2-16 can be used as
   member channels.

 * **Upper**: channel 16 is the manager channel, channel 1-15 can be used as
   member channels.

#### Channels (CHAN, Z1CHN)

The number of channels to use as member channels. This determines the number of
available polyphonic voices to use (within the limits of the synthesizer).

#### Excess Note Handling (STEAL, Z1ENH)

Tells MPE Emulator what to do then a new note is triggered while all polyphonic
voices are in use:

 * **Never**: there's no voice-stealing, the new note is ignored.

 * **Low**: the lowest note is stopped, and the new note is played on the
   channel where it used to be.

 * **High**: the highest note is stopped, and the new note is played on the
   channel where it used to be.

 * **Old**: the oldest note is stopped, and the new note is played on the
   channel where it used to be.

 * **New**: the newest note is stopped (the one played before the new one), and
   the new note is played on the channel where it used to be.

<a id="usage-zone-anchor"></a>

#### Anchor (ANCH, Z1ANC)

A split point for the keyboard. Expression and controller events can be applied
differently to each side of the split point. For example, one may want to send
channel pressure (aftertouch) events only to the newest or the highest note
above A3, but not to bass notes or chords played with the left hand.

<a href="#toc">Table of Contents</a>

<a id="usage-rule"></a>

### Rules

<a id="usage-rule-in"></a>

#### Input (IN, Z1RxIN)

The input controller (MIDI CC) type for the rule. CC events that are not
matched by any rule's input settings are sent globally on the manager channel,
and those that are matched by one or more rules are handled as the rules
specify. The possible values are:

 * **CC0** - *CC119*: normal MIDI CC numbers that are usually assigned to various
   knobs and faders on a MIDI keyboard. (Note: the mod wheel is *CC1*.)

 * **Pitch**: the pitch wheel.

 * **Ch AT**: channel pressure, also known as channel aftertouch.

 * **Learn**: MIDI learn, which means that the first controller message that is
   received after setting this option will be assigned to the input of the
   rule.

 * **none**: turn off the rule entirely.

<a id="usage-rule-out"></a>

#### Output (OUT, Z1RxOU)

The output controller (MIDI CC) type for the rule. Usually it should be the
same as the [input](#usage-rule-in), but in some cases, you might want to remap
a particular controller to another. For example, if your MIDI keyboard does not
have channel pressure (aftertouch) functionality, but it has some special
function in a particular synthesizer, then you can remap e.g. mod wheel events
to be sent as channel pressure: assign the mod wheel to the input, and select
channel aftertouch for the output, and then whenever you turn the mod wheel,
the synthesizer will receive an aftertouch message instead of it.
(Unfortunately, aftertouch is known to have some
[bugs and quirks in some plugin type and host combinations](#bugs-known).)

<a id="usage-rule-init"></a>

#### Initial Value (INIT, Z1RxIV)

The initial value of the rule (before distortions). See
[Reset](#usage-rule-reset) for the details.

<a id="usage-rule-target"></a>

#### Target (TARGET, Z1RxTR)

Which polyphonic note (channel) to send the output of the rule:

 * **Global**: send on the manager channel, and let the synthesizer apply it to
   all active notes.

 * **All BA**: send to all notes below the [anchor](#usage-zone-anchor).

 * **All AA**: send to all notes above the [anchor](#usage-zone-anchor).

 * **Low**: send on the channel that belongs to the lowest key that is being
   pressed across the entire keyboard.

 * **High**: send on the channel that belongs to the highest key that is being
   pressed across the entire keyboard.

 * **Old**: send to the channel which belongs to the key that has been pressed
   for the longest time across the entire keyboard.

 * **New**: send to the channel which belongs to the key that has been pressed
   for the shortest time across the entire keyboard.

 * **Low BA**: send on the channel that belongs to the lowest key that is being
   pressed below the [anchor](#usage-zone-anchor).

 * **High BA**: send on the channel that belongs to the highest key that is
   being pressed below the [anchor](#usage-zone-anchor).

 * **Old BA**: send to the channel which belongs to the key that has been
   pressed for the longest time below the [anchor](#usage-zone-anchor).

 * **New BA**: send to the channel which belongs to the key that has been
   pressed for the shortest time below the [anchor](#usage-zone-anchor).

 * **Low AA**: send on the channel that belongs to the lowest key that is being
   pressed above the [anchor](#usage-zone-anchor).

 * **High AA**: send on the channel that belongs to the highest key that is
   being pressed above the [anchor](#usage-zone-anchor).

 * **Old AA**: send to the channel which belongs to the key that has been
   pressed for the longest time above the [anchor](#usage-zone-anchor).

 * **New AA**: send to the channel which belongs to the key that has been
   pressed for the shortest time above the [anchor](#usage-zone-anchor).

**Note**: "above the anchor" here includes the anchor key itself.

<a href="usage-rule-dist"></a>

#### Distortion Level (DIST, Z1RxDL)

How much to distort the flat, linear curve of the controller with the selected
[distortion function](#usage-rule-dist-type).

<a id="usage-rule-invert"></a>

#### Invert (Z1RxNV)

Flip the controller: when 0% is received on the input, send 100%, and when the
input is at 100%, then send 0%.

<a id="usage-rule-reset"></a>

#### Reset (Z1RxRS)

Select note initialization and reset behaviour. A reset can occur in one of two
ways:

 * When the [target](#usage-rule-target) of a rule is "Global", then the
   output controller of the rule will be reset when the plugin is
   re-initialized by the host, and when the [zone configuration](#usage-zone)
   is changed.

 * When the target is anything else, then a reset can occur when a new note is
   triggered or an old one is stopped. A reset may affect the new note itself,
   and also the notes that used to be the targets of a rule before the note
   event, or which become the targets of a rule after the note event.

   (For example, when the target of a rule is the highest note and a C4 is
   being played, then a reset occurs when the C5 key is pressed, affecting both
   the C4's channel and the C5's channel, because the former used to be the
   only note and therefore also the highest note, and now the latter is the
   highest. Then if the C5 note is released, then C4 becomes the highest again,
   and it also triggers a reset.)

The options for performing resets are:

 * **INI**: send the [initial value](#usage-rule-init) of the rule to the
   affected notes.

 * **LST**: send the value that was received most recently for this rule to the
   affected notes.

 * **OFF**: no reset is sent to any channels, everything stays as it is until
   a new control event happens.

#### Midpoint (Z1RxMP)

Move the mouse cursor over the first function graph icon at the top right
corner of a rule, and start moving it while holding the left mouse button
down, or start using the mouse wheel to adjust the midpoint of the rule's
[input](#usage-rule-in). For example, if the midpoint is at 75%, then when the
rule's input controller is at 50%, MPE Emulator will send 75%. Moving the
controller between 0% and 50% will make MPE Emulator emit values between 0% and
75%, and moving the controller between 50% and 100% will make MPE Emulator emit
values between 75% and 100%.

Together with the [Invert](#usage-rule-invert) toggle, this makes it possible
to create a pair of rules which turn the two halves of the pitch bend wheel
into different controllers, and to misuse it in various creative ways.

<a id="usage-rule-dist-type"></a>

#### Distortion Type (Z1RxDT)

Click on the second function graph icon at the top right corner of a rule, or
use the mouse wheel while holding the mouse cursor over it to select the
non-linearity shape that the rule will use when its
[distortion level](#usage-rule-dist) parameter is set to a value above 0%.

<a href="#toc">Table of Contents</a>

<a id="bugs"></a>

Bugs
----

If you find bugs that are not mentioned in the
"[Known Issues and Quirks](#bugs-known)" section, please report them at
[https://github.com/attilammagyar/mpe-emulator/issues](https://github.com/attilammagyar/mpe-emulator/issues).

<a href="#toc">Table of Contents</a>

<a id="bugs-known"></a>

### Known Issues and Quirks

<a id="bugs-known-reaper-7-17"></a>

#### REAPER 7.17 lets raw MIDI control data pass through the VST 3 version

When a VST 3 plugin outputs MIDI controller messages in
[REAPER](https://www.reaper.fm/) 7.17 and below versions, they get merged with
the raw MIDI input regardless of whether the "_Replaces MIDI bus_" or the
"_Merges with MIDI bus_" setting is selected. This makes the VST 3 version of
MPE Emulator unusable in REAPER. This is a
[bug in REAPER](https://forum.cockos.com/showthread.php?t=292338) which
might get fixed in 7.18.

The FST (VST 2.4) version of MPE Emulator works fine in all versions of REAPER.

<a href="#toc">Table of Contents</a>

<a id="bugs-known-ardour"></a>

#### Ardour 8.6 ignores Channel Pressure (Aftertouch) events emitted by plugins

There's a bug in both the VST 2 and the VST 3 implementation in
[Ardour](https://ardour.org/) which makes it drop Channel Pressure (Aftertouch)
events (and Program Change events in case of VST 2).
[The status of the fix can be tracked here](https://github.com/Ardour/ardour/pull/899).

<a href="#toc">Table of Contents</a>

<a id="bugs-known-fl-studio"></a>

#### FL Studio does not support MPE at all (as of July 2024)

In fact, MIDI is not a first-class citizen in [FL Studio](https://www.image-line.com/),
and getting even basic MIDI to work with plugins (e.g. using controllers other
than the mod wheel and the pitch bend wheel) requires digging through various
settings dialogs and manually setting up controller links to plugin parameters.
By the way, those MIDI CC helper parameters that many plugin developers and
even the VST 3 standard use are not MIDI CC support, they are just workarounds
to mimic MIDI CC support.

As of July 2024, MPE in FL Studio seems hopeless.

<a href="#toc">Table of Contents</a>

<a id="bugs-known-vst3"></a>

#### VST 3 Note On and CC event ordering issues

A VST 3 plugin has no way to know the exact order of MIDI note events and
controller events on its input, and has no way to communicate it on its output,
because MIDI support is fundamentally broken in the VST 3 protocol.

MPE Emulator does what can be done to work around this, but since the
protocol's design makes it impossible to ensure the sequentiality between note
events and controller events, there might be cases where the VST 3 version of
MPE Emulator cannot fully satisfy the requirements of a synthesizer which
expects CC and note events to come in a particular well defined order. Note
that the FST (VST 2.4) version does not suffer from this problem.

The reason for this is that the VST 3 protocol separates the continuous and
consistent MIDI stream that comes out of MIDI input devices into separate
streams of note-related events and controller events, and passes the latter to
plugins as parameter automation.

While VST 2 has full support for MIDI (and extensions like MPE), the developers
of VST 3 decided to take several steps backward and break MIDI by implementing
controller handling based on a workaround that was used by VST 2 plugin
developers to mimic MIDI CC support for host applications that don't implement
it properly (like, for example, [FL Studio](#bugs-known-fl-studio)). Then when
both plugin developers and MIDI plugin users pushed them, they added another
workaround (named `LegacyMIDICCOutEvent`) to the protocol, which makes it
possible for VST 3 plugins to output MIDI CC events, but now the sequentiality
of MIDI events is broken for VST 3 plugins both on the input and consequently
on the output side.

<a href="#toc">Table of Contents</a>

<a id="bugs-known-ccpoly"></a>

#### The mod wheel or a CC event is not handled with polyphony despite matching one of the rules

Though the MPE specification allows basically any controller event to be sent
on the polyphonic channels (member channels) and to be handled individually for
each note, many commercial synthesizers use only the pitch wheel, the channel
pressure, and CC 74 (a.k.a. Brightness, Timbre, Sound 5, etc.) with polyphony,
because these are the ones that are mandated by the standard. If you set up a
rule in MPE Emulator to make e.g. the mod wheel polyphonic and the synthesizer
ignores it or still applies it globally, then MPE Emulator has no influence on
it, and the synthesizer is doing nothing wrong with respect to the standard.

Most DAW applications have a plugin or a special tool which lets you peek into
the raw MIDI messages that are sent and received by each plugin, bus, and MIDI
track. If you see something there that is clearly wrong on MPE Emulator's side, 
and it's not a [known quirk with a particular host and plugin type combination](#bugs-known)
then please [report it as a bug](https://github.com/attilammagyar/mpe-emulator/issues).

<a href="#toc">Table of Contents</a>

<a id="faq"></a>

Frequently Asked Questions
--------------------------

<a id="faq-mac"></a>

### Mac version?

Sorry, it's not likely to happen anytime soon, unless someone is willing to
create and maintain a Mac fork of MPE Emulator. For me to do it, it would
require quite a large investment, both in terms of effort and financially. If
MacOS would be available (at a reasonable price) for installing it in a virtual
machine that could be used for testing, I'd consider that. But as long as it
cannot be obtained (legally) without also buying a Mac, and I'm happy with my
current computer, I'm not going to invest in a new one.

<a href="#toc">Table of Contents</a>

<a id="faq-flstudio"></a>

### FL Studio: How to ...?

As of July 2024, [MPE in FL Studio is not supported](#bugs-known-fl-studio).

<a href="#toc">Table of Contents</a>

<a id="faq-fst"></a>

### Why do you say FST instead of VST 2?

> VST® is a trademark of Steinberg Media Technologies GmbH, registered in
> Europe and other countries.

I'm not a lawyer, so I have no idea if it would count as trademark infringement
if I claimed VST 2.4 compatibility for MPE Emulator without obtaining a license
from Steinberg Media Technologies GmbH, so I don't do it, especially since they
no longer issue new licenses for VST 2 to anybody. Instead, I use the name of
an open source programming library which makes it possible to build plugins
that can be loaded into VST 2.4 hosts without that license:
[FST](https://git.iem.at/zmoelnig/FST).

<a href="#toc">Table of Contents</a>

<a id="dev"></a>

Development
-----------

This section contains information for those who downloaded the source code of
MPE Emulator and want to compile it themselves.

<a id="dev-tools"></a>

### Tools

#### Linux

 * [GNU Make 4.3+](https://www.gnu.org/software/make/)
 * [G++ 11.4.0+](https://gcc.gnu.org/)
 * [MinGW-w64 10+](https://www.mingw-w64.org/)
 * [Valgrind 3.18.1+](https://valgrind.org/)
 * [Cppcheck 2.7](https://github.com/danmar/cppcheck)
 * [Doxygen 1.9.1+](https://www.doxygen.nl/)

#### Windows

 * [WinLibs MinGW-w64 13.1.0+ (MSVCRT)](https://winlibs.com/)
 * [Doxygen 1.9.6+](https://www.doxygen.nl/)

<a id="dev-dep"></a>

### Dependencies

The `lib/` directory contains code from the following projects:

 * [FST](https://git.iem.at/zmoelnig/FST)
 * [VST 3 SDK](https://github.com/steinbergmedia/vst3sdk)

**Note**: the `lib/` directory does not include the whole SDK packages, it only
contains what's required for compiling MPE Emulator.

#### Linux

To compile MPE Emulator on e.g. Ubuntu Linux 22.04 for all supported platforms,
the following packages need to be installed:

    apt-get install \
        binutils \
        build-essential \
        g++ \
        gcc-multilib \
        g++-multilib \
        libcairo2-dev \
        libcairo2-dev:i386 \
        libx11-dev \
        libx11-dev:i386 \
        libxcb1-dev \
        libxcb1-dev:i386 \
        libxcb-render0-dev \
        libxcb-render0-dev:i386 \
        mingw-w64

<a id="dev-compile"></a>

### Compiling

#### Windows

Assuming that you have installed MinGW-w64 to `C:\mingw64`, you can use the
following commands to run tests and compile MPE Emulator for Windows:

    SET PATH=C:\mingw64\bin;%PATH%
    SET TARGET_PLATFORM=x86_64-w64-mingw32
    SET DEV_OS=windows

    mingw32-make.exe check
    mingw32-make.exe all

#### Linux

Run `make check` for running tests.

The following commands (on a 64 bit Linux environment) will compile MPE
Emulator for 64 bit Windows, 32 bit Windows, `x86_64` Linux, `x86` Linux, and
`RISC-V 64` Linux respectively:

    TARGET_PLATFORM=x86_64-w64-mingw32 make all
    TARGET_PLATFORM=i686-w64-mingw32 make all
    TARGET_PLATFORM=x86_64-gpp make all
    TARGET_PLATFORM=i686-gpp make all
    TARGET_PLATFORM=riscv64-gpp make all

<a href="#toc">Table of Contents</a>
