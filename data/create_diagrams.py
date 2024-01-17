import matplotlib.pyplot as plt
import numpy as np

#change the absolute path here!!
absolutePath = "/home/jan/ns3/ns-allinone-3.39/ns-3.39/scratch/data/"
models = ["FriisPropagationLossModel", "FixedRssLossModel", "ThreeLogDistancePropagationLossModel", "TwoRayGroundPropagationLossModel", "NakagamiPropagationLossModel"]

def read_file(file_path):
    file = open(absolutePath + file_path, 'r')
    line = file.read()
    arr = np.array([s for s in line.strip().split(';') if s != ""])
    return arr.astype(float)



runtime_data = read_file("SimulationTime_runtime.txt")
throughput_data = read_file("SimulationTime_throughput.txt")

plt.plot(runtime_data, throughput_data, marker="o", color='b')
plt.plot(runtime_data, np.full(throughput_data.shape, throughput_data[9]), linestyle='--', c="red")
plt.plot(runtime_data[9], throughput_data[9], marker='o', color='red')
plt.xlabel('Runtime (seconds)')
plt.ylabel('Throughput (Mbps)')
plt.grid(True)
plt.show()


# plot first graph
for model in models:
    distance_data = read_file(model + "_distance.txt")
    rss_data = read_file(model + "_rss.txt")
    plt.plot(distance_data, rss_data)


plt.xlabel("Distance (m)")
plt.ylabel("RSS (dBm)")
plt.legend(models, loc="center right")
plt.show()

# plot second graph
for model in models:
    distance_data = read_file(model + "_distance.txt")
    throughput_data = read_file(model + "_throughput.txt")
    plt.plot(distance_data, throughput_data)

plt.xlabel("Distance (m)")
plt.ylabel("Throughput (Mbps)")
plt.legend(models)
plt.show()
