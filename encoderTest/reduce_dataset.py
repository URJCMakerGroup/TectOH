#!/usr/bin/env python
# coding: utf-8

# Magnetic Encoder reduce results
# Most of part of the data is redundant and takes a lot time to show
# this script reduces its size

import pathlib
import csv

import matplotlib.pyplot as plt
import numpy as np
from scipy import signal

# directory where the received binary files are
DIR = "./files/"


# ### Select File by uncommenting

csv_filename = "capture_10_1"
#csv_filename = "capture_25_1"
#csv_filename = "capture_50_1"
#csv_filename = "capture_82_1"
#csv_filename = "capture_100_1"

csv_fulfilename = DIR + csv_filename + '.csv'

# this is the order
# index time median2 median1 mean mean_int orig_data

time      = []
median2   = []
median1   = []
mean      = []
mean_int  = []
orig_data = []

distance_list = []

with open(csv_fulfilename,"r") as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    first_row = True
    before_anychange = True
    for csv_row in csv_reader:
        if first_row: # first row has the header
            first_row = False
        else:
            time.append(float(csv_row[1])) # 0 is the index
            median2_val = float(csv_row[2])
            median2.append(median2_val)
            median1.append(float(csv_row[3]))
            mean.append(float(csv_row[4]))
            mean_int.append(float(csv_row[5]))
            orig_data.append(int(csv_row[6]))
            index = int(csv_row[0])
            if index == 0:
                prev_index = 0
                prev_value = median2_val
            else:
                if prev_value != median2_val: #change
                    if before_anychange: # no change has been detected
                        min_distance = index
                        prev_value = median2_val
                        before_anychange = False
                    else: # normal situation
                        prev_value = median2_val
                        distance = index - prev_index
                        distance_list.append(distance)
                        prev_index = index
                        if distance < min_distance:
                            min_distance = distance
                            print('index: ', index)
                            print('distance: ', min_distance)
                            
print (min_distance)
distance_list.sort()
print (distance_list)
dist_avg = sum(distance_list)/len(distance_list)
print (dist_avg)
            
perc10 = int(len(distance_list)/10)
print (perc10)
min_distance = distance_list[perc10]

print (min_distance)

if min_distance >= 4:
    if min_distance < 16:
        skip = 4
    else:
        half_mindist = int(min_distance/2)
        # to have a number power of 4, and not have decimal time
        skip = half_mindist - half_mindist%4
else:
    skip = 1

print (skip)

# ------------- Save to csv and to list

red_index     = []
red_time      = []
red_median2   = []
red_median1   = []
red_mean      = []
red_mean_int  = []
red_orig_data = []

csvres_filename = csv_filename + '_red.csv'
with open(csvres_filename, 'w') as csv_file:

    csv_file.write('index,time,median2,median1,mean,mean int,original\n')
    for index in range(len(median2)):
        if index % skip == 0:
            csv_file.write(str(index) + ',')
            csv_file.write(str(int(time[index])) + ',')
            csv_file.write(str(median2[index]) + ',')
            csv_file.write(str(median1[index]) + ',')
            csv_file.write(str(mean[index]) + ',')
            csv_file.write(str(mean_int[index]) + ',')
            csv_file.write(str(orig_data[index]) + '\n')
            red_index = index
            red_time.append(int(time[index]))
            red_median2.append(median2[index])
            red_median1.append(median1[index])
            red_mean.append(mean[index])
            red_mean_int.append(mean_int[index])
            red_orig_data.append(orig_data[index])

# ------------- draw plots

max_value = max(red_median2)

fig, ax = plt.subplots()
ax.plot(red_time, red_median2, linewidth = 3, color='k', label='median 2')
ax.plot(red_time, red_orig_data, linewidth=0.5, color='b', linestyle='dotted', label='original data')
ax.plot(red_time, red_median1, color='r', label= 'median 1')
ax.plot(red_time, red_mean, color='g', label = 'mean')
#ax.plot(red_time, red_mean_int, color='m', linewidth = 1, linestyle='dotted', label = 'mean integer')
ax.set_xlim(left=0)
ax.set_ylim(bottom=0, top=max_value)
ax.set(yticks=np.arange(0,max_value,20))
ax.grid(True)
ax.legend()
plt.xlabel('Time (ms)')
plt.ylabel('Position (um)')
plt.show()

