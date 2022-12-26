#!/usr/bin/env python
# coding: utf-8

# ## Magnetic Encoder Analysis
# cut the data into smaller captures, because it is too big


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

# --- November 2022 wrong experiments
#data_filename = "mov_100mmh_1_to_10_fallo.bin"
#data_filename = "mov_100mmh_1_to_20.bin"
#data_filename = "mov_25mmh_10_to_20_cont_50.bin"
#data_filename = "mov_25mmh_1_to_5.bin"
#data_filename = "mov_25mmh_cont_50.bin"
#data_filename = "mov_75mmh_1_to_20.bin"

# --- November 2022 good experiments
dict_cuts = {
    25: {  # velocity in mm/h
        5 : { # distance of experiment in mm
            'time_ini': 0, # initial time cut in seconds
            'time_fin': 763 # final time cut in seconds
            },
        10: { # distance of experiment in mm
            'time_ini': 800, # initial time cut in seconds
            'time_fin': 2250 # final time cut in seconds
            },
        20: { # distance of experiment in mm
            'time_ini': 2700, # initial time cut in seconds
            'time_fin': 5620  # final time cut in seconds
            },
        50: { # This experiment didnt go well
            'time_ini': 5620, # initial time cut in seconds
            'time_fin': 12247 # final time cut in seconds
            }
        },
    75: {  # velocity in mm/h
        5 : { # distance of experiment in mm
            'time_ini': 0, # initial time cut in seconds
            'time_fin': 288 # final time cut in seconds
            },
        10: { # distance of experiment in mm
            'time_ini': 288, # initial time cut in seconds
            'time_fin': 875  # final time cut in seconds
            },
        20: { # distance of experiment in mm
            'time_ini': 875,  # initial time cut in seconds
            'time_fin': 1900  # final time cut in seconds
            },
        50: { # This experiment didnt go well
            'time_ini': 2063, # initial time cut in seconds
            'time_fin': 4500 # final time cut in seconds
            }
        },
    100: {  # velocity in mm/h
        5 : { # distance of experiment in mm
            'time_ini': 0, # initial time cut in seconds
            'time_fin': 255 # final time cut in seconds
            },
        10: { # distance of experiment in mm
            'time_ini': 255, # initial time cut in seconds
            'time_fin': 625  # final time cut in seconds
            },
        20: { # distance of experiment in mm
            'time_ini': 663,  # initial time cut in seconds
            'time_fin': 1400  # final time cut in seconds
            },
        50: { # This experiment didnt go well
            'time_ini': 1448, # initial time cut in seconds
            'time_fin': 3300 # final time cut in seconds
            }
        }
    }

file_prefix = "exp5kg_"
file_suffix = "mmh"
file_ext = ".bin"
DIR = "./files_nov2/"
for vel_i,dict_vel in dict_cuts.items(): # key and value (value is a dict)
    data_basefilename = file_prefix + str(vel_i) + file_suffix
    data_fulfilename = DIR + data_basefilename + file_ext
    with open(data_fulfilename,"rb") as f:
        raw = f.read()
    ini_list = []
    fin_list = []
    dist_list = []
    file_list = [] 
    for dist_i,dict_dist in dict_vel.items(): # the four distances: 5,10,20,50
        # time in seconds x 1000: milliseconds -> x4: 250 us each sample
        ini_list.append(4000*dict_dist['time_ini'])
        fin_list.append(4000*dict_dist['time_fin'])
        dist_list.append(dist_i)
        file_name_i = ( DIR + data_basefilename +
                        '_' + str(dist_i) + 'mm' + # distance of experiment
                        '_' + str(dict_dist['time_ini']) + # time init of the cut
                        '_' + str(dict_dist['time_fin']) + 's' + file_ext)
        rawfp = open(file_name_i, 'wb')
        file_list.append(rawfp)
    print (ini_list)
    print (fin_list)
    print (dist_list)
    for sample_index in range(len(raw)): # traverse the raw file content
        data = raw[sample_index]
        for dist_index in range(len(dist_list)): # the 4 experiment distances 
            if (sample_index >= ini_list[dist_index] and
                sample_index <  fin_list[dist_index]):
                rawfp = file_list[dist_index]
                rawfp.write(bytes([data]))
    for file_i in file_list:
        file_i.close()
