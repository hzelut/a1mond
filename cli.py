#!/usr/bin/env python3
import sys
import socket

SOCK_PATH = "/tmp/almond.sock"

def send_command(cmd):
  with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as client:
    client.connect(SOCK_PATH)
    client.sendall(cmd.encode())
    res = client.recv(1024)
    return res.decode()

def main():
  if len(sys.argv) < 2:
    print(f'Usage: {sys.argv[0]} <command>')
    return

  try:
    res = send_command(' '.join(sys.argv[1:]))
    print(res)
  except Exception as e:
    print(e)

if __name__ == '__main__':
  main()
