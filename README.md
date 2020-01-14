# ![Logo](media/logo.png)

This project is an extension of the original code I created for the BBT Tiny Whoop Race Gates. 
I've expanded and simpified the code and schematics for use with a new open-source project.

These gates were (at one time) manufactured under my company, [Bright Blue Tech](https://www.facebook.com/BrightBlueDroneTech/). We have now joined the [Hydra FPV](https://www.hydrafpv.com/) family.

# Table of Contents
1. [Releases](#releases)
2. [Features](#features)
3. [Overview](#overview)
4. [Electronics](#electronics)
5. [Hardware](#Hardware)
6. [Firmware](#firmware)
7. [Stands](#stands)
8. [Power](#power)
9. [Contributing](#contributing)
10. [License](#license)

# Releases
The latest release can be found here. [Latest Releases](insert project here)

# Features
- **Easy to Build** - This design makes use of two 3D printed parts, PEX water pipe, a Arduino NANO board, and some easily ordered parts.
- **Expandable Sizes** - [MultiGP specs for Tiny Whoop race gates](https://drive.google.com/file/d/0BxRbCdCm27o1R1luRWhKbHprbDA/view?usp=sharing) are specific, but these gates can be made in any size you desire.
- **Easily Mountable** - The 1/2" NPT PVC pipe thread built into the bottom of the base allows you to securely attach the gate to any stand, hanger, or wild contraption you can build with PVC pipes and components. Let your imagination run wild... or just use some of my [example stands](#stands).
- **Automatic Changing Patterns** - The gate automatically switches between 14 different lighting patterns (to be expanded)
- **Locked Pattern Mode** - If you prefer to use one pattern, you can lock a gate to that pattern.
- **Random Pattern Mode** - Randomize the patterns to insure your whole course is different.
- **Adjustable Brightness Control** - Different venues require different brightness levels. Or, keep them super bright before the race to attract attention to your race, then back them down for racing.
- **Save Preferences for Next Power On** - This feature will save you time on course setup. If you prefer a specific mode, color, pattern, or brightness, you can lock your gates to those preferences and they will power up in that mode every time. This is also very convienient to use if the gates are mounted in a difficult to access location, like in the rafters or hanging from your ceiling.
- **Erase Saved Preferences** - Just in case you change your mind.

# Overview
Matt Nowakowski and Flite Test have been SUPER GENEROUS and created a video of this gate build on their Tech Channel. 
<p align="center">
    <a href="https://www.youtube.com/watch?v=???????????"><img src="https://img.youtube.com/vi/??????????????/0.jpg"></a>
</p>
To enter specific display modes:

1. **Select Brightness** - Turn the selector to the desired brightness. The gate will progressively show a dial to represent brightness.
2. **Select Pattern** - Press the selector in (button) to select the pattern you like and it will hold that pattern.
3. **Return to Sequence Mode** - Press the button and hold until the first gate segment lights up. Release and you will start progressing through the patterns again, once every 20 seconds.
4. **Random Mode** - Press the button and hold until the second gate segment lights up. It will flash green. This is randomize mode, where the patterns come in random order every 20 seconds.
5. **Save Settings** - Change all the above parameters to your desired setup, then hold the button down until the third segment of the gate lights up. Before the red countdown reaches zero, do the same thing again to confirm. A blue flash will confirm that the settings were saved.
6. **Erase Settings** - Do the same as above, but wait for the fourth segment to light up. All settings will be erased.

# Electronics
# ![Schematic](media/BBTRGPSchematic.jpg)

# Hardware
[Thingiverse Project](https://www.thingiverse.com/thing:4101973)

According to the MultiGP Tiny Whoop racing specs, the maximum area of a gate is 361 square inches or less. That works out to a circumference of about 67" and will result in a nearly 21.5" diameter gate. There is a 1" gap introduced in the 3D base mount, so 66" section of PEX pipe should be perfect. This will fit about 106 LEDs when the strand is flattened and pulled to center. 

# Firmware

# Stands
Portability was a major factor for our Whoop races in Northern Colorado. We had several different venues, one kit, and nowhere to setup a permanant track. Based on this, I built in a 1/2" NPT threaded mount in the bottom of my boxes. This way, I could just purchase PVC pipes and fittings from my local hardware store and create almost any configuration I wanted. AND, it would be portable.

# Power

# Contributing

# License

