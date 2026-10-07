//
// Created by daniel on 10/2/26.
//

#pragma once


#include <cstdint>
#include <vector>


struct Memory {
    std::vector<uint8_t> data;
};

uint8_t readByte(Memory& memory, uint32_t address);

uint16_t readHalfword(Memory& memory, uint32_t address);

uint32_t readWord(Memory& memory, uint32_t address);

void writeByte(Memory& memory, uint32_t address, uint8_t value);

void writeHalfword(Memory& memory, uint32_t address, uint16_t value);

void writeWord(Memory& memory, uint32_t address, uint32_t value);
