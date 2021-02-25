import wave
import struct
import sys
import csv
import numpy
from scipy.io import wavfile
from scipy.signal import resample
#test
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


if __name__ == "__main__":
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
    wavfile.write('rec.wav', 10000, data_resampled)
    print ("File written succesfully !")