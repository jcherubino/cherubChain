#Quick test script to allow for socket testing on localhost
import socket

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

s.connect(("localhost", 33333))

#get first 2 bytes as payload len. network = big byte order
recvd = s.recv(2)

#specify big endian at same time to reverse byte order to native.
payload_len = int.from_bytes(recvd, byteorder="big", signed=False)
print(f"Payload length: {payload_len}")

#+8 for prev hash and hash
recvd = s.recv(payload_len+8)

prev_hash = int.from_bytes(recvd[:4], byteorder='big')
print(f"Prev hash: {prev_hash}")

cur_hash = int.from_bytes(recvd[4:8], byteorder='big')
print(f"Hash: {cur_hash}")

print(recvd[8:])
