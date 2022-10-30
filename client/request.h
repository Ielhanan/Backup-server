#pragma once
#include <iostream>
#include <WinSock2.h>
#define VERSION 3
#define PACKET_SIZE 1024
#define HEADER_SIZE 23
#pragma pack(push, 1)

class request {
public:
	union requestHeader {
					struct {
						char  clientID[16];
						uint8_t version;
						uint16_t code;
						uint32_t payloadSize;
					}ReqHead;
				char buffer[sizeof(ReqHead)];//this buffer include requst header as char arrey
				}FReqHead;
			char* payload;



#pragma pack(pop)
char* packRequest(char* payloadPackedBuffer,int payloadsize);
void sendPackets(SOCKET s,char* ptr,int size);

enum code
{
	REGISTER = 1100,
	PUBLICKEY = 1101,
	SENDFILE = 1103,
	FIXCRC = 1104,
	ERRCRC = 1105,
	ERR4CRC = 1106,
};
enum payloadSize {
	REGISTER_SIZE = 255,
	PUBLICKEY_SIZE = 415,
	SENDFILE_SIZE = 275,
	FIXCRC_SIZE=271,
};

~request();
};