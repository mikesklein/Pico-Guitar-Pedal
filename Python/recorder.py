import serial
import time
import csv
import wave
import struct
import numpy
from scipy.io import wavfile
from scipy.signal import resample


def write_wav(data, filename, framerate, amplitude):
    wavfile = wave.open(filename,'w')
    nchannels = 1
    framerate = framerate
    nframes = len(data)
    comptype = "NONE"
    compname = "not compressed"
    wavfile.setparams((nchannels,
                        framerate,
                        nframes,
                        comptype,
                        compname))
    frames = []
    for s in data:
        mul = int(s * amplitude)
        frames.append(struct.pack('h', mul))

    frames = ''.join(frames)
    wavfile.writeframes(frames)
    wavfile.close()
    print("%s written" %(filename))

ser = serial.Serial('/dev/cu.usbmodem0000000000001',baudrate=115200, timeout=None)
#ser.flushInput()
music = []
sample_rate = 5000
samples = 10
print("Capturing")
# for s in range(samples):
#     ser.write("g".encode())
#     ser_bytes = ser.readline()
#     response.append(ser_bytes)

for i in range(sample_rate*samples):
    ser_bytes = ser.readline()
    # bytesToRead = ser.inWaiting()
    music.append(ser_bytes)

print("Writing")

for samp in music:
    with open("audio_data.csv", "a") as f:
        writer = csv.writer(f, delimiter=",")
        writer.writerow([time.time(), samp.decode().lstrip().rstrip()])

print("Making Waves>")

fname = "audio_data.csv"
data = []
for time, value in csv.reader(open(fname), delimiter=','):
    try:
        data.append(float(value))
    except ValueError:
        pass

arr = numpy.array(data)
arr /= numpy.max(numpy.abs(data))
filename_head, extension = fname.rsplit(".", 1)
data_resampled = resample(arr, len(data))
wavfile.write('rec.wav', sample_rate, data_resampled)
print ("File written succesfully !")
