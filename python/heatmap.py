import seaborn
import matplotlib.pyplot as plt

file = open('../w_3_844_270.txt', mode = 'r')
lines = file.readlines()
file.close()
totalArray = []
for line in lines:
    line = line.split('\n')
    val = line[0].replace("#", "0")
    array = []
    for i in range(0, 204, 7):
        val2 = val[i:i+6]
        val2 = val2.replace(" ", "")
        if(val2):
            array.append(int(val2))
    totalArray.append(array)

ax = seaborn.heatmap(totalArray, square=True)
ax.set_xlabel('Actual')
ax.set_ylabel('Predicted')
plt.show()