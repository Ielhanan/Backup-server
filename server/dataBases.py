import sqlite3
import datetime
from Crypto.Cipher import AES, PKCS1_OAEP
from Crypto.PublicKey import RSA
import os
import  uuid
from Crypto.Util.Padding import  unpad

EMPTY=0


def createDataBase():
    data_base=sqlite3.connect("SERVER.db")
    data_base.text_factory = bytes
    try:
        data_base.execute("""CREATE TABLE clients (ID CHAR(16) NOT NULL
     PRIMARY KEY , name TEXT(127),publicKey CHAR(160) ,lastSeen DATETIME, AES BLOB(16));""")
        data_base.execute("""CREATE TABLE files (ID CHAR(16) NOT NULL ,fileName VARCHAR(255) NOT NULL ,pathName VARCHAR(255),
        verified BOOL,PRIMARY KEY(pathName));""")
    except:
        print("Table was not created. Possibly already exists.")

    data_base.commit()
    data_base.close()

#updating data base we new information
def updateDataBase(AES,publicKey,clientID):
    try:
        data_base = sqlite3.connect("SERVER.db")
        data_base.text_factory = bytes
        querry = """UPDATE clients SET publicKey=? ,AES=? ,lastSeen=? WHERE ID=?"""
        data_base.execute(querry,[publicKey,AES,datetime.datetime.now(),clientID])
        data_base.commit()
        data_base.close()
    except:
        print("Error updat data base with new public key and AES key")

#insert new client to data base
def insertNewClient(clientID,clientName):
    try:
        data_base=sqlite3.connect("SERVER.db")
        data_base.text_factory = bytes
        clientName=clientName.split('\00')[0]
        querry="""INSERT INTO clients VALUES (?,?,NULL,?,NULL);"""
        data_base.execute(querry,[clientID,clientName,datetime.datetime.now()])
        data_base.commit()
        data_base.close()
    except:
        print("Error to insert new client")


#check if client is alredy exist.
def checkIfExist(clientName):
    try:
        data_base = sqlite3.connect("SERVER.db")
        data_base.text_factory = bytes
        curr=data_base.cursor()
        clientName = clientName.split('\x00')[0]
        querry = """SELECT name FROM clients WHERE name=?"""
        curr.execute(querry,[clientName])
        result=curr.fetchall()
        data_base.commit()
        data_base.close()
        if(len(result) == EMPTY): #the client is not exist
            return False
        else :
            return True
    except:
        print("Error chekc if exist client in DB")

#return AES key for sepecific client
def getAESkey(clientID):
    try:
        data_base = sqlite3.connect("SERVER.db")
        data_base.text_factory = bytes
        curr=data_base.cursor()
        querry = """SELECT AES FROM clients WHERE ID=?"""
        curr.execute(querry,[clientID])
        result=curr.fetchall()
        data_base.commit()
        data_base.close()
        if(len(result) == EMPTY):
            return None
        else :
            return result[0]
    except:
        print("Error get AES key from DB")

#generate new UUID
def creatUUID(name):
    uniqeID=uuid.uuid4().hex
    return uniqeID


#save file in beckup folder under client folder
def saveFileInBeckup(request):
    try:
        fileName=request.fileName
        if os.path.isdir('files')!=True:#checking if exist file
            os.mkdir('files')
        if os.path.isdir(f'files\{request.clientID.hex()}') != True:#check if exist the client folder
            os.mkdir(f'files\{request.clientID.hex()}')
        if os.path.basename(request.fileName)!=request.fileName:#get the file name withoud the path
            fileName=os.path.basename(request.fileName)
        path = f'files\{request.clientID.hex()}\\' + fileName
        file = open(path, 'wb')
        decryptedFileContet = decrypt(request)
        file.write((decryptedFileContet))
        file.close()
    except:
        print("Error save file in beckup")

#decrypt the file content with the aes key
def decrypt(request):
    try:
        AESkey = getAESkey(request.clientID.hex())[0]
        iv=bytearray(16)
        cipher = AES.new(AESkey,AES.MODE_CBC,iv=iv)
        decryptedFileContent = cipher.decrypt(request.messegeContent)
        return unpad(decryptedFileContent,AES.block_size)
    except:
        print("Error decrypt file content")

#encrypt the text with the pubkey
def encryptPubkey(publicKey,text):
    try:
        rsa_pubkey= RSA.importKey(publicKey)
        rsa_pubkey = PKCS1_OAEP.new(rsa_pubkey)
        return rsa_pubkey.encrypt(text)
    except:
        print("Error encrypt with public key")


#save file in data base with beckup file path
def saveFileInDataBase(request):
    try:
        data_base=sqlite3.connect("SERVER.db")
        data_base.text_factory = bytes
        path = f'files\{request.clientID.hex()}\\' + request.fileName
        curr = data_base.cursor()
        #checking if the file is alredy exist if yes we print that the file has been overwrite
        checkIfExistQurry = """SELECT pathName FROM files WHERE pathName=?"""
        curr.execute(checkIfExistQurry,[path])
        result=curr.fetchall()
        if len(result)==0: #if the file is not exist in DB we insert it
            querry="""INSERT INTO files VALUES (?,?,?,?);"""
            data_base.execute(querry,[request.clientID.hex(),request.fileName,path,False])
        else:
            print("file has been overwrite")
        data_base.commit()
        data_base.close()
    except:
        print("Error saving file the in the data base")

#update the verify field after get crcfix
def verifyFile(request):
    try:
        data_base = sqlite3.connect("SERVER.db")
        data_base.text_factory = bytes
        pathName=f'files\{request.clientID.hex()}\\' + request.fileName
        querry = """UPDATE files SET verified=?  WHERE   pathName=?"""
        data_base.execute(querry,[True,pathName])
        data_base.commit()
        data_base.close()
    except:
        print(f"Error updat verify field at DB  file name={request.fileName} ")

#remove the file if crc fail 3 times
def deleteFileFromDataBase(request):
    try:
        data_base = sqlite3.connect("SERVER.db")
        data_base.text_factory = bytes
        pathName=f'files\{request.clientID.hex()}\\' + request.fileName
        querry = """DELETE FROM files  WHERE   pathName=?"""
        data_base.execute(querry,[pathName])
        data_base.commit()
        data_base.close()
    except:
        print(f"Error delete file from data base file name :{request.fileName}")


#remove file from beckup
def deleteFileInBeckup(request):
    try:
        fileName=request.fileName
        if os.path.basename(request.fileName)!=request.fileName:
            fileName=os.path.basename(request.fileName)
        path = f'files\{request.clientID.hex()}\\' + fileName
        if os.path.exists(path):
            os.remove(path)
    except:
        print(f"Error remove file in server beckup file name:{request.fileName}")




