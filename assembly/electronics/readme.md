# Electronic Assembly

## 1. Tools

Since you will need to make your own cables, you will need a crimping tool like these:

- https://www.pololu.com/product/1928
- https://www.electrocomponentes.es/crimpadoras/917-crimpadora-sn-01bm-prensa-terminales-dupont-awg-28-20-crimper-tool.html
- https://solectroshop.com/es/crimpadoras/1822-sn-28b-dupont-terminal-crimpadora.html

There are some tutorials to make custom connectors:

- https://www.instructables.com/Dupont-Crimp-Tool-Tutorial/
- https://youtu.be/GkbOJSvhCgU
- https://youtu.be/K7Qb3DzIX3s


Also you may need a soldering tool and a soldering iron and solder, to solder the cables to the enstops.


## 2. Put the jumpers in the motor driver socket of RAMPS 1.4

RAMPS 1.4: [https://reprap.org/wiki/RAMPS_1.4](https://reprap.org/wiki/RAMPS_1.4)

![RAMPS-jumpers](./ramps_jumpers.png)

## 3. Attach the RAMPS 1.4 to the Arduino board and attach the motor driver to the RAMPS


![Arduino-RAMPS-DRV8825](./arduino_ramps_pololu_sm.png)



Pay attention on the motor driver pinout, do not put it upside-down.
The Pololu DRV8825 and the A4988 have the trimmer in different positions

- Pololu DRV8825: [https://www.pololu.com/product/2133](https://www.pololu.com/product/2133)
- Pololu A4988: [https://www.pololu.com/product/1182](https://www.pololu.com/product/1182)



[Larger image: Arduino-RAMPS-DRV8825](./arduino_ramps_pololu.png)

## 4. Connect the elements as in the figure

If the power suppy is computer ATX, its green cable should be tied to ground (black)

![Electronics schematic](./electronics_sch.png)

[Larger image of electronics schematic](./electronics_sch_big.png)

The section of the [optional components](../../optional/.) has more information on how to connect the optical sensor.


