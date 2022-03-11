import os
import serial, sys
import pandas as pd
import numpy as np
from tqdm import tqdm
from plotly import graph_objects as go

class LiDAR:
    def __init__(self, port:str) -> None:
        self.port = port
        self.conn = serial.Serial(port, 115200)
        self.current_angle = 0
        self.storage_boilerplate = {        # column name for FSCAN (csv) file
            "x_angle": [],
            "y_angle": [],
            "measurement": []
        }
        self.command = {                    # commands for the lidar sensor
            "scan": 1,
            "scan4x" : 2,
            "scale": 3,
            "park": 4
        }
    
    # go along the edges of the area to be scanned, no measurement is read
    def go_scale(self, vfov:int, hfov:int, dfov:int) -> None:
        startx = 90 - vfov // 2
        stopx = 90 + vfov // 2
        starty = 90 - hfov
        stopy = 90 + dfov
        signal = f"{self.command['scale']} {startx} {stopx} {starty} {stopy}\n"
        self.conn.write(bytes(signal, encoding="ascii"))

    # park the sensor, program will exit because sensor is put into BOOTSEL mode
    def park(self) -> None:
        signal = f"{self.command['park']} 0 0 0 0\n"
        self.conn.write(bytes(signal, encoding="ascii"))

    # run scan, either in 4-measurement average mode or in one measurement per angle mode
    # vfov, hfov dfov are degree angles
    def scan(self, scan_angle:float, vfov:int, hfov:int, dfov:int, filename:str, quad:bool) -> None:
        # create new dict that will then be written to .fscan file
        storage = self.storage_boilerplate
        self.current_angle = 0

        # limit possible angles as device will else collide with construction
        if( vfov < 0 or vfov > 180):
            raise Exception("LiDAR RANGE ERROR")
        if( hfov < 0 or hfov > 50 ):
            raise Exception("LiDAR RANGE ERROR")
        if( dfov < 0 or dfov > 30 ):
            raise Exception("LiDAR RANGE ERROR")

        # calculate start and stop angles
        startx = 90 - vfov // 2
        stopx = 90 + vfov // 2
        starty = 90 - hfov
        stopy = 90 + dfov

        # format signal to be readable for the lidar and send to sensor to start scan
        signal = f"{self.command['scan4x' if quad else 'scan']} {startx} {stopx} {starty} {stopy}\n"
            
        self.conn.write(bytes(signal, encoding="ascii"))

        # run scan and store values in buffer, then write to file
        num_iter = 4 if quad else 1
        for y in range(stopy - starty + 1):
            for x in range(int((stopx - startx) // scan_angle) + 1):
                measurements = 0
                for _ in range(num_iter):
                    measurement = self.conn.read_until()
                    measurements += int(measurement.decode().strip('\n'))
                storage["x_angle"].append(startx + x)
                storage["y_angle"].append(starty + y)
                storage["measurement"].append(measurements // num_iter)
            self.current_angle += 1

        # write data in buffer to file        
        df = pd.DataFrame.from_dict(storage)
        df.to_csv(filename+".fscan", sep='\t', index=False)
        return True

# visualize fscan file in 3d view
def visualize(filename:str) -> bool:
    if( filename == None ):
        return False
    if( not os.path.isfile(filename) ):
        return False
    
    df = pd.read_csv(filename, sep='\t')

    # retrieve and remap angles in order to be able to parse them into x y z coords
    x_angles = (df["x_angle"].to_numpy().astype("d") - 90)
    y_angles = (df["y_angle"].to_numpy().astype("d") - 90)*-1
    measurements = df["measurement"].to_numpy().astype("d")

    # calculate x y z coordinates via trigonometry, the file only holds angles and measurement
    h1 = np.cos(np.deg2rad(y_angles)) * measurements
    z = (np.sin(np.deg2rad(y_angles)) * measurements)
    x = np.sin(np.deg2rad(x_angles)) * h1
    y = np.cos(np.deg2rad(x_angles)) * h1

    # do some magic and display plot
    rPlot = go.Scatter3d(x=x, y=y, z=z, mode="markers", marker=dict(size=2))
    fig = go.Figure(data=[rPlot], layout=go.Layout(scene=dict(aspectmode="data")))
    fig.show()
    return True