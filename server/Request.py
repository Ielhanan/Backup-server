import struct
import dataBases

VERSION=3
REQUEST_CODES={
    "REGISTER" : 1100,
    "PUBLICKEY": 1101,
    "SENDFILE" : 1103,
    "FIXCRC": 1104,
    "ERRCRC": 1105,
    "ERR4CRC": 1106,
    "HEADER_SIZE":23,
}

class RequestHeader :
    def __init__(self):
        self.version=VERSION


    def unpackRequestHeader(self,buffer,conn):
        #unpack header
         failcounter = 0
         offset=16#size of clientID
         prevOffset=0
         self.clientID=struct.unpack('<16s',buffer[prevOffset:offset])[0]
         prevOffset=offset
         offset+=1
         self.version = struct.unpack('<B', buffer[prevOffset:offset])[0]
         prevOffset = offset
         offset += 2
         self.code = struct.unpack('<h', buffer[prevOffset:offset])[0]
         prevOffset = offset
         offset += 4
         self.payloadSize = struct.unpack('<I', buffer[prevOffset:offset])[0]
         offset=0

        #unpack payloads depand on the request code

         if self.code == REQUEST_CODES["REGISTER"]:
             buffer=conn.recv(self.payloadSize)
             self.payloadName=buffer[offset:].decode('utf-8')

         elif self.code == REQUEST_CODES["PUBLICKEY"]:
             buffer = conn.recv(self.payloadSize)
             prevOffset=offset
             offset+=255
             self.payloadName=buffer[prevOffset:offset].decode('utf-8')
             prevOffset=offset
             offset+=160
             self.payloadPublicKey=buffer[prevOffset:offset]
         elif self.code == REQUEST_CODES["SENDFILE"]:
             buffer = conn.recv(self.payloadSize)
             while len(buffer) < self.payloadSize:#get all the file content
                 len("NOW BUF"+str(len(buffer)))
                 buffer+=conn.recv(self.payloadSize-len(buffer))
             prevOffset = offset
             offset += 16
             self.payloadClientID=struct.unpack('<16s',buffer[prevOffset:offset])[0]
             prevOffset = offset
             offset += 4
             self.contentSize=struct.unpack('<I',buffer[prevOffset:offset])[0]
             prevOffset = offset
             offset += 255
             self.fileName=buffer[prevOffset:offset].decode('utf-8').split("\x00")[0]
             prevOffset=offset
             self.messegeContent = buffer[prevOffset:]
             #now we save the file in beckup and data base
             dataBases.saveFileInBeckup(self)
             dataBases.saveFileInDataBase(self)
         elif self.code == REQUEST_CODES["FIXCRC"]:
             #the current req is fixcrc so we need to update the verify field in db
             buffer = conn.recv(self.payloadSize)
             prevOffset = offset
             offset += 16
             self.payloadClientID= struct.unpack('<16s',buffer[prevOffset:offset])[0]
             prevOffset = offset
             offset += 255
             self.fileName=buffer[prevOffset:offset].decode('utf-8').split("\x00")[0]
             dataBases.verifyFile(self)
             print(f"CRC FIX File beckuped succsessfuly filename: {self.fileName} clientID:{self.clientID.hex()} ")
         elif self.code == REQUEST_CODES["ERRCRC"]:
             buffer = conn.recv(self.payloadSize)
             failcounter+=1
             prevOffset = offset
             offset += 16
             self.payloadClientID= struct.unpack('<16s',buffer[prevOffset:offset])[0]
             prevOffset = offset
             offset += 255
             self.fileName=buffer[prevOffset:offset].decode('utf-8').split("\x00")[0]
             print(f"CRCERROR please resend the file filename: {self.fileName} clientID:{self.clientID.hex()}")
         elif self.code == REQUEST_CODES["ERR4CRC"]:
             buffer = conn.recv(self.payloadSize)
             prevOffset = offset
             offset += 16
             self.payloadClientID= struct.unpack('<16s',buffer[prevOffset:offset])[0]
             prevOffset = offset
             offset += 255
             self.fileName=buffer[prevOffset:offset].decode('utf-8').split("\x00")[0]
             print(f"CRC 3 TIME ERROR filename: {self.fileName} clientID:{self.clientID.hex}")
             #if 3 time fail we want to delete the file from beckup and DB
             dataBases.deleteFileFromDataBase(self)
             dataBases.deleteFileInBeckup(self)

