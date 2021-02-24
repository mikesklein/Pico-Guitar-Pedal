import serial
import time
import csv

ser = serial.Serial('/dev/cu.usbmodem0000000000001',2000000)
ser.flushInput()

ser.write("g".encode())
command = ser.readline().decode()
print(command)
if command:
    print('Capturing...')
    for i in range(44100):
        ser_bytes = ser.readline()
        decoded_bytes = float(ser_bytes.decode("utf-8"))
        with open("audio_data.csv", "a") as f:
            writer = csv.writer(f, delimiter=",")
            writer.writerow([time.time(), decoded_bytes])
