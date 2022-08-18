# ESP32 Power Meter 
  ![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)
  ![InfluxDB](https://img.shields.io/badge/InfluxDB-22ADF6?style=for-the-badge&logo=InfluxDB&logoColor=white)
  ![Grafana](https://img.shields.io/badge/grafana-%23F46800.svg?style=for-the-badge&logo=grafana&logoColor=white)

  ## Description 
  This project is for my ESP32-powered energy monitor that sends the following parameters to InfluxDB:
  * Ambient Temperature
  * Line-to-Neutral Voltage
  * Current (2 Probes)
  
  :warning: This project deals with 120 volts which can harm you. I take no responsibility of any damage as a result from this project. :warning:

  ## Table of Contents
  * [PCB Design](#pcb)  
  * [Hardware](#hardware)

  ## PCB
  I created a custom PCB for this project using the EasyEDA software, and then had the PCB manufactured through JCLPCB. I decided to go with a two-layer PCB simply because it is the same price as a one-layer, and has the added benefit of easier routing of the traces. Looking at the routing, I suspect it would be possible to make this a one-layer PCB with some careful placement of the resistors underneath the ESP, but that is for someone else to try :laughing:
  
  Something I am going to change in the next revision is making the solder pads for the CT clamps larger, as well as the holes for zip ties. When designing the PCB, it was sort of hard to visualize how big X millimeters is and it just so happened that my calipers ran out of battery. There's nothing wrong with this design, but it just made it cumbersome to solder on. Also the zip tie holes are too close to the connection points, so that needs to be adjusted in the next go-around. Something else I noticed is 120 volts are exposed in the through-hole pins on the bottom of the PCB, which requires some way of insulating if this is going to be installed in an electric panel like I did.
  
  #### Trace Layout: ![alt text](https://github.com/SwadeDotExe/ESP32-Power-Meter/blob/main/Images/TraceLayout.png "PCB Trace Layout")
  #### 3D Rendering: ![alt text](https://github.com/SwadeDotExe/ESP32-Power-Meter/blob/main/Images/PCB%203D%20Rendering.png "PCB 3D Rendering")
  #### Final PCB: ![alt text](https://github.com/SwadeDotExe/ESP32-Power-Meter/blob/main/Images/Final%20PCB.jpg "Final PCB")
  
  ## Hardware 
  This project relies on the ESP32 for reading and sending the data, and a device running [InfluxDB](https://docs.influxdata.com/influxdb/v2.0/install/) and [Grafana](https://grafana.com/docs/grafana/latest/setup-grafana/installation/debian/) for visualizing the data. You could also use something like [HomeBridge](https://github.com/homebridge/homebridge) to display these statistics in Apple HomeKit, which is what I did for the temperature reading. The HomeBridge setup is a little more convoluted because I had to relay the data through MQTT, so that is outside the scope of this project. (Feel free to add a [pull request](https://github.com/SwadeDotExe/ESP32-Power-Meter/pulls)!)
  
  Here are the other components I used:
  | Component        | Quantity      |
  | ---------------- |:-------------:|
  | 47Ω Resistor     | 2             |
  | 330Ω Resistor    | 3             |
  | 1kΩ Resistor     | 1             |
  | 180kΩ Resistor   | 6             |
  | 470Ω Resistor    | 1             |
  | 10uF Capacitor   | 3             |
  | 22uF Capacitor   | 1             |
  | 220uF Capacitor  | 1             |
  | TMP36            | 1             |

