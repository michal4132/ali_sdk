import serial
import time
import sys


def slow_write(w):
  for i in w:
    if(isinstance(i, bytes)):
        ser.write(i)
    else:
        ser.write(bytes([i]))
    time.sleep(0.001)

cmd = None

def write_binary():
    print("write binary")
    f = open("build/ledzior.bin", "rb")
    data = f.read()
    slow_write(data)
    print("write complete")

def read_uart():
    out = ser.read(1000)
    if(out == b"write\nenter size: "):
        wsize = input("enter size >").encode("ascii")
        slow_write(wsize + b"\n")
        write_binary()
    else:
        print(out)


ser = serial.Serial("/dev/ttyUSB0", 115200, xonxoff=True, bytesize=serial.EIGHTBITS ,stopbits=serial.STOPBITS_ONE ,timeout=1, parity=serial.PARITY_EVEN)

run = True
while run:
    try:
        cmd = input(">").encode("ascii")
        if(cmd == b"r"):
            read_uart()
        else:
            print(cmd)
            slow_write(cmd + b"\r")
            read_uart()
    except KeyboardInterrupt:
        run = False

ser.close()

# DECODE PACKET
# print("----------------------------------")

# packet_header = b'HEAD\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x05\xd1\x83\xe0\xc7\x01\x00\x00\x00!\x92\xffwF'

# print(f"packet: {packet_header}")

# readCRC = packet_header[PACKET_HEAD_CRC_OFFSET:PACKET_HEAD_CRC_OFFSET+4]
# #readCRC = fetch_long(readCRC)
# print(f"read CRC: {readCRC}")

# nCRC = MG_Table_Driven_CRC(0xFFFFFFFF, packet_header[PACKET_TYPE_OFFSET:], PACKET_HEAD_LEN - 8)
# print(f"decoded header CRC: {nCRC}")

# p = DecodePacketHead(packet_header)
# print(f"decoded header: {p}")

# sys.exit(0)
# print("start comtest")
# slow_write(b"comtest")
# slow_write(b"xd\n")
# out = ser.readline()
# print(out)
# out = ser.readline()
# print(out)

# ser.close()
