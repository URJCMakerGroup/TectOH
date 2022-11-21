# this is just an example and doesnt work because the 
# the binary data only goes from 0 to 255, so it has a lot
# of frequencies, since it is like a sawtooth


import matplotlib.pyplot as plt
import sys
import numpy as np
from scipy import signal
from scipy import fftpack
 
#data_filename = "exp5kg_25mmh_5mm_0_763s.bin"
#data_filename = "exp5kg_25mmh_10mm_800_2250s.bin"
data_filename = "exp5kg_25mmh_20mm_2700_5620s.bin"
#data_filename = "exp5kg_25mmh_50mm_5620_12247s.bin"

#data_filename = "exp5kg_75mmh_5mm_0_288s.bin"
#data_filename = "exp5kg_75mmh_10mm_288_875s.bin"
#data_filename = "exp5kg_75mmh_20mm_875_1900s.bin"
#data_filename = "exp5kg_75mmh_50mm_2063_4500s.bin"

#data_filename = "exp5kg_100mmh_5mm_0_255s.bin"
#data_filename = "exp5kg_100mmh_10mm_255_625s.bin"
#data_filename = "exp5kg_100mmh_20mm_663_1400s.bin"
#data_filename = "exp5kg_100mmh_50mm_1488_3300s.bin"


DIR = "./files_nov2/"
data_fulfilename = DIR + data_filename
print("File to show: " + data_fulfilename)

# mechanical frequency, revolutions per second of the screw
LEAD = 3 # leadscrew has 3 mm lead mm_rev

MOTOR_POLES = 200 # motor has 200 poles
GEAR_RATIO = 50.9 # gear ratio of the gear box

if '_100mmh_' in data_filename:
    vel_mmh = 100
elif '_75mmh_' in data_filename:
    vel_mmh = 75
elif '_25mmh_' in data_filename:
    vel_mmh = 25
else:
    print('no speed found')

# speed in mm per second
vel_mmseg = vel_mmh / 3600

# now we have the mm per second, we get the revolutions per second
rot_freq = vel_mmseg / LEAD # rev/s = (mm/s) / (mm/rev)

print ('Leadscrew rotation frequency : ' + str(rot_freq) + ' rev/s')

with open(data_fulfilename,"rb") as f:
    raw = f.read()

time = list(range(len(raw)))
data = list(range(len(raw)))

for x in range(len(raw)):
    data[x] = raw[x]
    
num_samples = len(data)
print("Samples read " + str(num_samples))

dt = 0.250 / 1000 # 0.250 -> ms 0.00025 s  # sampling interval
print("Sampling interval: " + str(1000*dt) + ' ms')
f_s = 1 / dt  #sampling rate. 4000
print("Sampling rate: " + str(f_s) + ' Hz')

#tot_time = num_samples * dt # total time
tot_time = num_samples / f_s # total time

fdata = np.fft.fft(data,num_samples) # compute FFT

PSD = fdata * np.conj(fdata) / num_samples # power spectrum (power per frequency)
freq_x = (1/tot_time) * np.arange(num_samples) # axis of frequencies

#print(PSD.shape)

#print(freq_x)

L = np.arange(1, np.floor(num_samples/2), dtype='int') # plot the first half

#print (L)

fig, ax = plt.subplots()
plt.plot(freq_x[L], PSD[L], color='c')




#fdata = fftpack.fft(data)
#freqs = fftpack.fftfreq(num_samples) * f_s

#fig, ax = plt.subplots()

#ax.stem(freqs, np.abs(fdata))
#ax.set_xlabel('Frequency in Hertz [Hz]')
#ax.set_ylabel('Frequency Domain (Spectrum) Magnitude')
#ax.set_xlim(-f_s / 2, f_s / 2)



plt.show()


# def compare_signal(raw_signal, **kwargs):
    # fig, ax1 = plt.subplots(1, 1)
    # ax1.plot(raw_signal, 'k-', label='Raw Signal')
    # for i, (sig_name, sig_value) in enumerate(kwargs.items()):
        # ax1.plot(sig_value, '-.', lw=4, alpha=0.5, label=sig_name)
    # ax1.legend()
    # plt.show()  
    
# simple_1d = np.arange(0, 1024, 4)
# simple_1d_wrapped = simple_1d % 255
# compare_signal(simple_1d, Wrapped=simple_1d_wrapped)
# compare_signal(simple_1d, Wrapped=simple_1d_wrapped, Unwrapped=np.unwrap(simple_1d_wrapped, discont=127))
