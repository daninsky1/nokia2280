//
// Created by daniel on 10/7/26.
//

#include "instruction_util.h"

#include <format>
#include <iostream>

const MnemonicInfo* getMnemonicInfo(Mnemonic mnemonic) {
    for (const auto& info : MNEMONIC_NAMES) {
        if (info.mnemonic == mnemonic)
            return &info;
    }
    return nullptr;
}

const MnemonicExtInfo* getMnemonicExtInfo(Opcode::Cond mnemonicExt) {
    for (const auto& info : MNEMONIC_EXT_NAMES) {
        if (info.mnemonicExt == mnemonicExt)
            return &info;
    }
    return nullptr;
}

std::string formatMnemonic(DecodedInstruction instruction) {
    const auto* mnemonicInfo = getMnemonicInfo(instruction.mnemonic);
    const auto* mnemonicExtInfo = getMnemonicExtInfo(instruction.cond);

    if (!mnemonicInfo || !mnemonicExtInfo) {
        return "INVALID";
    }

    std::string result{mnemonicExtInfo->name};

    return result;
}

inline void printArmInstruction(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3) {
    std::cout << std::format("{:02x} {:02x} {:02x} {:02x}\n",
        byte0, byte1, byte2, byte3
    );
}

inline void printArmInstruction(uint32_t instruction) {
    std::cout << std::format("{:02x} {:02x} {:02x} {:02x}\n",
        static_cast<uint8_t>(instruction),
        static_cast<uint8_t>(instruction >> 8),
        static_cast<uint8_t>(instruction >> (8 * 2)),
        static_cast<uint8_t>(instruction >> (8 * 3))
    );
}