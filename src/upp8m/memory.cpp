//
// Created by daniel on 10/2/26.
//

#include "memory.h"

#include <cstring>
#include <fstream>
#include <iostream>

std::vector<uint8_t> loadElf(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (!file) {
        std::cerr << "Failed to open file: " << path << "\n";
        std::abort();
    }
    const auto size = file.tellg();
    std::vector<uint8_t> program(size);

    file.seekg(0, std::ios::beg);

    if (!file.read(reinterpret_cast<char*>(program.data()), size)) {
        std::cerr << "Failed to load file: " << path << "\n";
        std::abort();
    }
    return program;
}

std::optional<const Elf32_Ehdr> readElfHeader(const std::vector<uint8_t>& file) {
    Elf32_Ehdr header{};

    std::memcpy(&header, file.data(), sizeof(Elf32_Ehdr));

    // Reading identification header
    if (std::memcmp(header.e_ident, ELFMAG, SELFMAG) != 0) {
        std::cerr << "Not an ELF file.\n";
        return std::nullopt;
    }

    if (header.e_ident[EI_CLASS] != ELFCLASS32) {
        std::cerr << "Unsupported ELF class.\n";
        return std::nullopt;
    }

    if (header.e_ident[EI_DATA] != ELFDATA2LSB) {
        std::cerr << "Unsupported ELF endianness.\n";
        return std::nullopt;
    }

    if (header.e_ident[EI_VERSION] != EV_CURRENT || header.e_version != EV_CURRENT) {
        std::cerr << "Unsupported ELF version.\n";
        return std::nullopt;
    }

    if (header.e_machine != EM_ARM) {
        std::cerr << "Not an arm ELF arch.\n";
        return std::nullopt;
    }

    if (header.e_type != ET_EXEC) {
        std::cerr << "Not an executable ELF.\n";
        return std::nullopt;
    }

    if (header.e_ehsize != sizeof(Elf32_Ehdr)) {
        std::cerr << "Invalid ELF header size.\n";
        return std::nullopt;
    }

    return header;
}

std::optional<std::vector<Elf32_Phdr>> readElfProgramHeaders(const std::vector<uint8_t>& file, const Elf32_Ehdr& header) {
    if (header.e_phentsize != sizeof(Elf32_Phdr)) {
        std::cerr << "Invalid ELF program header size.\n";
        return std::nullopt;
    }

    const size_t offset = header.e_phoff;
    const size_t count = header.e_phnum;
    const size_t entrySize = sizeof(Elf32_Phdr);

    // Validates whether the table fits in the file
    if (offset > file.size() ||
        count > (file.size() - offset) / entrySize) {
        std::cerr << "Program header table does not fit in the file.\n";
        return std::nullopt;
    }

    std::vector<Elf32_Phdr> programHeaders(count);
    if (count != 0) {
        std::memcpy(programHeaders.data(), file.data() + offset, count * entrySize);
    }

    return programHeaders;
}

std::vector<uint8_t> readProgramSegment(const std::vector<uint8_t>& elf, std::vector<Elf32_Phdr>& programHeader) {
    for (const Elf32_Phdr& phdr : programHeader) {
        if (phdr.p_type != PT_LOAD) continue;

        if (phdr.p_filesz > phdr.p_memsz) continue;

        if (phdr.p_offset > elf.size() ||
            phdr.p_filesz > elf.size() - phdr.p_offset)
            continue;

        if (phdr.p_vaddr != 0) {
            std::cerr << "Program segment is not at address 0!\n" <<
                "For advanced memory layout is not supported yet.\n";
            continue;
        }

        std::vector<uint8_t> segment{
            elf.begin() + phdr.p_offset,
            elf.begin() + phdr.p_offset + phdr.p_filesz
        };

        // NOTE: Only one loadable program segment is supported.
        return segment;
    }
    return {};
}

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

