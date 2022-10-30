#pragma once
#include "response.h"


void respond::unpackRespond(char* serverRespond) {
	try {
		char* ptr = serverRespond;
		memcpy(&this->FResHead.ResHead.version, ptr, sizeof(uint8_t));
		memcpy(&this->FResHead.ResHead.code, ptr + sizeof(uint8_t), sizeof(uint16_t));
		memcpy(&this->FResHead.ResHead.payloadSize, ptr + (sizeof(uint8_t) + sizeof(uint16_t)), sizeof(uint32_t));
		ptr += sizeof(this->FResHead);
		this->payload = new char[FResHead.ResHead.payloadSize];
		memset(this->payload, 0, FResHead.ResHead.payloadSize);
		memcpy(this->payload, ptr, this->FResHead.ResHead.payloadSize);
	}
	catch (const std::exception&)
	{
		std::cout << "Error creat socket" << std::endl;
	}
}

respond::~respond() {
	delete [] this->payload;
}