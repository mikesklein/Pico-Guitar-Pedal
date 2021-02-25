import pyaudio
import numpy as np
import wave

form_1 = pyaudio.paInt16
CHUNK = 4096 # number of data points to read at a time
RATE = 44100 # time resolution of the recording device (Hz)
wav_output_filename = 'test2.wav'
chans = 1
record_secs = 10     #record time
dev_index = 2


frames = []

p = pyaudio.PyAudio() # start the PyAudio class

# try:
#     default_device_index = p.get_default_input_device_info()  # Set default to first in list or ask Windows
# except IOError:
#     default_device_index = -1
#
# print("Available devices:\n")  # Select Device
# for i in range(0, p.get_device_count()):
#     info = p.get_device_info_by_index(i)
#     print(str(info["index"]) + \
#           ": \t %s \n \t %s \n" % (info["name"], \
#           p.get_host_api_info_by_index(info["hostApi"])["name"]))
#
#     if default_device_index == -1:
#         default_device_index = info["index"]


stream = p.open(format=pyaudio.paInt16,channels=1,rate=RATE,input=True,
              frames_per_buffer=CHUNK) #uses default input device

# create a numpy array holding a single read of audio data
for i in range(100): #to it a few times just to see
    data = np.frombuffer(stream.read(CHUNK),dtype=np.int16)
    frames.append(data)

# for i in range(int(10*44100/1024)): #go for a few seconds
#     data = np.frombuffer(stream.read(CHUNK),dtype=np.int16)
#     peak=np.average(np.abs(data))*2
#     bars="#"*int(50*peak/2**16)
#     print("%04d %05d %s"%(i,peak,bars))

# close the stream gracefully
stream.stop_stream()
stream.close()
p.terminate()

wavefile=wave.open(wav_output_filename,'wb')
wavefile.setnchannels(chans)
wavefile.setsampwidth(p.get_sample_size(form_1))
wavefile.setframerate(RATE)
wavefile.writeframes(b''.join(frames))
wavefile.close()
