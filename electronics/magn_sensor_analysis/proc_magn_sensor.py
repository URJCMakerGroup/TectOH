#!/usr/bin/env python
# coding: utf-8

# ## Magnetic Encoder Analysis

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
#data_filename = "mov_25mmh_10_to_20_cont_50.bin"
#data_filename = "mov_25mmh_1_to_5.bin"
#data_filename = "mov_25mmh_cont_50.bin"
data_filename = "mov_75mmh_1_to_20.bin"

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
window = 61
# take window an odd value, same number of data in each side
if window % 2 == 0:
    window + 1
    
#print(window)

side_window = int(window/2)
#print(side_window) # the pixels at each side

orig_data = [] 
median_data = []
mean_data = []
mean_data_int = []
time = [] # although we are filtering, it is ok to start at zero

primer = 1
base = 0 # will increase or decrease if the data goes above 255 or below 0

# each increment is 2 mm/4096 -> 0.488 um
um_incr = 2000/4096

# distance to go back to see the trend
BACK = 6000

# for debuging any index
DBG_INDX = 129788*4
DBG = False # set true to print for debug


for index in range(len(data)):
    # example window=9, side_window=4
    #         numdata=20 (0 to 19)
    # when the index is 4 or larger, it has 4 data on the left: 0,1,2,3
    # or when the index is 15 < numdata-side_window)
    
    if index >= side_window and index < numdata-side_window:
        data_window = data[index-side_window:index+side_window+1]

        # go back 8000 positions to see the value
        if index > BACK + side_window:
            # now we get the tendency by going back BACK points
            back_index = index - BACK
            base_floor = 256 * int(median_data[back_index]/256) # take the base
        else:
            back_index = side_window
            base_floor = 0
        base_ceil  = base_floor + 256 
        # check if some elements of data_window has gone out of boundaries
        adjust_data_window = []
        if index == DBG_INDX and DBG:
            print(data_window)
        if max(data_window) - min(data_window) > 200:
            if index == DBG_INDX and DBG:
                print(max(data_window))
                print (np.median(data[back_index-side_window:back_index+side_window+1]))
            # it means that it has overflow
            if np.median(data[back_index-side_window:back_index+side_window+1]) > 127:
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

                base = base_ceil # the next base will be ceil
                
            else: # going down
                for datawin_i in data_window:
                    if datawin_i > 127:
                        adjust_data_window.append(base_floor - 256 + datawin_i)
                    else:
                        adjust_data_window.append(base_floor + datawin_i)
                base = base_floor-256 # the next base will be floor
                if index == DBG_INDX and DBG:
                    print(adjust_data_window)
                    print(base_ceil)
                    print(base_floor)
        else: # no overflow
            for datawin_i in data_window:
                if median_data: # if not empty (not the first data to attach
                    # check the last data, it should be similar, sometimes
                    # there is a 256 error with the previous, when the previous
                    # median window has overflow, but not this, but base has 
                    # been changed
                    base_datawin_i = base + datawin_i
                    if median_data[-1] - base_datawin_i > 100: # there is error with base 
                        base = base + 256
                        base_datawin_i = base + datawin_i
                    elif base_datawin_i - median_data[-1] > 100: # there is error with base 
                        base = base - 256
                        base_datawin_i = base + datawin_i
                    adjust_data_window.append(base_datawin_i)
                else:
                    adjust_data_window.append(base + datawin_i)
                 
              
        orig_data.append(data[index])
        median_data.append(int(np.median(adjust_data_window)))
        if index == DBG_INDX and DBG:
            print(median_data[-1])
        mean_data.append(round(np.mean(adjust_data_window),1))
        mean_data_int.append(int(round(np.mean(adjust_data_window),0)))

        # each sample is 0.25 ms
        time.append(0.25 * (index-side_window))

# get the minimum vale of all, so have them at zero
min_val = min([min(median_data), min(mean_data_int), int(min(mean_data))])
for index in range(len(median_data)):
    median_data[index] = median_data[index] - min_val 
    mean_data[index] = mean_data[index] - min_val 
    mean_data_int[index] = mean_data_int[index] - min_val 

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
    if median_data[index] > max_value:
        max_value = median_data[index]

max_value = max_value + (20 - max_value % 20)

# ------------- Save to csv

base_filename = pathlib.Path(data_filename).stem
csv_filename = DIR + base_filename + '.csv'
with open(csv_filename, 'w') as csv_file:

    csv_file.write('index,time,median2,median1,mean,mean int,original\n')
    for index in range(len(median_data)):
        csv_file.write(str(index) + ',')
        csv_file.write(str(time[index]) + ',')
        csv_file.write(str(median2_data[index]) + ',')
        csv_file.write(str(median_data[index]) + ',')
        csv_file.write(str(mean_data[index]) + ',')
        csv_file.write(str(mean_data_int[index]) + ',')
        csv_file.write(str(orig_data[index]) + '\n')

# ------------- draw plots


fig, ax = plt.subplots()
ax.plot(time, median2_data, linewidth = 3, color='k', label='median 2')
ax.plot(time, orig_data, linewidth=0.5, color='b', linestyle='dotted', label='original data')
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

