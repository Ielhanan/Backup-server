#pragma once
#include "request.h"
#include "CRC.h"
char*  request::packRequest( char* payloadPackedBuffer,int payloadsize) {
	try
	{
		char* buffer = new char[sizeof(this->FReqHead.buffer) + payloadsize];
		memset(buffer, 0, sizeof(this->FReqHead.buffer) + payloadsize);
		memcpy(buffer, this->FReqHead.buffer, sizeof(this->FReqHead.buffer));
		char* tmp = buffer;
		tmp += sizeof(this->FReqHead.buffer);
		if (payloadPackedBuffer != NULL)//if not exist payload 
			memcpy(tmp, payloadPackedBuffer, payloadsize);
			//prepare the packet for sending to server, unite the payload buffer and header 
		return buffer;//return the head of buffer
	}
	catch (const std::exception&)
	{
		std::cout << "Error creat socket" << std::endl;
		return NULL;
	}
}

void request::sendPackets(SOCKET s,char* buffer,int size) {
	send(s, buffer, size, 0);
}

request::~request() {
	delete[]  this->payload;
}
