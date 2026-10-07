//
// Created by daniel on 10/7/26.
//

#include "instruction_util.h"

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
