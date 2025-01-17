import seaborn
import matplotlib.pyplot as plt

dir = "../1_diff/"

def createPlot(filename):
    file = open(dir + 'w_' + filename + '.txt', mode = 'r')
    lines = file.readlines()
    file.close()
    totalArray = []
    for line in lines:
        line = line.split('\n')
        val = line[0].replace("#", "0")
        array = []
        for i in range(0, (10 * 29) + 1, 10):
            val2 = val[i : i + 9]
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

createPlot("2772946886")

for i in range (100000000, 2800000000, 100000000):
    filename = str(i).zfill(10)
    createPlot(filename)
