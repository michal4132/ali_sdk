import serial
import time
import sys

# PACKET HEADER 19 bytes + data
# [ 4  ][ 1  ][    1     ][  4  ][  1   ][ 4 ][ 4 ]   [LEN ][ 4 ]
# [HEAD][TYPE][BLANK FLAG][INDEX][REPEAT][LEN][CRC]   [DATA][CRC]

# PACKET TYPE
PACKET_COMMAND = 0 
PACKET_DATA = 1
PACKET_REQUEST = 2
PACKET_STATUS = 3

# PACKET STATUS
COMMAND_STATUS_OK = 0
COMMAND_STATUS_ERROR = 1
COMMAND_STATUS_RUNNING = 2
COMMAND_STATUS_EXECUTED = 3
COMMAND_STATUS_CANCEL = 4

# PACKET PARAMS
PACKET_HEAD_LEN = 19
PACKET_TYPE_OFFSET = 4
PACKET_BLANKFLAG_OFFSET = 5
PACKET_INDEX_OFFSET = 6
PACKET_REPEAT_OFFSET = 10
PACKET_LENGTH_OFFSET = 11
PACKET_HEAD_CRC_OFFSET = 15

crc_type = 2
MG_CRC_Table = [None] * 256
MG_CRC_24_CCITT = 0x00ddd0da
MG_CRC_24 = 0x00ad85dd
MG_CRC_32_CCITT = 0x04c11db7
MG_CRC_32 = 0xedb88320

# From "len" length "buffer" data get CRC, "crc" is preset value of crc 
def MG_Compute_CRC(crc, buf, len):
    index = 0

    if(crc_type == 0):
        while(len != 0):  # Length limited
            len -= 1
            crc ^= buf[index] << 16
            index += 1
            for i in range(8):
                if(crc & 0x00800000):	# Highest bit procedure
                    crc = (crc << 1) ^ MG_CRC_24_CCITT
                else:
                    crc <<= 1
        return (crc & 0x00ffffff)  # Get lower 24 bits FCS

    elif(crc_type == 1):
        while(len != 0):  # Length limited
            len -= 1
            crc ^= buf[index]
            index += 1
            for i in range(8):
                if(crc & 1):
                    crc = (crc >> 1) ^ MG_CRC_24
                else:
                    crc >>= 1
        return (crc & 0x00ffffff)  # Get lower 24 bits FCS

    elif(crc_type == 2):
        while(len != 0):  # Length limited
            len -= 1
            crc ^= buf[index] << 24
            index += 1
            for i in range(8):
                if(crc & 0x80000000): # Highest bit procedure
                    crc = (crc << 1) ^ MG_CRC_32_CCITT
                else:
                    crc <<= 1
        return (crc & 0xffffffff) # Get lower 32 bits FCS

    elif(crc_type == 3):
        while(len != 0): # Length limited
            len -= 1
            crc ^= buf[index]
            index += 1
            for i in range(8):
                if(crc & 1): # Lowest bit procedure
                    crc = (crc >> 1) ^ MG_CRC_32
                else:
                    crc >>= 1
        return (crc & 0xffffffff) # Get lower 32 bits FCS

# Setup fast CRC compute table
def MG_Setup_CRC_Table():
    zero = [0]

    for count in range(256):
        if(crc_type == 0):
            MG_CRC_Table[count] = MG_Compute_CRC(count << 16, zero, 1)

        elif(crc_type == 1):
            MG_CRC_Table[count] = MG_Compute_CRC(count, zero, 1)

        elif(crc_type == 2):
            MG_CRC_Table[count] = MG_Compute_CRC(count << 24, zero, 1)

        elif(crc_type == 3):
            MG_CRC_Table[count] = MG_Compute_CRC(count, zero, 1)

# Fast CRC compute
def MG_Table_Driven_CRC(crc, bufptr, len):
    for i in range(len):
        if(crc_type == 0):
            crc=(MG_CRC_Table[((crc >> 16) & 0xff) ^ bufptr[i]] ^ (crc << 8)) & 0x00ffffff

        elif(crc_type == 1):
            crc=(MG_CRC_Table[(crc & 0xff) ^ bufptr[i]] ^ (crc >> 8)) & 0x00ffffff

        elif(crc_type == 2):
            crc=(MG_CRC_Table[((crc >> 24) & 0xff) ^ bufptr[i]] ^ ((crc << 8) & 0xFFFFFFFF)) & 0xffffffff

        elif(crc_type == 3):
            crc=(MG_CRC_Table[(crc & 0xff) ^ bufptr[i]] ^ (crc >> 8)) & 0xffffffff

    return crc

def fetch_long(p):
    return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];

def store_long(v):
    p = [None] * 4 
    p[0] = (v >> 24) & 0xFF
    p[1] = (v >> 16) & 0xFF
    p[2] = (v >> 8) & 0xFF
    p[3] = v & 0xFF
    return p

def MakePacketHeadBuffer(packet):
    packet_header = [0] * PACKET_HEAD_LEN
    # set packet header information
    packet_header[0] = b"H"
    packet_header[1] = b"E"
    packet_header[2] = b"A"
    packet_header[3] = b"D"
    packet_header[PACKET_TYPE_OFFSET] = packet["type"]
    packet_header[PACKET_BLANKFLAG_OFFSET] = packet["blank_flag"]

    packet_index = store_long(packet["index"])
    packet_header[PACKET_INDEX_OFFSET+0] = packet_index[0]
    packet_header[PACKET_INDEX_OFFSET+1] = packet_index[1]
    packet_header[PACKET_INDEX_OFFSET+2] = packet_index[2]
    packet_header[PACKET_INDEX_OFFSET+3] = packet_index[3]

    packet_header[PACKET_REPEAT_OFFSET] = packet["repeat"]

    packet_len = store_long(packet["length"])
    packet_header[PACKET_LENGTH_OFFSET+0] = packet_len[0]
    packet_header[PACKET_LENGTH_OFFSET+1] = packet_len[1]
    packet_header[PACKET_LENGTH_OFFSET+2] = packet_len[2]
    packet_header[PACKET_LENGTH_OFFSET+3] = packet_len[3]

    # print("len:", PACKET_HEAD_LEN - 8)
    # print(packet_header[PACKET_TYPE_OFFSET:])
    nCRC = MG_Table_Driven_CRC(0xFFFFFFFF, packet_header[PACKET_TYPE_OFFSET:], PACKET_HEAD_LEN - 8)
    packet_nCRC = store_long(nCRC)
    # print(nCRC, packet_nCRC)
    packet_header[PACKET_HEAD_CRC_OFFSET+0] = packet_nCRC[0]
    packet_header[PACKET_HEAD_CRC_OFFSET+1] = packet_nCRC[1]
    packet_header[PACKET_HEAD_CRC_OFFSET+2] = packet_nCRC[2]
    packet_header[PACKET_HEAD_CRC_OFFSET+3] = packet_nCRC[3]
    return packet_header

def DecodePacketHead(packet_head):
    packet = dict()

    # set packet head information
    packet["type"] = packet_head[PACKET_TYPE_OFFSET]
    packet["blank_flag"] = packet_head[PACKET_BLANKFLAG_OFFSET]

    packet["index"] = fetch_long(packet_head[PACKET_INDEX_OFFSET:])
    packet["repeat"] = packet_head[PACKET_REPEAT_OFFSET]

    packet["length"] = fetch_long(packet_head[PACKET_LENGTH_OFFSET:])
    return packet

ser = serial.Serial("/dev/ttyUSB0", 115200, timeout=2, parity=serial.PARITY_EVEN)

MG_Setup_CRC_Table()
#print(MG_CRC_Table)

def slow_write(w):
  for i in w:
    if(isinstance(i, bytes)):
        ser.write(i)
    else:
        ser.write(bytes([i]))
    # time.sleep(0.00001)


def send_packet(packet_header, packet):
    data_nCRC = MG_Table_Driven_CRC(0xFFFFFFFF, packet["data"], packet["length"])
    data_nCRC = store_long(data_nCRC)
    slow_write(packet_header)
    if(packet["length"] > 0):
        slow_write(packet["data"])
        slow_write(data_nCRC)

def read_uart():
    time.sleep(0.5)
    toRead = ser.inWaiting()
    out = ser.read(toRead)
    if(len(out) > 5 and out[0:4] == b"HEAD"):
        packet = DecodePacketHead(out)
        packet["data"] = out[PACKET_HEAD_LEN:-4]

        if(packet["type"] == PACKET_COMMAND):
            print("type: COMMAND")
        elif(packet["type"] == PACKET_DATA):
            print("type: DATA")
        elif(packet["type"] == PACKET_REQUEST):
            print("type: REQUEST")
        elif(packet["type"] == PACKET_STATUS):
            print("type: STATUS")

        print(f"length: {packet['length']}")
        print(f"index: {packet['index']}")
        print(f"blank: {packet['blank_flag']}")
        print(f"repeat: {packet['repeat']}")
        if(packet["type"] == PACKET_STATUS):
            status = packet['data'][0]
            if(status == COMMAND_STATUS_OK):
                print("Status: OK")
            elif(status == COMMAND_STATUS_ERROR):
                print("Status: Error")
            elif(status == COMMAND_STATUS_RUNNING):
                print("Status: Running")
            elif(status == COMMAND_STATUS_EXECUTED):
                print("Status: Executed")
            elif(status == COMMAND_STATUS_CANCEL):
                print("Status: Cancel")

            print(f"data: {packet['data'][1:]}")
        else:
            print(f"data: {packet['data']}")
        print("-----------")

    else:
        print(out)

packet = {
        "type": 0, "blank_flag": 0, "index": 0, "repeat": 0, "length": 0,
        "data": b""
    }


while True:
    cmd = input(">").encode("ascii")
    if(cmd == b"c"):
        ser.write(b"c")
        print("OK")
        toRead = ser.inWaiting()
        out = ser.read(toRead)
        print(out)
    elif(cmd == b"get"):
        # packet["type"] = PACKET_COMMAND
        # packet["data"] = b"dump 16 16\n"
        # packet["length"] = len(packet["data"])
        # packet_header = MakePacketHeadBuffer(packet)
        # send_packet(packet_header, packet)
        # print(packet)    
        # read_uart()
        print("GET")
        packet["type"] = PACKET_REQUEST
        packet["blank_flag"] = 0
        packet["data"] = b"\x00\x00\x00\x01\x00"
        packet["length"] = 5
        packet_header = MakePacketHeadBuffer(packet)
        send_packet(packet_header, packet)
        read_uart()
        # packet["index"] += 1
    elif(cmd == b"abort"):
        packet["type"] = PACKET_STATUS
        packet["data"] = b"\x04\x00\x00\x00\x00"
        packet["length"] = 5
        packet_header = MakePacketHeadBuffer(packet)
        send_packet(packet_header, packet)
        read_uart()
        packet["index"] += 1
    elif(cmd == b"dupa"):
        out = b"c\rG"
        slow_write(out)
        read_uart()
        packet["index"] += 1
    elif(cmd == b"i"):
        packet["index"] += 1
    else:
        packet["type"] = PACKET_COMMAND
        packet["data"] = cmd + b"\n"
        packet["length"] = len(cmd) + 1
        packet_header = MakePacketHeadBuffer(packet)
        send_packet(packet_header, packet)
        print(packet)    
        read_uart()

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
