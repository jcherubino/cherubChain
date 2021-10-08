#Quick test script to allow for socket testing on localhost
import socket

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

s.connect(("localhost", 33333))

s.send(bytearray([0xFF]));

print(int.from_bytes(s.recv(1), byteorder='little'))
