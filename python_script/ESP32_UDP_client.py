import socket
import pyaudio
import threading
import time

"""
    This is a simple UDP client that sends audio data to a server and receives audio data from the server.
    you can use this script as the android APP to communicate with the ESP32_UDP_server
"""


server_ip = '10.192.12.146'
server_port = 5555
chunk = 512

audio = pyaudio.PyAudio()
stream_in = audio.open(format=pyaudio.paInt16, channels=1, rate=16000, input=True, frames_per_buffer=chunk)
stream_out = audio.open(format=audio.get_format_from_width(2), channels=1,
                        rate=16000, output=True, frames_per_buffer=chunk)
# message to response
message = 'Hello, UDP Server!'

# create UDP socket
client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)


client_socket.settimeout(0.5)


def input_thread():
    while True:
        # send message to server
        frames = stream_in.read(chunk)
        client_socket.sendto(frames, (server_ip, server_port))
        print("Sent frames")


def output_thread():
    while True:
        try:
            # receive response from server
            response, server_address = client_socket.recvfrom(chunk*2)
            stream_out.write(response)
            print(f'Received response from {server_address}')
        except socket.timeout:
            print('Time out, no response')


thread1 = threading.Thread(target=input_thread)
thread2 = threading.Thread(target=output_thread)
thread1.start()
thread2.start()
