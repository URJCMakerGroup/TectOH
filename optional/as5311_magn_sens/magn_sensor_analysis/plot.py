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
#DIR = "./files_nov/"
DIR = "./files_nov2/"

# ow.walk [2] are filenames
dir_list = next(os.walk(DIR))[2]

# could be reduced or not
csv_list = [csv for csv in dir_list if csv.endswith('.csv')]

if not csv_list: # list is false if empty
    print("No csv file found in resuls directory: " + DIR)
    exit()

csv_list.sort()

# without the extension:
csv_base_list = [pathlib.Path(csvfile).stem for csvfile in csv_list]

print ("  --- csv list:\n" + '\n'.join(csv_base_list))

print('\n  Entry data name to generate plot:\n\n')
csv_name = input()

if not csv_name:
    print("taking: ", csv_base_list[0])
    print("\n\n")
    csv_name      = csv_base_list[0]
elif not csv_name in csv_base_list: 
    print("file not found: ", csv_name)
    exit()

# -------------------- Options
limit_in = True # if you want to plot from just the beginning of movement
#limit_in = False # if you want to plot from time zero
#scale_segs = False
scale_segs = True
#only_mean2 = False
only_mean2 = True
# -------------------- Options

if limit_in == True:
    if csv_name.startswith("exp5kg_25mmh_5mm_"):
        init = 32 * 1000 * 4 # start
        end = 751 * 1000 * 4 # end
    elif csv_name.startswith("exp5kg_25mmh_10mm_"):
        init = 3287 * 4 # start
        end = 1443.76 * 1000 * 4 # end
    elif csv_name.startswith("exp5kg_25mmh_20mm_"):
        init = 38776 * 4 # start
        end = 2881.064 * 1000 * 4 # end
    elif csv_name.startswith("exp5kg_100mmh_20mm_"):
        #init = 968 * 4 # start at 950 ms
        init = 0 * 4 # it seems that it start earlier, but there is some noise
        end = 719.3 * 1000 * 4 # end at 719,200 s
    elif csv_name.startswith("exp5kg_100mmh_50mm_"):
        init = 9783 * 4 # start at 9783 ms
        end = 1805.7 * 1000 * 4 # end at 1805,700 s
    elif csv_name.startswith("exp5kg_75mmh_50mm_"):
        init = 11980 * 4 # start at 11980 ms
        end =  2413.11 * 1000 * 4 # end at 2.4 Ms
    elif csv_name.startswith("exp5kg_75mmh_20mm_"):
        init = 58113 * 4 # start
        end =  1018.226 * 1000 * 4 # end
    elif csv_name.startswith("exp5kg_75mmh_10mm_"):
        init = 37400 * 4 # start
        end =  517.437 * 1000 * 4 # end
        print (init)
    else:
        init = 0
        end = 0
else:
    init = 0
    end = 0


csv_fulfilename = DIR + csv_name + '.csv'

# this is the order
# index time median2 median1 mean mean_int orig_data

red_time      = []
red_median2   = []
red_median1   = []
red_mean      = []
red_mean2     = []
red_mean_int  = []
red_orig_basedata = []
red_orig_data = []

#mean2_included = False


with open(csv_fulfilename,"r") as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    first_row = True
    for csv_row in csv_reader:
        if first_row: # first row has the header
            first_row = False
            idx = 0
        else:
            if idx >= init:
                if idx == init:
                    zero_time =float(csv_row[1])
                if scale_segs == True:
                    red_time.append((float(csv_row[1])-zero_time)/1000) # 0 is the index
                else:
                    red_time.append(float(csv_row[1])-zero_time) # 0 is the index
                red_median2.append(float(csv_row[2]))
                red_median1.append(float(csv_row[3]))
                red_mean.append(float(csv_row[4]))
                red_mean_int.append(float(csv_row[5]))
                basedata = csv_row[6]
                if '.' in basedata:
                    print("error, base data is not integer: " + basedata + ' # index: ' + csv_row[0])
                red_orig_basedata.append(int(basedata))
                red_orig_data.append(int(csv_row[7]))
                red_mean2.append(float(csv_row[8]))
            idx += 1
                            
# ------------- draw plots

#level = 15480 # change value if you want to see the base at certain level
level = 0
#level = 1000
#level = 15480 # change value if you want to see the base at certain level
#level = 31200 # change value if you want to see the base at certain level
if level != 0:
    red_orig_data = np.asarray(red_orig_data)
    red_orig_data += level

#max_value = max(red_median2)


if only_mean2 == True:
    min_mean2 = min(red_mean2)
    for idx in range(len(red_mean2)):
        red_mean2[idx] -= min_mean2 


fig, ax = plt.subplots()
if only_mean2 == False:
    max_value = max(red_orig_basedata) + 50
    ax.plot(red_time, red_median2, linewidth = 3, color='k', label='median 2')
    ax.plot(red_time, red_orig_basedata, linewidth=0.5, color='c', linestyle='dotted', label='original basedata')
    ax.plot(red_time, red_orig_data, linewidth=0.5, color='m', linestyle='dotted', label='original data')
    #ax.plot(red_time, red_median1, color='r', label= 'median 1')
    ax.plot(red_time, red_mean, color='g', label = 'mean')
    #ax.plot(red_time, red_mean_int, color='m', linewidth = 1, linestyle='dotted', label = 'mean integer')
else:
    max_value = max(red_mean2) + 50

ax.plot(red_time, red_mean2, color='b', linewidth = 1, label = 'mean2')
ax.set_ylim(bottom=0, top=max_value)
ax.set_xlim(left=0)
#ax.set(yticks=np.arange(0,max_value,20))
if only_mean2 == False:
    ax.legend()
ax.grid(True)
plt.title(csv_name)
if scale_segs == True:
    plt.xlabel('Time (s)')
else:
    plt.xlabel('Time (ms)')
plt.ylabel('Position (um)')
plt.show()

