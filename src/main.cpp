#include <iostream>
#include <cstdlib>
#include <optional>
#include <format>

#include "upp8m/arm/arm7tdmi.h"
#include "upp8m/memory.h"


namespace fs = std::filesystem;

std::optional<fs::path> getCurrentHomeDirectory() {
    const char* home = std::getenv("HOME");
    if (home == nullptr) {
        return std::nullopt;
    }
    return fs::path(home);
}

std::optional<fs::path> getTestProgramPath() {
    auto homeDirectory = getCurrentHomeDirectory();

    if (!homeDirectory) {
        std::cerr << "Invalid home directory\n";
        return std::nullopt;
    }
    fs::path programPath = homeDirectory.value() / "dev/nokia2280/test/test.elf";
    return programPath;
}

void printHexInstructions(const std::vector<uint8_t> & segment) {
    // NOTE: Naively prints 32-bit instructions, it do not distinguish between arm and thumb instructions yet!
    for (std::size_t i = 0; i + 4 <= segment.size(); i += 4) {
        std::cout << std::format("{:02x} {:02x} {:02x} {:02x}\n",
            segment[i], segment[i + 1], segment[i + 2], segment[i + 3]
        );
    }
}
int main() {
    Arm7tdmi cpu{};
    std::cout << "ARM7TDMI Emulator\n" << "!\n";
    std::cout << showRegisterState(cpu) << "\n";

    auto path = getTestProgramPath();
    if (!path) {
        std::cerr << "Invalid program path\n";
        return 1;
    }
    auto elf = loadElf(path.value());

    auto elfHeader = readElfHeader(elf);
    if (!elfHeader) {
        std::cerr << "Invalid ELF header\n";
        return 1;
    }

    auto programHeaders = readElfProgramHeaders(elf, elfHeader.value());
    if (!programHeaders) {
        std::cerr << "Invalid program headers\n";
        return 1;
    }

    auto binaryProgram = readProgramSegment(elf, programHeaders.value());

    printHexInstructions(binaryProgram);

    std::cout << "\n";

    std::cout <<
        std::format("Loaded program of size: {} bytes\n", elf.size());
}