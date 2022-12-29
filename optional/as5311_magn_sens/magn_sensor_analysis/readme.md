# Tectonic Open Hardware (TectOH) Sandbox

This folder contains the captures taken at different speeds to validate the Tectonic Open Hardware (TectOH) Sandbox.

The captures are detailed as follow:

* capture_10_1.bin: Capture using the AS5311 sensor at 10 mm/h for 1 mm forward (F) and 1 mm backward (B). The order of the captures are F, B, F, B, and F. 
* capture_25_1.bin: Capture using the AS5311 sensor at 25 mm/h for 1 mm forward (F) and 1 mm backward (B). The order of the captures are F, B, F, F, B, B, F, and B. 
* capture_50_1.bin: Capture using the AS5311 sensor at 50 mm/h for 1 mm forward (F) and 1 mm backward (B). The order of the captures are F, B, F, B, F, and B. 
* capture_82_1.bin:Capture using the AS5311 sensor at 82 mm/h for 1 mm forward (F) and 1 mm backward (B). The order of the captures are F, B, F, B, F, and B.
* capture_100_1.bin: Capture using the AS5311 sensor at 100 mm/h for 1 mm forward (F) and backward (B). The order of the captures are F, B, F, B, F, and B. 
	
In order to preproces the data, the python script [proc_magn_sensor.py](./proc_magn_sensor.py) has been created and also shows a graph. As a result, a csv file is generated. To run it, open it and uncomment the line related to the file to analyze. Then run it with this command

```
python proc_magn_sensor.py
```

------------

Nevertheless, the results are already generated and a csv file is available for each capture:


* capture_10_1.csv
* capture_25_1.csv
* capture_50_1.csv
* capture_82_1.csv
* capture_100_1.csv

These files are quite large, so they have been reduced by removing redundant data. The script is [reduce_dataset.py](./reduce_dataset.py)

This script generates the reduced csv files:


* capture_10_1_red.csv
* capture_25_1_red.csv
* capture_50_1_red.csv
* capture_82_1_red.csv
* capture_100_1_red.csv

