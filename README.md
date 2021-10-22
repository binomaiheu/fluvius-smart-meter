# Fluvius smart meter readout project

This is my version of a readout board for the Belgian smart energy meter deployed by Fluvius. The device contains 2 ports which allow the interested hobbyist to read out the real time energy consumption every sectond P1 and the full 4kHz waveform via the S1 port. This repo is for my own education as I shamelessly copied and adapted the work of Matthijs Kooijman and Willem Aandwiel, see links below for the originals !

## Links

Below is a collection of references with the original material upon which I based this project : 

* Decoder : https://github.com/matthijskooijman/arduino-dsmr
* Logger : https://github.com/mrWheel/DSMRlogger2HTTP
* https://mrwheel.github.io/DSMRloggerWS/
* Schematic : https://opencircuit.nl/blog/Slimme-meter-uitlezer

The specs for the smart meter can be found here : 
* https://www.netbeheernederland.nl/_upload/Files/Slimme_meter_15_a727fce1f1.pdf
* https://www.fluvius.be/sites/fluvius/files/2019-07/1901-fluvius-technical-specification-user-ports-digital-meter.pdf
* https://www.fluvius.be/sites/fluvius/files/2019-12/e-mucs_h_ed_1_3.pdf


## Hardware

### P1 port pinout

The Fluvius meter P1 port uses an RJ12 connector with the following pinout : 

| pin | kleur  | functie      |
|-----|------- |--------------|
| 1   | wit    | Power +5 V   |
| 2   | zwart  | Data Request |
| 3   | rood   | Data GND     |
| 4   | groen  | n.c.         |
| 5   | geel   | Data line    |
| 6   | blauw  | Power GND    |


### Circuit board

The circuit in the `kicad/` subfolder is essentially based on the work of Willem Aandewiel (https://opencircuit.nl/blog/Slimme-meter-uitlezer) with some modifications of my own whilst getting to know KiCad. 

I'm using a Lolin D32 ESP board (https://www.wemos.cc/en/latest/d32/index.html) to readout & process the telegrams. For the footprint i installed the following library in KiCad : 

```
https://github.com/pauls-3d-things/kicad-library
```

This contains both the schematic as well as the footprint for the Lolin D32 board. 
