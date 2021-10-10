#Quick test script to allow for socket testing on localhost
import socket

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

s.connect(("localhost", 33333))

#get first 4 bytes as block len. network = big byte order
recvd = s.recv(4)

#specify big endian at same time to reverse byte order to native.
block_len = int.from_bytes(recvd, byteorder="big", signed=False)
print(f"Remaining block bytes: {block_len}")

recvd = s.recv(block_len)

prev_hash = int.from_bytes(recvd[:4], byteorder='big')
print(f"Prev hash: {prev_hash}")

cur_hash = int.from_bytes(recvd[4:8], byteorder='big')
print(f"Hash: {cur_hash}")

plen = int.from_bytes(recvd[8:10], byteorder='big')
print(f"Payload length: {plen}")

print(recvd[10:])
