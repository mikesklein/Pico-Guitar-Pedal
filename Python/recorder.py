import serial
import time
import csv

ser = serial.Serial('/dev/cu.usbmodem0000000000001',baudrate=115200, timeout=None)
#ser.flushInput()
music = []

print("Capturing")
# for s in range(samples):
#     ser.write("g".encode())
#     ser_bytes = ser.readline()
#     response.append(ser_bytes)
ser.write("g".encode())
for i in range(132300):
    ser_bytes = ser.readline()
    # bytesToRead = ser.inWaiting()
    music.append(ser_bytes)

print("Writing")

for samp in music:
    with open("audio_data.csv", "a") as f:
        writer = csv.writer(f, delimiter=",")
        writer.writerow([time.time(), samp.decode().lstrip().rstrip()])


#
# print("Converting")
#
# for r in response:
#     samp = r.decode().lstrip().rstrip()
#     music.append(samp)
#
# print("Writing")
#
# for sample in music:
#     if sample:
#         with open("audio_data.csv", "a") as f:
#             writer = csv.writer(f, delimiter=",")
#             writer.writerow([time.time(), sample])
#         #
#         # # ser.write("g".encode())
#         # # command = ser.readline().decode()
#         # if command:
#         #     print('Capturing...')
