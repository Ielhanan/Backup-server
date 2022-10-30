#pragma once
#include <string>
#include <cstdint>
#include <fstream>
#include <iostream>

class CRC
{
public:
	uint32_t nchar;
	uint32_t crc;
	~CRC();
	void update(unsigned char* buf, uint32_t size);
	uint32_t digest();
	uint32_t calcCrc(std::string filePath);
	CRC();
};

