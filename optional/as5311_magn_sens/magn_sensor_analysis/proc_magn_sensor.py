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
plot_graph = False # better to draw the plot with plot.py
# plot_graph = True

# ------------- Select File by uncommenting

# --- No load experiments
data_filename = "noload_10mmh_1mm.bin"
#data_filename = "noload_25mmh_1mm.bin"
#data_filename = "noload_50mmh_1mm.bin"
#data_filename = "noload_82mmh_1mm.bin"
#data_filename = "noload_100mmh_1mm.bin"

# --- Load experiments
#data_filename = "load5kg_25mmh_5mm.bin"
#data_filename = "load5kg_25mmh_10mm.bin"
#data_filename = "load5kg_25mmh_20mm.bin"

#data_filename = "load5kg_75mmh_20mm.bin"
#data_filename = "load5kg_75mmh_50mm.bin"

#data_filename = "load5kg_100mmh_20mm.bin"
#data_filename = "load5kg_100mmh_50mm.bin"

# I have the experiments in different folders

if data_filename.startswith("noload"):
    DIR = "./exp_noload/"
else: # if data_filename.startswith("load5kg"):
    DIR = "./exp_5kg/"

data_fulfilename = DIR + data_filename

print ('Analisys for ' + data_filename)


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

DIFF = 128

for index in range(len(data)):
    # example window=9, side_window=4
    #         numdata=20 (0 to 19)
    # when the index is 4 or larger, it has 4 data on the left: 0,1,2,3
    # or when the index is 15 < numdata-side_window)
    
    if index >= side_window and index < numdata-side_window:
        data_index = data[index]
        orig_data.append(data_index)
        data_window = data[index-side_window:index+side_window+1]
        if first_value_in == False: # this is the first value to be in
            base = 0
            first_value_in = True
            prev_median = data_index
        else:
            if len(median_data) > side_window:
                window_1stmedian = median_data[-(side_window-1)]
            else: # not enough data yet
                window_1stmedian = median_data[0]
            base = 256 * (window_1stmedian // 256)
        adjust_data_window = []
        for idx, datawin_i in enumerate(data_window):
            if median_data: # if not empty (not the first data to attach
                # check the last data, it should be similar, sometimes
                # there is a 256 error with the previous
                if idx == 0: # take its median, because some times there is a big change
                    # although it is not common
                    base_datawin_i = base + datawin_i
                    if window_1stmedian - base_datawin_i > DIFF: # there is error with base 
                        base = base + 256
                        base_datawin_i = base + datawin_i
                    elif base_datawin_i - window_1stmedian > DIFF: # there is error with base 
                        base = base - 256
                        base_datawin_i = base + datawin_i
                    if abs(base_datawin_i - window_1stmedian) > DIFF: # check if the modification is right
                        print ('Check Index - idx0: ' + str(index) +
                               ' - basedatawin: ' + str(base_datawin_i) +
                               ' - win_1stmedian: '   + str(window_1stmedian) +
                               ' - diff: '   + str(base_datawin_i - window_1stmedian)) 
                    adjust_data_window.append(base_datawin_i)
                    basedatawin_i_prev = base_datawin_i
                else: # the following elements will take the reference of the previous
                    base_datawin_i = base + datawin_i
                    if basedatawin_i_prev - base_datawin_i > DIFF: # there is error with base 
                        base = base + 256
                        base_datawin_i = base + datawin_i
                    elif base_datawin_i - basedatawin_i_prev > DIFF: # there is error with base 
                        base = base - 256
                        base_datawin_i = base + datawin_i
                    if abs(base_datawin_i - basedatawin_i_prev) > DIFF: # check if the modification is right
                        print ('Check index: ' + str(index) +
                               ' - id' + str(idx) +
                               ' - basedatawin: ' + str(base_datawin_i) +
                               ' - prev_datawin: '   + str(base_datawin_i_prev) +
                               ' - diff: '   + str(base_datawin_i - base_datawin_i_prev)) 
                    adjust_data_window.append(base_datawin_i)
                    basedatawin_i_prev = base_datawin_i
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
            max_diff_window = max(adjust_data_window) - min(adjust_data_window)
            if (max_diff_window > DIFF) and debug1:
                print('Check, adjust data window should have similar values')
                print('Index: ' + str(index))
                print('Diff: ' + str(max_diff_window))
                print('adjust_data_window: ' + str(adjust_data_window))
                debug1 = False
                 
        # we have the window for the median
        median_data_i = int(np.median(adjust_data_window))
        median_data.append(median_data_i)
        # calculate its base
        base_median_i  = 256 * (median_data_i // 256)
        # introduce the orginal data with its base
        origbase_data_i = data_index + base_median_i
        if median_data: # if not empty (not the first data to attach
            if prev_median - origbase_data_i > DIFF: # there is error with base 
                origbase_data_i += 256
            elif origbase_data_i - prev_median > DIFF: # there is error with base 
                origbase_data_i -= 256
            if abs(prev_median - origbase_data_i) > DIFF: # there is error with base 
                print ('Check Index: ' + str(index) + 
                       ' - origbase_data_i: ' + str(origbase_data_i) +
                       ' - prev_median: '   + str(prev_median) +
                       ' - diff: '   + str(origbase_data_i - prev_median)) 
        orig_base_data.append(origbase_data_i)
              
        if index == DBG_INDX and DBG:
            print(median_data[-1])
        #mean_data.append(round(np.mean(adjust_data_window),1))
        # no round
        mean_data.append(np.mean(adjust_data_window))
        mean_data_int.append(int(round(np.mean(adjust_data_window),0)))

        # each sample is 0.25 ms
        time.append(0.25 * (index-side_window))
        prev_median = median_data_i # for the next

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
csv_filename = DIR + base_filename + '.csv'
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

# ------------- draw plots, better to draw the plot with plot.py

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

