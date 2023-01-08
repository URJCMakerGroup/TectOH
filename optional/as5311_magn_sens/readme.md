# Displacement monitoring with FPGA and the AS5311 magnetic linear sensor

The [AS5311 sensor](https://ams.com/en/as5311) is interfaced with an external FPGA board because the Arduino could not handle the Sandbox operation at the same time.
We have used an FPGA board, but a high-performace microcontroller could be used.

## VHDL design for interfacing the AS5311 with an FPGA

[VHDL design for interfacing the high resolution magnetic linear sensor AS5311](./vhdl/.)

The interface with the sensor is explained. Then the FPGA sends the data to a computer.

----

## Magnetic sensor analysis

In this folder we have the [Python scripts](./magn_sensor_analysis) that analyse the data from the magnetic sensor and plots it.

A link to the data from the sensor is also in the folder
