#!/usr/bin/env python
import struct
import sys

def read_uint8(file):
    return file.read(1)

def read_uint32(file):
    return struct.unpack(">I", file.read(4))[0]


class Extractor:
    def __init__(self, path):
        self.f = open(path, "rb")
        self.next_partition_offset = 0
        self.first = True

    def __del__(self):
        self.f.close()

    def read_block(self):
        self.f.seek(self.next_partition_offset)
        block_offset = self.next_partition_offset
        block_magic = read_uint32(self.f)
        block_size = read_uint32(self.f)
        self.next_partition_offset += read_uint32(self.f)
        block_crc = read_uint32(self.f)

        if(block_size == 0 and self.first):
            block_size = self.next_partition_offset

        print(f"magic: {block_magic}")
        print(f"size: {block_size}")
        print(f"next offset: {self.next_partition_offset}")


        block_name = self.f.read(16).replace(b"\xFF", b"\x00").decode().rstrip("\x00")
        block_version = self.f.read(16).replace(b"\xFF", b"\x00").decode().rstrip("\x00")
        block_date = self.f.read(16).replace(b"\xFF", b"\x00").decode().rstrip("\x00")

        print(f"block name: {block_name}")
        print(f"block version: {block_version}")
        print(f"block date: {block_date}")

        print(f"pos: {self.f.tell()}")

        if(block_size != 0):
            self.f.seek(block_offset)
            data = self.f.read(block_size)
            out_file = open(f"out/{block_name}.bin", "wb")
            out_file.write(data)
            out_file.close()
            self.f.seek(block_offset)

        # TODO: read actual data here

        print("--------------------------------")
        self.first = False

    def is_next_block(self):
        return (self.next_partition_offset > self.f.tell() or self.first)
            

def main():
    if(len(sys.argv) <= 1):
        print("no input file")
        return 

    input_file = sys.argv[1]

    extractor = Extractor(input_file)
    while(extractor.is_next_block()):
        extractor.read_block()

if __name__ == "__main__":
    main()
