#!/usr/bin/env python
# coding: utf-8

# ## Magnetic Encoder Analysis
# when the direction of the motor is forward

import pathlib

import matplotlib.pyplot as plt
import numpy as np
from scipy import signal

# ------------- Select File by uncommenting

# --- September 2022 experiments
#data_filename = "capture_10_1.bin"
#data_filename = "capture_25_1.bin"
#data_filename = "capture_50_1.bin"
#data_filename = "capture_82_1.bin"
#data_filename = "capture_100_1.bin"

# --- November 2022 experiments
#data_filename = "mov_100mmh_1_to_10_fallo.bin"
#data_filename = "mov_100mmh_1_to_20.bin"
data_filename = "mov_25mmh_10_to_20_cont_50.bin"
#data_filename = "mov_25mmh_1_to_5.bin"
#data_filename = "mov_25mmh_cont_50.bin"
#data_filename = "mov_75mmh_1_to_20.bin"

if data_filename.startswith("mov"):
    # november experiment
    DIR = "./files_nov/"
else:
    # september experiment
    DIR = "./files/"

data_fulfilename = DIR + data_filename


with open(data_fulfilename,"rb") as f:
    raw = f.read()

# each data is taken every 250 us
data = []

numdata = len(raw)
print("Number of samples", numdata)

for data_i in range(numdata):
    data.append(raw[data_i])
    
# do the median filter
window = 65 # to have 32 in each side
# take window an odd value, same number of data in each side
if window % 2 == 0:
    window + 1
    
#print(window)

side_window = int(window/2)
#print(side_window) # the pixels at each side

orig_data = [] 
orig_base_data = [] # added the base
median_data = []
mean_data = []
mean_data_int = []
time = [] # although we are filtering, it is ok to start at zero

primer = 1
base = 0 # will increase or decrease if the data goes above 255 or below 0

# each increment is 2 mm/4096 -> 0.488 um
um_incr = 2000/4096


# for debuging any index
DBG_INDX = 129788*4
DBG = False # set true to print for debug


# counts the number of overflows of the counting
base_floor = 0
base_ceil = 256
# the overflow will increment when found the first 240 (half)
# and will not increment again until a 100 if found
ovf_possible = False

for index in range(side_window, len(data)-side_window):
    # example window=9, side_window=4
    #         numdata=20 (0 to 19)
    # when the index is 4 or larger, it has 4 data on the left: 0,1,2,3
    # or when the index is 15 < numdata-side_window)
    
    data_window = data[index-side_window:index+side_window+1]

    adjust_data_window = []
    if index == DBG_INDX and DBG:
        print(data_window)
    # check if some elements of data_window has gone out of boundaries
    if data[index] >= 48 and data[index] < 208: # means that we are done with posibility of the overflow
        # number are approximate, but the jitter should be that large (even less) until the next
        if ovf_possible == True: # first time, increment the base
            base_floor += 256
            base_ceil  = base_floor + 256
        ovf_possible = False # to do the change of base only once
 
    if max(data_window) - min(data_window) > 200:
        ovf_possible = True # to allow for the next base change
        if index == DBG_INDX and DBG:
            print(max(data_window))

        # we are going up
        for datawin_i in data_window:
            if datawin_i < 127:
                adjust_data_window.append(base_ceil + datawin_i)
            else:
                adjust_data_window.append(base_floor + datawin_i)
            if primer == 1 and DBG:
                print(datawin_i)
                print(base_ceil)
                print(base_floor)
                primer = 0
            if index == DBG_INDX and DBG:
                print(adjust_data_window)
                print(base_ceil)
                print(base_floor)
        if data[index] < 127:
            orig_base_data.append(base_ceil + data[index])
        else:
            orig_base_data.append(base_floor + data[index])
    else: # we are not the the edge
        if ovf_possible == False: # we are in safe zone
            for datawin_i in data_window:
                adjust_data_window.append(base_floor + datawin_i)
            orig_base_data.append(base_floor + data[index])
        else: # we could be
            for datawin_i in data_window:
                if datawin_i < 127:
                    adjust_data_window.append(base_ceil + datawin_i)
                else:
                    adjust_data_window.append(base_floor + datawin_i)
            if data[index] < 127:
                orig_base_data.append(base_ceil + data[index])
            else:
                orig_base_data.append(base_floor + data[index])
                 
              
    orig_data.append(data[index])
    median_data.append(int(np.median(adjust_data_window)))
    if index == DBG_INDX and DBG:
        print(median_data[-1])
    mean_data.append(round(np.mean(adjust_data_window),1))
    mean_data_int.append(int(round(np.mean(adjust_data_window),0)))

    # each sample is 0.25 ms
    time.append(0.25 * (index-side_window))

# get the minimum vale of all, so have them at zero
min_val = min([min(median_data), min(mean_data_int), int(min(mean_data)), min(orig_base_data)])
for index in range(len(median_data)):
    median_data[index] = median_data[index] - min_val 
    mean_data[index] = mean_data[index] - min_val 
    mean_data_int[index] = mean_data_int[index] - min_val 
    orig_base_data[index] = orig_base_data[index] - min_val 

# second mean filter
side2_window = int(side_window/2)
median2_data = []
trend = [] 
for index, data_i in enumerate(median_data):
    if index >= side2_window and index < numdata-side2_window:
        data_window = median_data[index-side2_window:index+side2_window+1]
        median2_data.append(int(np.median(data_window)))
    else:
        median2_data.append(data_i)
      
max_value = 0
# and now to have it in micrometer
for index in range(len(median_data)):
    median_data[index]   = round(median_data[index]   * um_incr,1)
    median2_data[index]  = round(median2_data[index]  * um_incr,1)
    mean_data[index]     = round(mean_data[index]     * um_incr,1)
    mean_data_int[index] = round(mean_data_int[index] * um_incr,1)
    orig_base_data[index] = round(orig_base_data[index] * um_incr,1)
    if median_data[index] > max_value:
        max_value = median_data[index]

max_value = max_value + (20 - max_value % 20)

# ------------- Save to csv

base_filename = pathlib.Path(data_filename).stem
csv_filename = DIR + 'fwd' + base_filename + '.csv'
with open(csv_filename, 'w') as csv_file:

    csv_file.write('index,time,median2,median1,mean,mean int,orig_base,original\n')
    for index in range(len(median_data)):
        csv_file.write(str(index) + ',')
        csv_file.write(str(time[index]) + ',')
        csv_file.write(str(median2_data[index]) + ',')
        csv_file.write(str(median_data[index]) + ',')
        csv_file.write(str(mean_data[index]) + ',')
        csv_file.write(str(mean_data_int[index]) + ',')
        csv_file.write(str(orig_base_data[index]) + ',')
        csv_file.write(str(orig_data[index]) + '\n')

# ------------- draw plots


fig, ax = plt.subplots()
ax.plot(time, median2_data, linewidth = 3, color='k', label='median 2')
ax.plot(time, orig_data, linewidth=1, color='b', linestyle='dotted', label='original base')
ax.plot(time, median_data, color='r', label= 'median 1')
ax.plot(time, mean_data, color='g', label = 'mean')
#ax.plot(time, mean_data_int, color='m', linewidth = 1, linestyle='dotted', label = 'mean integer')
ax.set_xlim(left=0)
ax.set_ylim(bottom=0, top=max_value)
ax.set(yticks=np.arange(0,max_value,20))
ax.grid(True)
ax.legend()
plt.xlabel('Time (ms)')
plt.ylabel('Position (um)')
plt.show()

