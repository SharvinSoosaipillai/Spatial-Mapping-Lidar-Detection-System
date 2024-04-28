import serial
import time
import open3d as o3d
import math
import random

# Initialize the serial port
s = serial.Serial(port='COM5', baudrate=115200, timeout=10)
print("Opening: " + s.name)

coordinates = []
data = []
NUMBEROFSCANS = 3;
STEPS = 32
ANGLE = 11.25
SPACING = 25

# Reset the buffers of the UART port to delete the remaining data in the buffers
s.reset_output_buffer()
s.reset_input_buffer()

# Send the character 's' to MCU via UART
# This will signal MCU to start the transmission
s.write(b's')  # Assuming 's' is encoded as bytes

# how many seconds until it breaks out of while loop
timeout_duration = 10  

# Get the current time
start_time = time.time()

# Continuously listen to the serial port
while True:
    x = s.readline()
    if x:
        print(x.decode())
        data.append(int(x.decode()))
    elif (start_time - time.time()):
        print("exit out of the loop")
        break;
       

totalData = 0
for i in range(NUMBEROFSCANS):
    for j in range(STEPS):
        x = math.cos(math.radians(ANGLE * j)) *data[totalData]
        y = math.sin(math.radians(ANGLE * j)) *data[totalData]
        totalData+=1
        coordinates.append([x, y, SPACING*(i + 1)])        

for i in coordinates:
    print (i)

f = open("pointcloud.xyz", "w")

with open("pointcloud.xyz", "w") as f:
    for p in range(STEPS * NUMBEROFSCANS):
        f.write('{0:f} {1:f} {2:d}\n'.format(coordinates[p][0], coordinates[p][1], SPACING*coordinates[p][2]))

# Read the point cloud from the file
pcd = o3d.io.read_point_cloud("pointcloud.xyz", format="xyz")
pcd.paint_uniform_color([0, 0, 0])  # RGB value for black


lines = []
for i in range(NUMBEROFSCANS * STEPS):
    if (i + 1) % STEPS != 0:  # Ensure we don't connect the last point to the first point of the next scan
        lines.append([i, i + 1])
    else:
        lines.append([i, i - STEPS + 1])

# Remove lines connecting points between circles
for i in range(STEPS, NUMBEROFSCANS * STEPS, STEPS):
    for j in range(STEPS):
        lines.append([i + j - STEPS, i + j])

# Add the connections to the point cloud
line_set = o3d.geometry.LineSet()
line_set.points = pcd.points
line_set.lines = o3d.utility.Vector2iVector(lines)

# Visualize the point cloud with connections
o3d.visualization.draw_geometries([pcd, line_set])