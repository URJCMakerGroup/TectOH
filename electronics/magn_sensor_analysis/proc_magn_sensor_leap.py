#!/usr/bin/env python
# coding: utf-8

# ## Magnetic Encoder Analysis
# in this version, instead of going back to see the trend, 
# we just see the previous value, and know that it cannot
# bee a huge leap between samples

import pathlib

import matplotlib.pyplot as plt
import numpy as np
from scipy import signal

# set to True or False if you want to have the results plotted
plot_graph = False
# plot_graph = True

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
#data_filename = "mov_75mmh_1_to_20.bin"

# --- November 2022 good experiments
#data_filename = "exp5kg_25mmh.bin"
#data_filename = "exp5kg_75mmh.bin"
#data_filename = "exp5kg_100mmh.bin"

# cuts:

#data_filename = "exp5kg_25mmh_5mm_0_763s.bin"
#data_filename = "exp5kg_25mmh_10mm_800_2250s.bin"
#data_filename = "exp5kg_25mmh_20mm_2700_5620s.bin"
#data_filename = "exp5kg_25mmh_50mm_5620_12247s.bin"

#data_filename = "exp5kg_75mmh_5mm_0_288s.bin"
#data_filename = "exp5kg_75mmh_10mm_288_875s.bin"
#data_filename = "exp5kg_75mmh_20mm_875_1900s.bin"
#data_filename = "exp5kg_75mmh_50mm_2063_4500s.bin"

#data_filename = "exp5kg_100mmh_5mm_0_255s.bin"
#data_filename = "exp5kg_100mmh_10mm_255_625s.bin"
#data_filename = "exp5kg_100mmh_20mm_663_1400s.bin"
data_filename = "exp5kg_100mmh_50mm_1488_3300s.bin"


if data_filename.startswith("mov"):
    # november experiment wrong experiment
    DIR = "./files_nov/"
elif data_filename.startswith("exp5kg"):
    # november experiment good experiment
    DIR = "./files_nov2/"
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
orig_base_data = [] 
median_data = []
mean_data = []
mean_data_int = []
time = [] # although we are filtering, it is ok to start at zero

primer = 1

# each increment is 2 mm/4096 -> 0.488 um
um_incr = 2/4.096

# for debuging any index
DBG_INDX = 129788*4
DBG = False # set true to print for debug

debug1 = True # to print a debug the first time

first_value_in = False

for index in range(len(data)):
    # example window=9, side_window=4
    #         numdata=20 (0 to 19)
    # when the index is 4 or larger, it has 4 data on the left: 0,1,2,3
    # or when the index is 15 < numdata-side_window)
    
    if index >= side_window and index < numdata-side_window:
        orig_data.append(data[index])
        data_window = data[index-side_window:index+side_window+1]
        if first_value_in == False: # this is the first value to be in
            base = 0
            first_value_in = True
            prev_median = data[index]
        else:
            prev_median = median_data[-1]
            base = 256 * (prev_median // 256)

        adjust_data_window = []
        for datawin_i in data_window:
            if median_data: # if not empty (not the first data to attach
                # check the last data, it should be similar, sometimes
                # there is a 256 error with the previous
                base_datawin_i = base + datawin_i
                if prev_median - base_datawin_i > 100: # there is error with base 
                    base_datawin_i = base + datawin_i + 256
                elif base_datawin_i - prev_median > 100: # there is error with base 
                    base_datawin_i = base + datawin_i - 256
                if abs(base_datawin_i - prev_median) > 100: # check if the modification is right
                    print ('Check Index: ' + str(index)) 
                adjust_data_window.append(base_datawin_i)
            else: # only the first time, not optimized, but just once
                if max(data_window) - min(data_window) > 100: # too much difference
                    # overflow
                    if datawin_i > 215:
                        adjust_data_window.append(datawin_i)
                    elif datawin_i < 50: # small: overflow
                        adjust_data_window.append(datawin_i + 256)
                    else:
                        adjust_data_window.append(datawin_i)
                        print ('check first index, should be an error')
                else:
                    adjust_data_window.append(datawin_i)
            if (max(adjust_data_window) - min(adjust_data_window) > 100) and debug1:
                print('Error, adjust data window should have similar values')
                print('Index: ' + index)
                print('adjust_data_window: ' + str(adjust_data_window))
                debug1 = False
                 
        # we have the window for the median
        median_data.append(int(np.median(adjust_data_window)))
        # introduce the orginal data with its base

        origbase_data_i = data[index] + base
        if median_data: # if not empty (not the first data to attach
            if prev_median - origbase_data_i > 100: # there is error with base 
                origbase_data_i += 256
            elif origbase_data_i - prev_median > 100: # there is error with base 
                origbase_data_i -= 256
            if abs(prev_median - origbase_data_i) > 100: # there is error with base 
                print ('Check Index: ' + str(index)) 
        orig_base_data.append(origbase_data_i)
              
        if index == DBG_INDX and DBG:
            print(median_data[-1])
        #mean_data.append(round(np.mean(adjust_data_window),1))
        # no round
        mean_data.append(np.mean(adjust_data_window))
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

# second median filter
# second mean filter
side2_window = int(side_window/2)
median2_data = []
mean2_data = []
trend = [] 
for index, data_i in enumerate(median_data):
    if index >= side2_window and index < numdata-side2_window:
        mediandata_window = median_data[index-side2_window:index+side2_window+1]
        median2_data.append(int(np.median(mediandata_window)))

        meandata_window = mean_data[index-side2_window:index+side2_window+1]
        mean2_data.append(np.mean(meandata_window))
    else:
        median2_data.append(data_i)
        mean2_data.append(mean_data[index])
      
max_value = 0
# and now to have it in micrometer
for index in range(len(median_data)):
    median_data[index]   = round(median_data[index]   * um_incr,1)
    median2_data[index]  = round(median2_data[index]  * um_incr,1)
    mean_data[index]     = round(mean_data[index]     * um_incr,2)
    mean2_data[index]    = round(mean2_data[index]     * um_incr,2)
    mean_data_int[index] = round(mean_data_int[index] * um_incr,1)
    if median_data[index] > max_value:
        max_value = median_data[index]

max_value = max_value + (20 - max_value % 20)

# ------------- Save to csv

base_filename = pathlib.Path(data_filename).stem
csv_filename = DIR + base_filename + '_lp.csv'
with open(csv_filename, 'w') as csv_file:

    csv_file.write('index,time,median2,median1,mean,mean int,orig_base,original,mean2\n')
    for index in range(len(median_data)):
        csv_file.write(str(index) + ',')
        csv_file.write(str(time[index]) + ',')
        csv_file.write(str(median2_data[index]) + ',')
        csv_file.write(str(median_data[index]) + ',')
        csv_file.write(str(mean_data[index]) + ',')
        csv_file.write(str(mean_data_int[index]) + ',')
        csv_file.write(str(orig_base_data[index]) + ',')
        csv_file.write(str(orig_data[index]) + ',')
        csv_file.write(str(mean2_data[index]) + '\n')

# ------------- draw plots

if plot_graph: # draw the plot
    fig, ax = plt.subplots()
    ax.plot(time, median2_data, linewidth = 3, color='k', label='median 2')
    ax.plot(time, orig_data, linewidth=0.5, color='b', linestyle='dotted', label='original data')
    #ax.plot(time, median_data, color='r', label= 'median 1')
    ax.plot(time, mean_data, color='r', label = 'mean1')
    ax.plot(time, mean2_data, color='g', label = 'mean2')
    ax.plot(time, mean_data_int, color='m', linewidth = 1, linestyle='dotted', label = 'mean integer')
    ax.set_xlim(left=0)
    ax.set_ylim(bottom=0, top=max_value)
    ax.set(yticks=np.arange(0,max_value,20))
    ax.grid(True)
    ax.legend()

    plt.title(base_filename)
    plt.xlabel('Time (ms)')
    plt.ylabel('Position (um)')
    plt.show()

