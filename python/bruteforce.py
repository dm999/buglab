import numpy as np
import run_cython
import time

#~ help(run_cython)

threads = 9

val = []

wdt = 5
hgt = 3


maxVal = 0
indexV = 0

start = time.time()

for i in range(0, 2**(hgt * wdt)):
    valTmp = list('{0:0b}'.format(i))
    newLen = hgt * wdt - len(valTmp)
    valTmp = [0] * newLen + valTmp
    val.append(valTmp)
    if(len(val) == 1000):
        val = np.array(val).astype(np.bool)
        res = run_cython.py_compute(val, threads)
        for j in range (0, val.shape[0]):
            if(maxVal == res[j][0]):
                finalRes.append(res[j])
            if(maxVal < res[j][0]):
                maxVal = res[j][0]
                indexV = j
                finalRes = []
                finalRes.append(res[j])
        val = []

val = np.array(val).astype(np.bool)
res = run_cython.py_compute(val, threads)

for i in range (0, val.shape[0]):
    if(maxVal == res[i][0]):
        finalRes.append(res[i])
    if(maxVal < res[i][0]):
        maxVal = res[i][0]
        indexV = i
        finalRes = []
        finalRes.append(res[i])


end = time.time()
print(f'time: {end - start:.3f}')
print(f"{maxVal} {len(finalRes)} {hgt}x{wdt}")


for k in range (0, len(finalRes)):
    for i in range (0, hgt):
        for j in range (0, wdt):
            if(finalRes[k][i * wdt + j + 1] == 1e9): print("  #", end=' ')
            else: print(f"{finalRes[k][i * wdt + j + 1].astype(int):3d}", end=' ')
        print()
    print()