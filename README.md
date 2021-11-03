# Fluvius smart meter readout project

This is my version of a readout board for the Belgian smart energy meter deployed by Fluvius. The device contains 2 ports which allow the interested hobbyist to read out the real time energy consumption every sectond P1 and the full 4kHz waveform via the S1 port. This repo is for my own education as I shamelessly copied and adapted the work of Matthijs Kooijman and Willem Aandwiel, see links below for the originals !

## Links

Below is a collection of references with the original material upon which I based this **original project** : 
* https://willem.aandewiel.nl/index.php/2018/08/28/slimme-meter-uitlezer/
* https://opencircuit.nl/blog/Slimme-meter-uitlezer 

And a few links on **github** : 

* Decoder : https://github.com/matthijskooijman/arduino-dsmr
* Logger : https://github.com/mrWheel/DSMRlogger2HTTP
* https://mrwheel.github.io/DSMRloggerWS/

You can find a **python decoder** for the serial messages here : 
* https://github.com/jvhaarst/DSMR-P1-telegram-reader

The **specs** for the smart meter can be found here : 
* https://www.netbeheernederland.nl/_upload/Files/Slimme_meter_15_a727fce1f1.pdf
* https://www.fluvius.be/sites/fluvius/files/2019-07/1901-fluvius-technical-specification-user-ports-digital-meter.pdf
* https://www.fluvius.be/sites/fluvius/files/2019-12/e-mucs_h_ed_1_3.pdf


## Hardware

### P1 port pinout

The Fluvius meter P1 port uses an RJ12 connector with the following pinout.

| pin | function     |
|-----|--------------|
| 1   | Power +5 V   |
| 2   | Data Request |
| 3   | Data GND     |
| 4   | n.c.         |
| 5   | Data line    |
| 6   | Power GND    |

The protocol is `serial TTL` between 0 and 5 V, but with *polarity reversed*, meaning that 5V indicates a `0` and 0V indicates a `1`.

### Circuit board

The circuit in the `kicad/` subfolder is essentially based on the work of Willem Aandewiel (https://opencircuit.nl/blog/Slimme-meter-uitlezer) with some modifications of my own whilst getting to know KiCad. 

I'm using a Lolin D32 ESP board (https://www.wemos.cc/en/latest/d32/index.html) to readout & process the telegrams. For the footprint i installed the following library in KiCad : 

* https://github.com/pauls-3d-things/kicad-library

This contains both the schematic as well as the footprint for the Lolin D32 board. 


### Manufacturing

See :
* https://aisler.net  for in the EU manufacturing at reasonable price



### Power

For powering the esp32 I found this post on reddit : https://www.reddit.com/r/esp32/comments/bwgn1q/how_to_power_a_lolin_d32_externally_from_a_dc/, it presents 3 options (copy pasted): 

* *I figured it out with the help of [Dave Bird's Tech Note 097](https://www.youtube.com/watch?v=yZjpYmWVLh8) which explained how the on-board regulator can be disabled (open-circuit) by tying the EN pin to ground. This allows direct application of 3.3V onto the 3V pin, powering the board directly. Additionally, if EN is held low, a USB cable can be connected and serial comms continues to operate, because the CH340C has 3.3V supply. Power from the USB is shorted to ground via 100kohm resistor, which is no big deal.*
* *An alternative is to power at 4.2V via the battery connector. The TP4054 can take up to 7V on its BAT pin, and the charger won't enable unless the voltage drops below 4.2V, so one could also supply, say, 5V via this connector. The D2 diode will protect USB.*
* *One third option is to supply 5V via VBUS pin, ensuring it is as close as possible to the USB voltage (a difference of a few tenths of a volt may not cause any problems). If paranoid, a diode can be used to protect the USB supply.*