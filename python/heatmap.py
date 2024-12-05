import seaborn
import matplotlib.pyplot as plt

dir = "../1/"

def createPlot(filename):
    file = open(dir + 'w_' + filename + '.txt', mode = 'r')
    lines = file.readlines()
    file.close()
    totalArray = []
    for line in lines:
        line = line.split('\n')
        val = line[0].replace("#", "0")
        array = []
        for i in range(0, 204, 8):
            val2 = val[i:i+7]
            val2 = val2.replace(" ", "")
            if(val2):
                array.append(int(val2))
        totalArray.append(array)

    #~ plt.tight_layout()
    fig, ax = plt.subplots(figsize=(15,8))
    ax = seaborn.heatmap(totalArray)
    ax.set_xlabel('Actual')
    ax.set_ylabel('Predicted')
    #~ plt.show()
    fig.savefig(dir + filename + '.jpg')
    plt.close()

createPlot("030364964")

for i in range (1000000, 30400000, 1000000):
    filename = str(i).zfill(9)
    createPlot(filename)
