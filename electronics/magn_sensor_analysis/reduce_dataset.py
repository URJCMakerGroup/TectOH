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

# ------------- Select File by uncommenting (extension .csv not included)

# --- September 2022 experiments
#csv_filename = "capture_10_1"
#csv_filename = "capture_25_1"
#csv_filename = "capture_50_1"
#csv_filename = "capture_82_1"
#csv_filename = "capture_100_1"

# --- November 2022 experiments, wrong experiments
#csv_filename = "mov_100mmh_1_to_10_fallo"
#csv_filename = "mov_100mmh_1_to_20"
#csv_filename = "mov_25mmh_10_to_20_cont_50"
#csv_filename = "mov_25mmh_1_to_5"
#csv_filename = "mov_25mmh_cont_50"
#csv_filename = "mov_75mmh_1_to_20"

# cuts from november experiments, second set:

csv_filename = "exp5kg_25mmh_5mm_0_763s"
#csv_filename = "exp5kg_25mmh_10mm_800_2250s"
#csv_filename = "exp5kg_25mmh_20mm_2700_5620s"
#csv_filename = "exp5kg_25mmh_50mm_5620_12247s"

#csv_filename = "exp5kg_75mmh_5mm_0_288s"
#csv_filename = "exp5kg_75mmh_10mm_288_875s"
#csv_filename = "exp5kg_75mmh_20mm_875_1900s"
#csv_filename = "exp5kg_75mmh_50mm_2063_4500s"

#csv_filename = "exp5kg_100mmh_5mm_0_255s"
#csv_filename = "exp5kg_100mmh_10mm_255_625s"
#csv_filename = "exp5kg_100mmh_20mm_663_1400s"
#csv_filename = "exp5kg_100mmh_50mm_1488_3300s"


if csv_filename.startswith("mov"):
    # november experiment wrong experiment
    DIR = "./files_nov/"
elif csv_filename.startswith("exp5kg"):
    # november experiment good experiment
    DIR = "./files_nov2/"
else:
    # september experiment
    DIR = "./files/"

csv_fulfilename = DIR + csv_filename + '.csv'

# this is the order
# index time median2 median1 mean mean_int orig_basedata orig_data

time      = []
median2   = []
median1   = []
mean      = []
mean_int  = []
orig_data = []
orig_basedata = [] # added the base

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
            orig_basedata.append(int(csv_row[6]))
            orig_data.append(int(csv_row[7]))
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
red_orig_basedata = [] # added the base

csvres_filename = DIR + csv_filename + '_red.csv'
with open(csvres_filename, 'w') as csv_file:

    csv_file.write('index,time,median2,median1,mean,mean int,original base,original\n')
    for index in range(len(median2)):
        if index % skip == 0:
            csv_file.write(str(index) + ',')
            csv_file.write(str(int(time[index])) + ',')
            csv_file.write(str(median2[index]) + ',')
            csv_file.write(str(median1[index]) + ',')
            csv_file.write(str(mean[index]) + ',')
            csv_file.write(str(mean_int[index]) + ',')
            csv_file.write(str(orig_basedata[index]) + ',')
            csv_file.write(str(orig_data[index]) + '\n')
            red_index = index
            red_time.append(int(time[index]))
            red_median2.append(median2[index])
            red_median1.append(median1[index])
            red_mean.append(mean[index])
            red_mean_int.append(mean_int[index])
            red_orig_basedata.append(orig_basedata[index])
            red_orig_data.append(orig_data[index])

# ------------- draw plots

max_value = max(red_median2)

fig, ax = plt.subplots()
ax.plot(red_time, red_median2, linewidth = 3, color='k', label='median 2')
ax.plot(red_time, red_orig_basedata, linewidth=0.5, color='c', linestyle='dotted', label='original basedata')
ax.plot(red_time, red_orig_data, linewidth=0.5, color='b', linestyle='dotted', label='original data')
ax.plot(red_time, red_median1, color='r', label= 'median 1')
ax.plot(red_time, red_mean, color='g', label = 'mean')
#ax.plot(red_time, red_mean_int, color='m', linewidth = 1, linestyle='dotted', label = 'mean integer')
ax.set_xlim(left=0)
ax.set_ylim(bottom=0, top=max_value)
ax.set(yticks=np.arange(0,max_value,20))
ax.grid(True)
ax.legend()
plt.title('Reduced ' csv_filename)
plt.xlabel('Time (ms)')
plt.ylabel('Position (um)')
plt.show()

