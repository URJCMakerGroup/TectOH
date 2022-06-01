# VHDL

## as5311_uart

VHDL code to test the displacement rate of the Sandbox.
It controls the [AS5411 linear position  sensor](https://ams.com/en/as5311) and sends the position data to the UART (via USB).

The linear sensor has a magentic strip with 2mm pole pair and sends the absolute position within this 2mm in 12 bits -> Resolution 2mm/4096 = 0.488 nm

The linear sensor sends the data via SSI to the FPGA. Each SSI message has 18 bits, which 12 bits corresponds to the positional data. There are other information bits in the message.

The FPGA receives this 12 bit positional data to the UART.

AS5311 has a sampling rate about 10kHz. However, due to the slow speeds of the Sandbox, we don't need sampling at such higher rates.

The SSI clock is provided by the FPGA. The maximum frequency is 1MHz (T=1us). Since the SSI sends 18 bits, and there are some other timming constraints, aproximately, the message frequency is around 20 times lower. The whole message has a frequency around 50kHz (20 us per message). But it is 12 bit message.

Since we don't need such high rates due to the low speed of the Sandbox, we use a frequency of 100kHz (T=10us). So it is safer to send through the cables. This leads to a message frequency of 5kHz (200 us per message).

We even lower the *message frequency to 4kHz* (250 us per message). Therefore, **each sample is taken every 250 us**. 4000 samples per second.

The Sanbox has a 3mm lead, a 200 pole stepper motor, a 50.9 gear reduction, and the motor moves using halfsteps (HS). This makes that the gantry advances 147 nm per halfstep.

Since the linear sensor resolution is 488 nm, we need to move 3.31 halfsteps for the sensor to notice a change in position. Therefore, the sensor is not capable of sensing just one halfstep.

When the motor is not slewing, and using microstepping, the motor takes around **6.5 ms per halfstep**. 
Therefore, since we are sampling every 250 us, we would take around 25 samples for each halfstep. Since the distance resolution of the sensor is three times larger that the distance of a halfstep. These 25 samples should be of the same value, or at most would vary in just one unit (of the 12 bits of the position).

Given these slow speeds relative to the sensor resolution, at 6.5 ms per halfstep, the sensor will notice a change every 85.75 samples.

Since the UART sends 8 bits, we are going to just send the 8-lsb, and the computer will easily make the calculations. Since from each of the sendings, the variation would be one unit at most.

The UART sends the 8-bit position at 115200 bauds, which is 11520bytes per second. That is, the message frequency 11.52 kHz (11.52 kB/s).
Since this frequency is higher than the 4kHz message frequency of the linear sensor. We can just directly send the position value received from the sensor without the need of using a buffer. We could even send to messages for the 4 MSB that are not being send. But there is no need to do that, because the changes in position are of one unit at most. So the computer can easily figure out the position.

The data can be received by a terminal such as RealTerm. Setting the baudrate to 115200. Selecting the adequate port and saving the data received in a file (in Capture tab).

---

The VHDL design includes modules to control a motor, to move it while doing the test. However, we don't need it, since the motor is controlled by the Arduino. We have left it in case it is useful in the future.

### Inputs and Outputs

- btn_reset: is the reset
- btnc (central): initialize the millimiter count
- sw(0): determines if we are requesting the AS5133 the position
or the magnetic field. **Should be ON (HIGH '1') to request the position**
- sw(1): motor enable. OFF (LOW '0') is active

Leds show the status bits of the AS5311
- led(0) : parity OK
- led(1) : Mag DEC 
- led(2) : Mag INC
- led(3) : LIN
- led(4) : COF
- led(5) : OC

I/O not used in this test, this is when the motor is controlled by the FPGA
- btn_r (right): a step forward
- btn_l (left): a step back
- btn_u (up): 16 steps forward
- btn_d (down): 16 steps back


