import matplotlib.pyplot as plt
import sys
import numpy as np
from scipy import signal
import scipy.fftpack
import csv
 
data_filename = "exp5kg_25mmh_5mm_0_763s_lp.csv"
data_filename = "exp5kg_25mmh_10mm_800_2250s_lp.csv"
#data_filename = "exp5kg_25mmh_20mm_2700_5620s_lp.csv"
#data_filename = "exp5kg_25mmh_50mm_5620_12247s_lp.csv"

#data_filename = "exp5kg_75mmh_5mm_0_288s_lp.csv"
#data_filename = "exp5kg_75mmh_10mm_288_875s_lp.csv"
#data_filename = "exp5kg_75mmh_20mm_875_1900s_lp.csv"
#data_filename = "exp5kg_75mmh_50mm_2063_4500s_lp.csv"

#data_filename = "exp5kg_100mmh_5mm_0_255s_lp.csv"
#data_filename = "exp5kg_100mmh_10mm_255_625s_lp.csv"
#data_filename = "exp5kg_100mmh_20mm_663_1400s_lp.csv"
#data_filename = "exp5kg_100mmh_50mm_1488_3300s_lp.csv"

# choose what to plot, uncomment one:
#plot_data = "median1"
#plot_data = "median2"
#plot_data = "mean"
plot_data = "mean2"
#plot_data = "mean_int"
#plot_data = "orig_basedata"

if data_filename.startswith("exp5kg_25mmh_5mm_"):
    init = 32 * 1000 * 4 # start at 30 s
    end = 751 * 1000 * 4 # end
elif data_filename.startswith("exp5kg_25mmh_10mm_"):
    init = 3287 * 4 # start at 3.287 s
    end = 1443.76 * 1000 * 4 # end
elif data_filename.startswith("exp5kg_25mmh_20mm_"):
    init = 38776 * 4 # start 
    end = 2881.064 * 1000 * 4 # end
elif data_filename.startswith("exp5kg_100mmh_20mm_"):
    init = 968 * 4 # start at 950 ms
    end = 719.3 * 1000 * 4 # end at 719,200 s
elif data_filename.startswith("exp5kg_100mmh_50mm_"):
    init = 9783 * 4 # start at 9783 ms
    end = 1805.7 * 1000 * 4 # end at 1805,700 s
elif data_filename.startswith("exp5kg_75mmh_50mm_"):
    init = 11980 * 4 # start at 11980 ms
    end =  2413.11 * 1000 * 4 # end at 2.4 Ms
elif data_filename.startswith("exp5kg_75mmh_20mm_"):
    init = 58113 * 4 # start
    end =  1018.226 * 1000 * 4 # end
elif data_filename.startswith("exp5kg_75mmh_10mm_"):
    init = 37400 * 4 # start
    end =  517.437 * 1000 * 4 # end
else:
    init = 1000
    end = 0

DIR = "./files_nov2/"
data_fulfilename = DIR + data_filename
print("File to show: " + data_fulfilename)

print("Taking ini: " + str(init/4000) + "  end: " + str(end/4000) + ' seconds')

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

x         = []
time      = []
median2   = []
median1   = []
mean      = []
mean2     = []
mean_int  = []
orig_basedata = [] # original data but with added base, so not limited 0 to 255
orig_data = []

mean2_included = False

with open(data_fulfilename,"r") as csv_file:
    row_count = sum(1 for row in csv_file)

with open(data_fulfilename,"r") as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    first_row = True
    sample_i = 0
    if end == 0:
        end = row_count - 1000
    
    for csv_row in csv_reader:
        if first_row: # first row has the header
            first_row = False
            if len(csv_row) == 9:
                print (str(len(csv_row)))
                mean2_included = True # it was not included in the beginning
        else:
            if sample_i > init and sample_i < end: 
                x.append(float(csv_row[1])) # x is the index, x axis
                time.append(float(csv_row[1])) # 0 is the index
                median2.append(float(csv_row[2]))
                median1.append(float(csv_row[3]))
                mean.append(float(csv_row[4]))
                mean_int.append(float(csv_row[5]))
                orig_basedata.append(int(csv_row[6]))
                if mean2_included:
                    mean2.append(float(csv_row[8]))

            sample_i += 1



if plot_data == "median1":
    data = median1
elif plot_data == "median2":
    data = median2
elif plot_data == "mean":
    data = mean
elif plot_data == "mean_int":
    print ('mean_int')
    data = mean_int
elif plot_data == "mean2":
    data = mean2
else:
    data = orig_basedata
    print ('taking orig_basedata')
    
num_samples = len(data)
print("Samples read " + str(num_samples))

dt = 0.250 / 1000 # 0.250 -> ms 0.00025 s  # sampling interval
print("Sampling interval: " + str(1000*dt) + ' ms')
f_s = 1 / dt  #sampling rate. 4000
print("Sampling rate: " + str(f_s) + ' Hz')

#tot_time = num_samples * dt # total time
tot_time = num_samples / f_s # total time

#option = 'PSD' 
#option = 'scipy' 
option = 'orig' 
#option = 'scipy2' 

# to have "horizontal"
x_pt = [0, num_samples-1]
y_pt = [orig_basedata[0], orig_basedata[-1]]
coefficients = np.polyfit(x_pt, y_pt,1)
line = np.poly1d(coefficients)
avg_vel = line(x)
#end_val = orig_basedata[-1]
#avg_vel = x * (end_val/num_samples)
data_horiz = np.divide(data, avg_vel)
data_horiz = data_horiz - np.mean(data_horiz)

remove = 150000-1
data_horiz = data_horiz[remove:]
data_horiz = data_horiz - np.mean(data_horiz)
data = data[remove:]
x = x[remove:]
avg_vel = avg_vel[remove:]
num_samples -= remove

fig, ax = plt.subplots()
if option == 'PSD':
    #fdata = np.fft.fft(data,num_samples) # compute FFT
    fdata = np.fft.fft(data) # compute FFT
    PSD = fdata * np.conj(fdata) / num_samples # power spectrum (power per frequency)
    freq_x = (1/tot_time) * np.arange(num_samples) # axis of frequencies

    #print(PSD.shape)
    #print(freq_x)

    L = np.arange(1, np.floor(num_samples/2), dtype='int') # plot the first half
    #print (L)

    plt.plot(freq_x[L], PSD[L], color='c', label='power spectrum')
    plt.title(plot_data + ' PSD numpy ' + data_filename)
elif option=='scipy':
    fdata = scipy.fftpack.fft(data)
    freq_x = np.linspace(0.0, 1/(2. * dt), num_samples//2)
    plt.plot(freq_x, (2./num_samples) * np.abs(fdata[:num_samples//2]), color='c', label ='scipy fft')
    plt.title(plot_data + ' scipy ' + data_filename)
elif option=='orig': # plot the original shape, but horizontal
    plt.plot(x, avg_vel, color='c', label = 'mean2 vel - avg dist')
    plt.plot(x, data_horiz, color='b', label = 'vel - avg vel')
    plt.plot(x, data, color='r', label ='mean2 vel')
    plt.title(plot_data + ' No FFT: orig ' + data_filename)
elif option=='scipy2':
    fdata = scipy.fftpack.fft(data_horiz)
    freq_x = np.linspace(0.0, 1/(2. * dt), num_samples//2)
    plt.plot(freq_x, (2./num_samples) * np.abs(fdata[:num_samples//2]), color='c', label= 'scipy (vel -avg_vel)' )
    plt.title(plot_data + ' scipy horiz ' + data_filename)
    
    
plt.show()

# ------------------ derivative with FFT
#freq_shift = np.fft.fftshift(freq_x)
#deriv_fdata = freq_shift * fdata * (1j)
#deriv_data = np.real(np.fft.ifft(deriv_fdata))

#fig, ax = plt.subplots()
#plt.plot(time, data, color='b', label='data')
#plt.plot(time, deriv_data, color='c', label=plot_data)
#plt.show()

#Ts = 0.250 / 1000 # 0.250 -> ms 0.00025 s
#print("Sampling interval: " + str(1000*Ts) + ' ms')
#f_s = 1 / Ts  #sampling rate. 4000
#print("Sampling rate: " + str(f_s) + ' Hz')

#tot_time = num_samples / f_s # total time
#fdata = fftpack.fft(median2)
#freqs = fftpack.fftfreq(num_samples) * f_s

#fig, ax = plt.subplots()

#ax.stem(freqs, np.abs(fdata))
#ax.set_xlabel('Frequency in Hertz [Hz]')
#ax.set_ylabel('Frequency Domain (Spectrum) Magnitude')
#ax.set_xlim(-f_s / 2, f_s / 2)





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
