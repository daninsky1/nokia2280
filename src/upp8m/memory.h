//
// Created by daniel on 10/2/26.
//

#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>
#include <elf.h>
#include <optional>


/* Adress Space
 * For now is a flat RAM only address space
 */
struct Memory {
    std::vector<uint8_t> data;
};

std::vector<uint8_t> loadElf(const std::filesystem::path& path);

std::optional<const Elf32_Ehdr> readElfHeader(const std::vector<uint8_t>& file);

std::optional<std::vector<Elf32_Phdr>> readElfProgramHeaders(const std::vector<uint8_t>& file, const Elf32_Ehdr& header);

std::vector<uint8_t> readProgramSegment(const std::vector<uint8_t>& elf, std::vector<Elf32_Phdr>& programHeader);

uint8_t readByte(Memory& memory, uint32_t address);

uint16_t readHalfword(Memory& memory, uint32_t address);

uint32_t readWord(Memory& memory, uint32_t address);

void writeByte(Memory& memory, uint32_t address, uint8_t value);

void writeHalfword(Memory& memory, uint32_t address, uint16_t value);

void writeWord(Memory& memory, uint32_t address, uint32_t value);
