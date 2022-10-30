# C++ and Python Project: Backup Server & Client

## About
The project represent a backup server , each client can register and send file . The server get the files and store it in uniq file path for each client.
The files encrypt before send and decrypt at the server. 

## server
* Developed with python 3.9.9.
* Support multi clients by using multythreading
* used Crypto library for encrypt and decrypt

## client
* Developed with Visual Studio 2019.
* Client code written with ISO C++14 Standard. (Default by Visual Studio 2019).
* Crypto++ Library 8.5 is used. https://www.cryptopp.com
* <b>These instructions apply for x64 configuration. Upon loading .sln, use x86 build!</b>

## configure cryptopp

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


#### respond 2103 recieved fix cksum

| Field | Size | Meaning |
|---------|---------|---------|
|client ID| 16 bytes | client ID |
|content size| 4 bytes | file content after encryption |
|file name| 255 bytes | file name that sent |
|file name| 16 bytes | Cksum |

## The Algorithm
Client can ask for register , if he exist in DB the server will respond him that he exist. Then the client generate RSA keys and send the public key to server.
The server gets the pubkey , generate simetric AES key and encrypt it with RSA key.The client get the AES key decrypt it ans save it.
Meanwhile the server always update the AES ans RSA keys of each client at DB , the client encrypt desired file with the AES key and send it to server.
The server decrypt file content and save it on local backup folder, calculate cksum and send back to client.Client check server cksum with his cksum , if equal he update the server that the file is good, if not equal the client try resend file . If there is 3 time error with cksum , the file will be deleted from beckup and the server will respond that file wasnt backuped .
