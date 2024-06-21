MPE Emulator
============

MPE is short for [MIDI Polyphonic Expression][mpe], and MPE Emulator is a
[VST plugin][plugin] which enhances non-MPE MIDI data with MPE capabilities
based on user-defined mappings and rules.

For example, it can turn non-polyphonic aftertouch (channel pressure), pitch
bend, or any MIDI Control Change (CC) message into polyphonic by applying it
selectively only to the lowest, highest, oldest, or newest note, and it can
also remap and shape controller data, and do various other tricks to enhance a
musical performance.

  [mpe]: https://en.wikipedia.org/wiki/MIDI#MIDI_Polyphonic_Expression
  [plugin]: https://en.wikipedia.org/wiki/Virtual_Studio_Technology

<img src="https://raw.githubusercontent.com/attilammagyar/mpe-emulator/master/mpe-emulator.png" alt="Screenshot of MPE Emulator" />

The first experimental release of the plugin is expected to arrive by the end
of June 2024 or by the beginning of July 2024. (Until then, you may attempt to
compile it from source.)

> VST® is a trademark of Steinberg Media Technologies GmbH, registered in
> Europe and other countries.
