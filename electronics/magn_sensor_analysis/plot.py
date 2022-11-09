#!/usr/bin/env python
# coding: utf-8

# Plot the results, that are in a csv file

import pathlib
import csv
import os

import matplotlib.pyplot as plt
import numpy as np

# directory where the csv files are
#DIR = "./files/"
DIR = "./files_nov/"

# ow.walk [2] are filenames
dir_list = next(os.walk(DIR))[2]

# take only the reduced csv files, the en with _red.csv
redcsv_list = [redcsv for redcsv in dir_list if redcsv.endswith('_red.csv')]

if not redcsv_list: # list is false if empty
    print("No reduced csv file found in resuls directory: " + DIR)
    exit()

redcsv_list.sort()

# without the extension:
csv_base_list = [pathlib.Path(redcsvfile).stem for redcsvfile in redcsv_list]

print ("  --- reduced csv list:\n" + '\n'.join(csv_base_list))

print('\n  Entry data name to generate plot:\n\n')
csv_name = input()

if not csv_name:
    print("taking: ", csv_base_list[0])
    print("\n\n")
    csv_name      = csv_base_list[0]
elif not csv_name in csv_base_list: 
    print("file not found: ", csv_name)
    exit()


csv_fulfilename = DIR + csv_name + '.csv'

# this is the order
# index time median2 median1 mean mean_int orig_data

red_time      = []
red_median2   = []
red_median1   = []
red_mean      = []
red_mean_int  = []
red_orig_data = []

with open(csv_fulfilename,"r") as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    first_row = True
    for csv_row in csv_reader:
        if first_row: # first row has the header
            first_row = False
        else:
            red_time.append(float(csv_row[1])) # 0 is the index
            red_median2.append(float(csv_row[2]))
            red_median1.append(float(csv_row[3]))
            red_mean.append(float(csv_row[4]))
            red_mean_int.append(float(csv_row[5]))
            red_orig_data.append(int(csv_row[6]))
                            
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
plt.title(csv_name)
plt.xlabel('Time (ms)')
plt.ylabel('Position (um)')
plt.show()

