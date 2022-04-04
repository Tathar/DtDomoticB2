#!/usr/bin/env python3
import serial
import time
import csv

ser = serial.Serial(port='/dev/ttyACM0',baudrate=57600)
ser.flushInput()

def get_filename() -> str:
    return "./log_{}.csv".format(time.strftime("%y_%m_%d_%H_%M_%S"))

filename = get_filename()

while True:
    try:
        ser_bytes = ser.readline()
        decoded_bytes = ser_bytes[0:len(ser_bytes)-2].decode("UTF8")
        #print(decoded_bytes)
        if ( "starting board" in decoded_bytes) :
            filename = get_filename()

        with open(filename,"a") as f:
            writer = csv.writer(f,delimiter=",")
            writer.writerow([time.strftime("%y-%m-%d-%H:%M:%S"),decoded_bytes])
    except:
        print("Keyboard Interrupt")
        break