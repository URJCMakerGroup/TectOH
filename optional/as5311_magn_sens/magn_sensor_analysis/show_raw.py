import matplotlib.pyplot as plt
import sys
import numpy as np
from scipy import signal
 
filename = sys.argv[1]
print("File to show " + filename)

with open(filename,"rb") as f:
    raw = f.read()

time = list(range(len(raw)))
data = list(range(len(raw)))

for x in range(len(raw)):
    data[x] = raw[x]
    
print("Samples read " + str(len(data)))

arr_data = np.array(data, dtype=int)

plt.plot(time, arr_data)
plt.xlabel('Samples')
plt.ylabel('Position (??)')
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
