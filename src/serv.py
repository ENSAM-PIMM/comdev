# Echo server program
import socket

print 'started'
HOST = ''                 # Symbolic name meaning the local host
PORT = 1707              # Arbitrary non-privileged port
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT)) # start on this port
s.listen(1)  # listen to the new port
conn, addr = s.accept()
print 'Connected by', addr
while 1:
    data = conn.recv(1024)
    if not data: break
    conn.send(data)
conn.close()
