import os
import struct
import crc
import Request
import dataBases


AES_KEY_SIZE=16
VERSION=3
RESPONSE_CODES={
    "REGISTER_SUCCSESS" : 2100,
    "PUBLICKEY_SUCCSESS": 2102,
    "SENDFILE_SUCCSESS" : 2103,
    "MESSEGE_SUCCSESS": 2104,
    "REGISTER_FAIL"  : 2101
}



class ResponseHeader :
    def __init__(self):
        self.version=VERSION


    def packResponse(self,request):
        replayPacket=None
        #each case pack relevant payload depands on previous request
        if request.code==Request.REQUEST_CODES["REGISTER"]:  #case request was register
            self.code=RESPONSE_CODES["REGISTER_SUCCSESS"]
            self.payloadSize=16
            if dataBases.checkIfExist(request.payloadName)==True:
                #checking if client is already exist and send respond 2101
                self.code=RESPONSE_CODES["REGISTER_FAIL"]
                print("Register failed please try again ")
                self.payloadSize = 0
                replayPacket = struct.pack(f"<BhI", self.version, self.code, self.payloadSize)
            else:#register new client and send respond with new CLIENTid
                self.payloadClientID = dataBases.creatUUID(request.payloadName)
                dataBases.insertNewClient(self.payloadClientID,request.payloadName)
                clientNamePrint=request.payloadName.split('\x00')[0]
                print(f"""Succsessful register User name:{clientNamePrint} with ID : {self.payloadClientID} """)
                replayPacket=struct.pack(f"<BhI",self.version,self.code,self.payloadSize)
                replayPacket+=bytes.fromhex(self.payloadClientID)
        if request.code==Request.REQUEST_CODES["PUBLICKEY"]:
            #if previous req was public key we generate AES and send it encrypted
            self.code=RESPONSE_CODES["PUBLICKEY_SUCCSESS"]
            AES=os.urandom(AES_KEY_SIZE)
            self.payloadClientID=request.clientID
            self.AES=dataBases.encryptPubkey(request.payloadPublicKey,AES)
            self.payloadSize=len(self.AES)+len(self.payloadClientID)
            dataBases.updateDataBase(AES,request.payloadPublicKey.hex(),self.payloadClientID.hex())
            replayPacket=struct.pack(f"<BhI",self.version,self.code,self.payloadSize)
            replayPacket+=self.payloadClientID
            replayPacket+=self.AES
        if request.code==Request.REQUEST_CODES["SENDFILE"]:
            #the file has arrived , we calc cksum and send back to client to ensure the file has arrived good
            print(f"File sent succsessful , file name from client path :{request.fileName} client id :{request.clientID.hex()}")
            self.clientID=request.clientID
            self.code=RESPONSE_CODES["SENDFILE_SUCCSESS"]
            self.contentSize=request.contentSize
            self.fileName=request.fileName
            serverFileName=os.path.basename(self.fileName)
            self.CKsum=crc.run(f'files\\{request.clientID.hex()}\\{serverFileName}')
            self.payloadSize=279 #16 for id 4 for content size 255 for filepath 4 for cksum
            replayPacket = struct.pack(f"<BhI", self.version, self.code, self.payloadSize)
            replayPacket+=struct.pack("<16s",request.clientID)
            replayPacket+=struct.pack(f"<I",self.contentSize)
            replayPacket+=struct.pack("<255s ",request.fileName.encode('utf-8'))
            replayPacket+=struct.pack(f"I",self.CKsum)
        if request.code==Request.REQUEST_CODES["FIXCRC"]:
            print(f"Thanks for beckup file  {request.fileName}")
            self.code=RESPONSE_CODES["MESSEGE_SUCCSESS"]
            self.payloadSize=0
            replayPacket = struct.pack(f"<BhI", self.version, self.code, self.payloadSize)
        return replayPacket

