#pragma once
#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include <fstream>
#include <string>
#include <filesystem>
#include<WS2tcpip.h>
#define VERSION 3
#define CLIENT_ID_SIZE 16
#define RESPONSE_HEADER_SIZE 7
#pragma pack(push, 1)


class respond {
public:
	union respondHeader {
		struct {
			uint8_t version;
			uint32_t payloadSize;
			uint16_t code;

		}ResHead;
		char buffer[sizeof(ResHead)];//this buffer include requst header as char arrey
	}FResHead;
	char* payload;



#pragma pack(pop)
	void unpackRespond( char* serverRespond);
	enum resCode
	{
		REGISTER_SUCCSESS = 2100,
		REGISTER_FAILED= 2101,
		PUBLICKEY_SUCCSESS = 2102,
		SENDFILE_SUCCSESS = 2103,
		MESSEGE__SUCCSESS = 2104,
	};
	~respond();
	enum resPayloadSize {
		REGISTER_PAYLOADSIZE = 16,
		PUBLICKEY_PAYLOADSIZE =32,
		RECIVED_FILE_PAYLOADSIZE=(255+4+4+16)



	};
};
