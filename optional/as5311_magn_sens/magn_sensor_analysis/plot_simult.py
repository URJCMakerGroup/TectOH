#!/usr/bin/env python
# coding: utf-8

# Plot various results simultaneously results, that are in a csv file

import pathlib
import csv
import math
import os

import matplotlib.pyplot as plt
import numpy as np

# directory where the csv files are
#DIR = "./files/"
#DIR = "./files_nov/"
DIR = "./files_nov2/"


data_file_list = [
#   "exp5kg_25mmh_5mm_0_763s_lp.csv",
#   "exp5kg_25mmh_5mm_0_763s_lp.csv",
#   "exp5kg_25mmh_10mm_800_2250s_lp.csv",
   "exp5kg_25mmh_20mm_2700_5620s_lp.csv",
#   "exp5kg_25mmh_50mm_5620_12247s_lp.csv",

#   "exp5kg_75mmh_5mm_0_288s_lp.csv",
#   "exp5kg_75mmh_10mm_288_875s_lp.csv"
   "exp5kg_75mmh_20mm_875_1900s_lp.csv",
#   "exp5kg_75mmh_50mm_2063_4500s_lp.csv",

#   "exp5kg_100mmh_5mm_0_255s_lp.csv",
#   "exp5kg_100mmh_10mm_255_625s_lp.csv",
   "exp5kg_100mmh_20mm_663_1400s_lp.csv"
#   "exp5kg_100mmh_50mm_1488_3300s_lp.csv"
]

labels = []

for name_i in data_file_list:
    idx = name_i.find("mm_")
    labels.append(name_i[:idx+2])

print(labels)

# -------------------- Options
limit_in = True # if you want to plot from just the beginning of movement
#limit_in = False # if you want to plot from time zero
#scale_segs = False
scale_segs = True
#scale_mm = False
scale_mm = True
#only_mean2 = False
only_mean2 = True
# -------------------- Options

mean2_datalist_list = [] # these are list of datalist
time_datalist_list = []

for csv_name in data_file_list:
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
        else:
            init = 0
            end = 0
    else:
        init = 0
        end = 0


    mean2_datalist  = []

    time_datalist = []

    with open(DIR + csv_name,"r") as csv_file:
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
                        time_datalist.append((float(csv_row[1])-zero_time)/1000) # 0 is the index
                    else:
                        time_datalist.append(float(csv_row[1])-zero_time) # 0 is the index
                    if scale_mm == True:
                        mean2_datalist.append(float(csv_row[8])/1000)
                    else:
                        mean2_datalist.append(float(csv_row[8]))
                idx += 1

    mean2_datalist_list.append(mean2_datalist)
    time_datalist_list.append(time_datalist)
                            
# ------------- draw plots


colors = ['b', 'g', 'r']

max_y_value = 0
max_t_value = 0
fig, ax = plt.subplots()
for idx, datalist_i  in enumerate(mean2_datalist_list):

    time_datalist_i = time_datalist_list[idx]
    max_y_value_i = max(datalist_i)
    if max_y_value_i > max_y_value:
        max_y_value = max_y_value_i
    max_t_value_i = max(time_datalist_i)
    if max_t_value_i > max_t_value:
        max_t_value = max_t_value_i
    ax.plot(time_datalist_i, datalist_i, color=colors[idx], label=labels[idx])

if scale_mm == True:
    max_y_value = math.ceil(max_y_value)
else:
    max_y_value = 100*math.ceil(max_y_value/100)
if scale_segs == True:
    max_t_value = 100*math.ceil(max_t_value/100)
else:
    max_t_value = 100000*math.ceil(max_t_value/100000)

ax.set_ylim(bottom=0, top=max_y_value)
ax.set_xlim(left=0, right=max_t_value)
#ticks_y = 100*((max_y_value//20)//100)
if scale_mm == True:
    ticks_y = 1
else:
    ticks_y = 1000
ax.set(yticks=np.arange(0,max_y_value,ticks_y))
ax.set(xticks=np.arange(0,max_t_value,100))
ax.legend()
ax.grid(True)
#plt.title(csv_name)
if scale_segs == True:
    plt.xlabel('Time (s)')
else:
    plt.xlabel('Time (ms)')
if scale_mm == True:
    plt.ylabel('Position (mm)')
else:
    plt.ylabel('Position (um)')
plt.show()

