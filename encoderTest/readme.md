# Tectonic Open Hardware (TectOH) Sandbox

This folder contains the captures taken at different speeds to validate the Tectonic Open Hardware (TectOH) Sandbox.

The captures are detailed as follow:

* capture_10_1.bin: Capture using the AS5311 sensor at 10 mm/h for 1 mm forward (F) and 1 mm backward (B). The order of the captures are F, B, F, B, and F. 
* capture_25_1.bin: Capture using the AS5311 sensor at 25 mm/h for 1 mm forward (F) and 1 mm backward (B). The order of the captures are F, B, F, F, B, B, F, and B. 
* capture_50_1.bin: Capture using the AS5311 sensor at 50 mm/h for 1 mm forward (F) and 1 mm backward (B). The order of the captures are F, B, F, B, F, and B. 
* capture_82_1.bin:Capture using the AS5311 sensor at 82 mm/h for 1 mm forward (F) and 1 mm backward (B). The order of the captures are F, B, F, B, F, and B.
* capture_100_1.bin: Capture using the AS5311 sensor at 100 mm/h for 1 mm forward (F) and backward (B). The order of the captures are F, B, F, B, F, and B. 
	
In order to visualize the data, a python script has been created to visualize the captures (showResults.py). You can visualize it with the following command:
```
python showResults.py capture_50_1.bin
```

### ToDo list: 
* Improve the visualization of the data by unwrapping.
