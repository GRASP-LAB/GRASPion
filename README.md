# GRASPion
The GRASPion is an intelligent bristlebot deisgned for research and education purposes based around
the Adafruit QTPy SAMD21, making it extremely easy-to-use through the Arduino environment. It relies
on two vibrating motors embedded in the bot to move around, and thanks to the carefuly design of its
body and legs available here, it can execute controlled trajectories, by moving straight or turning,
making it a new platform for the experimental study of active matter. It achieves higher velocities
(approx. 15 cm/s) and is capable of higher computing power, than previous types of such bots.

<img width="1024" height="620" alt="image" src="https://github.com/user-attachments/assets/87e0c45d-7d9f-4518-a012-564adfe0e1b0" />

The physical properties of the bot and the results of the examples in this repository are given in
our technical paper (see here). The GRASPion, with a printed and fully assembled circuit board, 3D
printed body, battery, motors and legs is available for order through
[graspion.be](https://graspion.be).


## 3D printing
The bot is made out of three 3D printed objects: the chassis, the legs and the wedge plate in
between them. The STL files of the thre can be found in the printing directory.

### The chassis
The chassis (or the body) is printed out of ABS plastic using the Stratasys J35 PolyJet
printer. This allows for a high resolution print, needed in order to properly fit the vibrating
motors and the battery, and giving a hard body to the robot in order to handle eventual collisions.

### The legs
The legs and wedge plate are printed out of PLA, and we find that the Prusament PLA Prusa Galaxy
handles well and gives consistently reproducible results, using the Prusa XL printer, on the
Structural setting, with 15% infill. It is very important that the legs are printed one-by-one, and
not multiple legs at the same time, since this significantly alters layer adhesion and finally
flexibility of the legs. The wedge plate is printed using the same material.


## Arduino setup

We recommend using the legay Arduino IDE (1.8.19). In order to compile and upload the sketch, you
will need to use the following settings:

|    Setting    |     Value     |
| ------------- | ------------- |
|       Board      | Adafruit QT Py M0(SAMD21) ([see here](https://learn.adafruit.com/add-boards-arduino-v164/setup))|
|   Optimization   |  Ofast  |
|     USB Stack    |  TinyUSB  |

The libraries that are needed to sucessfully compile the code are:

|Adafruit Neopixel       | https://github.com/adafruit/Adafruit_NeoPixel          |
|Adafruit DMA neopixel   | https://github.com/adafruit/Adafruit_NeoPixel_ZeroDMA  |
|Adafruit_APDS9960       | https://github.com/adafruit/Adafruit_APDS9960          |
|Adafruit_MLX90393       | https://github.com/adafruit/Adafruit_MLX90393_Library  |
|Adafruit Unified Sensor | https://github.com/adafruit/Adafruit_Sensor            |
|Adafruit_BusIO          | https://github.com/adafruit/Adafruit_BusIO             |
|Arduino-IRremote        | https://github.com/Arduino-IRremote/Arduino-IRremote   |


## Code examples
