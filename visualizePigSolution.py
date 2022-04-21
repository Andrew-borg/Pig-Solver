import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

data = pd.read_csv(r'C:\Users\Andrew\Desktop\Pig\savedsolution.csv').iloc[: , :4]

#data = data.iloc[:, :4].to_numpy()
data = data[data['roll'] == 1].iloc[:, :3].to_numpy()

voxel_array = np.zeros((100, 100, 100), dtype=bool)

for row in data:
    voxel_array[row[0]][row[1]][row[2]] = 1

x, y, z = np.indices((100, 100, 100))
colors = np.empty(voxel_array.shape, dtype=object)
for height in range(100):
    colors[(x == height)] = str(((height * .5) + 25) * .01)

ax = plt.figure().add_subplot(projection='3d')
ax.voxels(voxel_array, facecolors=colors, edgecolor='none')

plt.show()
