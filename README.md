# Fish-Keytar
Arduino code for a fish keytar (MIDI controller).

# Implementation
The keytar is made from a piece of wood, nails, plastic sheets coated in indium tin oxide, two MPR-121 capactive touchsensors, two butotons, wire, hot glue, and an Arduino Leonardo. The wood is painted, and the plastic sheets are cut in the shape of piano keys. Each key is secured to the board with a nail that sticks out the other side. The nails are soldered to wires which are soldered to the capacitive touch sensors. The sensors are wired to the Arduino. The Arduino can be connected to a computer via a mini-USB to USB cable. When powered, the Arduino advertises itself as a MIDI controller. When the keys are touched, their capacitance is changed, and the MPR-121s report that change in capacitance. Each key is associated with a MIDI note that is turned on or off. The two additional buttons are wired to the Arduino. When pressed, they shift the notes represented by the keys up one octave, up two octaves, and down one octave depending on the combination of buttons pressed. Two eye hooks are screwed into the board, which hold an old suitcase strap to allow the keytar to be held easily.
