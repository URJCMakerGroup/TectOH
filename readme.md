# Tectonic Open Hardware (TectOH) Sandbox

This is the second version of the Open Hardware Sandbox. [Link to the first version](https://github.com/URJCMakerGroup/TFG-Cristina-Fernandez) 

Documentation is still under construction, meanwhile check the first version

## License

Hardware License: [CERN-OHL-S 2.0](cern_ohl_s_v2.txt)

Software License: [LGPL 3.0 or later](License.md)

Documentation License: [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/)

![licence summary](imgs/oshw_lic.png)

## User guide

[Sandbox User guide](./userguide) Under construction...

## Firmware
[Arduino code of the Sandbox](./firmware)

## Electronics

### Linear Magnetic Sensor

### VHDL

Displacement rate test with FPGA and linear sensor AS5311
[VHDL code](./electronics/vhdl/as5311_uart)

### Magnetic sensor analysis

Analysis of the linear magnetic sensor
[Python code](./electronics/magn_sensor_analysis)

 - Preprocessing of the raw data from the sensor [Python code](proc_magn_sensor.py)
 - Reducing the size of the processed data [Python code](reduce_dataset.py)
