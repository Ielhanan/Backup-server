# C++ and Python Project: Backup Server & Client

## About
This project represents a backup server .Each client can register to the server and send files.The server recieves the files and stores them in a unique
file path for each client.
The files are  encrypted before sending and decrypted within the server. 

## Server
* Developed with python 3.9.9.
* Supports multiclient by  multithreading
* Applied Crypto library for encryption and decryption

## Client
* Developed with Visual Studio 2019.
* Client code written with ISO C++14 Standard (Default by Visual Studio 2019).
* Crypto++ Library 8.5 is used. https://www.cryptopp.com
* <b>These instructions apply for x64 configuration, upon loading .sln, use x86 build!</b>

## Configure cryptopp

 Get Crypto++
* Download the copy of Crypto++ for Windows platform via https://www.cryptopp.com/#download. (ZIP).
* Extract the archive file to your directory of choice. Example path: <i>"D:\cryptopp850\"</i>

 Complie Crypto++ library
* Open <i>"D:\cryptopp850\cryptest.sln"</i> with Visual Studio.
* Build the solution. Make sure build configuration matches. (For example, Debug, Win32).
* Close the solution.
* We will use the static library <i>cryptlib.lib</i>. (If Win32, Debug was built, the library will be located within <i>Win32\Output\Debug</i>).


## Protocol information
### Request
#### request header
| Field | Size | Meaning | 
|---------|---------|---------|
| ClientID | 16 bytes | uniq id for client | 
| version | 1 byte | version of server |
| code | 2 bytes | number of code request | 
|payload size | 4 bytes | payload size | 

#### code 1100 registeration

| Field | Size | Meaning |
|---------|---------|---------|
|Name| 255 bytes | client name |

#### code 1101 send public key

| Field | Size | Meaning |
|---------|---------|---------|
|Name| 255 bytes | client name |
| public key | 160 bytes | client public key |

#### code 1103 send file 

| Field | Size | Meaning |
|---------|---------|---------|
|client ID| 16 bytes | client ID |
| content size | 4 bytes | file content size after encryption |
| file name | 255 bytes | file name that sent |
| message content | dynamic | file content  |

#### code 1104 fixed CRC

| Field | Size | Meaning |
|---------|---------|---------|
|client ID| 16 bytes | client ID |
| file name | 255 bytes | file name that sent |

#### code 1105 error CRC

| Field | Size | Meaning |
|---------|---------|---------|
|client ID| 16 bytes | client ID |
| file name | 255 bytes | file name that sent |

#### code 1106 4th fail crc

| Field | Size | Meaning |
|---------|---------|---------|
|client ID| 16 bytes | client ID |
| file name | 255 bytes | file name that sent |



## Response
#### respons header


| Field | Size | Meaning | 
|---------|---------|---------|
| version | 1 bytes | version of server  | 
| code | 2 bytes | code of respond | 
| payload size | 2 bytes | payload size | 
| payload | dynamic | respond payload | 


#### respond 2100 registration succsess

| Field | Size | Meaning |
|---------|---------|---------|
|client ID| 16 bytes | client ID |


#### respond 2101 registration fail

| Field | Size | Meaning |
|---------|---------|---------|
|client ID| 16 bytes | client ID |


#### respond 2102 recived AES key

| Field | Size | Meaning |
|---------|---------|---------|
|client ID| 16 bytes | client ID |
|AES encrypted| dynamic | AES encrypted |


#### respond 2103 recieved fix checksum

| Field | Size | Meaning |
|---------|---------|---------|
|client ID| 16 bytes | client ID |
|content size| 4 bytes | file content after encryption |
|file name| 255 bytes | file name that sent |
|file name| 16 bytes | Cksum |

## The Algorithm
Client can register to the server. If he exists in the database the server will respond to him that he exist, otherwise the server will generate a unique id for the client and respond to him with it. Then the client generates RSA keys and sends the public key to server.
The server gets the public key , generates simetric AES key and encrypts it with RSA key. The client gets the AES key and decrypts it and saves it.
Meanwhile the server always updates the AES ans RSA keys of each client in the database, the client encrypts desired file with the AES key and sends it to server.
The server decrypts the file content and saves it on a local backup folder, calculates checksum and sends back to the client. Client examines server checksum with his checksum. If they are equal he updates the server that the file is "good", if they are not equal, the client tries to resend the file. If there is an error with the checksum 3 times, the file will be deleted from backup and the server will respond that the file was not backed up.
