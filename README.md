# Smart Pot Plant Flaura

## The story
This project is based on [this awesome idea](https://github.com/FlauraPlantPot/Flaura) by Martin McMaker.
Unfortunately, it was working only with Blynk and the pot was hard to waterproof.

I completely redesign the PCB and 3D model.
It is now easier to seal, only 2 sealing points that can be done with hot glue or silicon. Thanks to Martin Vinter for [the inspiration](https://github.com/MartinVinter/Flaura2)

This new smart pot works with HomeAssistant/ESPHome, no need for Blynk. HA code is derived from MathGaming [modified version of Flaura](https://github.com/mathgaming/FlauraHomeAssistant).

This new design has 4 variations for the external shell and can be printed in 2 different colors (container & lid).

## Build your own

### BOM

##### Hoses
| Name | Type | Diameter | Length | Link |
| --- | --- | --- | --- | --- |
| Watering ring hose | PTFE | Ø4-Ø2mm | 35mm | |
| Connector hose ring/T | Silicone | Ø5-Ø3mm | | |
| T-connector | 3.2x2.4mm, 4mm | | |
| Connector hose T/pot | Silicone | Ø5-Ø3mm | | |
| Pump hose | Silicone | Ø5-Ø3mm ou Ø6-Ø4mm ? | | |

PVC hose could work as well, but I advise silicone instead because its is softer and should not break the 3D hose connector easily.

##### Electronics
| Quantity | Name | Specs | Note | Link |
| --- | --- | --- | --- | --- |
| 1 | Lolin D32 |  |  |  |
| 1 | Diaphragm water pump | DC 3.7V, Hose diameters: | 2 visable diaphragms -> suitable for water | |
| 3 |  Resistor | 100kΩ |  |  |
| 1 | Resistor | 330Ω |  |  |
| 1 | Ceramic capacitor | 100nF |  |  |
| 1 | Diode 1N4007 | 1A |  |  |
| 1 | MOSFET IRLZ44N | Logic level | Don't confuse with : IRFZ44N |  |
| 1 | Push button |  |  |  |
| 1 | Power switch |  |  |  |
| 2 | Pin Header Female 16 Pins |  |  |  |
| 2 | Pin Header Male 16 Pins |  |  |  |
| 1 | Pin Header Female 3 Pins |  |  |  |
| 2 | JST Connector Female 2 Pins  | PH Pitch 2.0 |  |  |
| 1 | JST Connector Female 5 Pins  | PH Pitch 2.0 |  |  |
| 2 | JST Connector Male 2 Pins  | PH Pitch 2.0 |  |  |
| 1 | JST Connector Male 5 Pins  | PH Pitch 2.0 |  |  |
|  | M3 screw |  |  |  |
| 1 | 18650 Li-Ion Battery 3.7V with JST Connector | 3.7V, > 2000 mAh, Protection circuitry included | I strongly discourage to buy cheap batteries on AliExpress, buy good batteries = safer and less issues |  |
| 1 | Capacitve Soil Moisture Sensor |  |  |  |

##### PCB

show circuit
show PCB
put link pcb

##### 3D models
Choose one of the following external shell variations:
Container/external shell
Print it
Inner planter
planter helper
battery holder
pump holder


Print settings
Easiest way would to open the 3mf and print it diretly, it's already all configured.
Details:
Print on it's head
7 perimeters
Enable external perimeters
infill 90%
support on enforcer only
15 top solid layer
Seam Rear
0.15mm pour nozzle 0.4 nozzle

###  Instructions 

##### Build instructions

##### Home Assistant & ESPHome instructions

You need to setup Home Assistant and ESPHome
Then just plug your LolinD32 into your computer, add a new device in ESPHome and copy paste the esphome.yaml

### Design choices

Most of the component choices were based on the original project.
But I made some changes/improvements.

##### Resistors
R1, R3 and R4 are just pulldown resistors, so I chose 100kΩ.

R2 is to limit the current drawn by the MOSFET through the GPIO pins.
Lolin D32 max current per GPIO: 12mA
```R = U/Imax = 3.3/0.012 = 275Ω```
So R2 should at least be 275Ω, I chose R2 330Ω. 


##### Pins used
| GPIO | Function |
| --- | --- |
| 5  | Built-in LED |
| 13 | Wakeup deep sleep |
| 19 | Power soil moisture sensor |
| 17 | Power pump |
| 27 | Power water level sensor 100% |
| 26 | Power water level sensor 75% |
| 25 | Power water level sensor 50% |
| 33 | Power water level sensor 25% |
| 32 | Water level sensor signal |
| 34 | Soil moisture sensor signal |
| 35 | Battery voltage |

I avoided the use of strapping pins, on ESP32 they are: 0, 2, 4, 12, 15


### Some Fusion 360 PCB tips

To edit a single component footprint:
- Go on Schematics
- Find component in "Place component" left panel > right click > Edit library
- Find the footprint and double click will open the footprint editor
- Edit as you need and save
- Go back to schematic, right click the schematic component you wanted to update > replace
- A window opens, click on Open library to enable your custom library
- Find your component in the list

This should update the footprint in the PCB editor

___

Check errors before export:
- In schematics > Validate > Errors > Check everything OK then sync
- In PCB editor > Rules DRC/ERC > DRC > Check everything OK

___

To export PCB manufacturing files:
- In PCB editor > Manufacturing > CAM processor > CAM
- Check output files > Profile > Add layer > 46 milling
- Process job
- Send files in "DrillFiles" and "GerberFiles" to manufacturer



