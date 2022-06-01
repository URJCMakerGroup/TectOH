import matplotlib
matplotlib.use('TkAgg') 
import matplotlib.pyplot as plt
import sys
 
filename = sys.argv[1]
print(filename)

with open(filename,"rb") as f:
    raw = f.read()

time = list(range(len(raw)))
data = list(range(len(raw)))

for x in range(len(raw)):
    data[x] = raw[x]
    
print(len(time))
print(len(data))

plt.plot(time, data)
plt.xlabel('Samples')
plt.ylabel('Position (??)')
plt.show()