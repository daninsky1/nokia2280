//
// Created by daniel on 10/2/26.
//

#include "memory.h"

#include <iostream>

uint8_t readByte(Memory &memory, uint32_t address) {
    std::cerr << "readByte not implemented.\n";
    std::abort();
    return 0;
}

uint16_t readHalfword(Memory &memory, uint32_t address) {
    std::cerr << "readHalfword not implemented.\n";
    std::abort();
    return 0;
}

uint32_t readWord(Memory &memory, uint32_t address) {
    std::cerr << "readWord not implemented.\n";
    std::abort();
    return 0;
}

void writeByte(Memory &memory, uint32_t address, uint8_t value) {
    std::cerr << "writeByte not implemented.\n";
    std::abort();
}

void writeHalfword(Memory &memory, uint32_t address, uint16_t value) {
    std::cerr << "writeHalfword not implemented.\n";
    std::abort();
}

void writeWord(Memory &memory, uint32_t address, uint32_t value) {
    std::cerr << "writeWord not implemented.\n";
    std::abort();
}

