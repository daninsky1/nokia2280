//
// Created by daniel on 10/7/26.
//

#pragma once

#include <string_view>

#include "arm7tdmi.h"

struct MnemonicInfo {
    Mnemonic mnemonic;
    std::string_view name;
    std::string_view description;
};

struct MnemonicExtInfo {
    Opcode::Cond mnemonicExt;
    std::string_view name;
    std::string_view description;
};

constexpr MnemonicExtInfo MNEMONIC_EXT_NAMES[] {
    {.mnemonicExt = Opcode::Cond::EQ, .name = "EQ", .description = "Equal"},
    {.mnemonicExt = Opcode::Cond::NE, .name = "NE", .description = "Not equal"},
    {.mnemonicExt = Opcode::Cond::CS_HS, .name = "CS_HS", .description = "Carry set/unsigned higher or same"},
    {.mnemonicExt = Opcode::Cond::CC_LO, .name = "CC_LO", .description = "Carry clear/unsigned lower"},
    {.mnemonicExt = Opcode::Cond::MI, .name = "MI", .description = "Minus/negative"},
    {.mnemonicExt = Opcode::Cond::PL, .name = "PL", .description = "Plus/positive or zero"},
    {.mnemonicExt = Opcode::Cond::VS, .name = "VS", .description = "Overflow"},
    {.mnemonicExt = Opcode::Cond::VC, .name = "VC", .description = "No overflow"},
    {.mnemonicExt = Opcode::Cond::HI, .name = "HI", .description = "Unsigned higher"},
    {.mnemonicExt = Opcode::Cond::LS, .name = "LS", .description = "Unsigned lower or same"},
    {.mnemonicExt = Opcode::Cond::GE, .name = "GE", .description = "Signed greater than or equal"},
    {.mnemonicExt = Opcode::Cond::LT, .name = "LT", .description = "Signed less than"},
    {.mnemonicExt = Opcode::Cond::GT, .name = "GT", .description = "Signed greater than"},
    {.mnemonicExt = Opcode::Cond::LE, .name = "LE", .description = "Signed less than or equal"},
    {.mnemonicExt = Opcode::Cond::AL, .name = "AL", .description = "Always (unconditional)"},               // AL can be hidden
    {.mnemonicExt = Opcode::Cond::UNCONDITIONAL, .name = "UNCONDITIONAL", .description = "Unconditional"},    // Unconditional
};

constexpr MnemonicInfo MNEMONIC_NAMES[] {
    {.mnemonic = Mnemonic::AND, .name = "AND", .description = "Loginal AND"},
    {.mnemonic = Mnemonic::EOR, .name = "EOR", .description = "Loginal Exclusive OR"},
    {.mnemonic = Mnemonic::SUB, .name = "SUB", .description = "Subtract"},
    {.mnemonic = Mnemonic::RSB, .name = "RSB", .description = "Reverse Subtract"},
    {.mnemonic = Mnemonic::ADD, .name = "ADD", .description = "Add"},
    {.mnemonic = Mnemonic::ADC, .name = "ADC", .description = "Add with Carry"},
    {.mnemonic = Mnemonic::SBC, .name = "SBC", .description = "Subtract with Carry"},
    {.mnemonic = Mnemonic::RSC, .name = "RSC", .description = "Reverse Subteract with Carry"},
    {.mnemonic = Mnemonic::TST, .name = "TST", .description = "Test"},
    {.mnemonic = Mnemonic::TEQ, .name = "TEQ", .description = "Test Equivalence"},
    {.mnemonic = Mnemonic::CMP, .name = "CMP", .description = "Compare"},
    {.mnemonic = Mnemonic::CMN, .name = "CMN", .description = "Compare Negated"},
    {.mnemonic = Mnemonic::ORR, .name = "ORR", .description = "Logical (inclusive) OR"},
    {.mnemonic = Mnemonic::MOV, .name = "MOV", .description = "Move"},
    {.mnemonic = Mnemonic::BIC, .name = "BIC", .description = "Bit Clear"},
    {.mnemonic = Mnemonic::MVN, .name = "MVN", .description = "Move Not"},
};

const MnemonicInfo* getMnemonicInfo(Mnemonic mnemonic);

const MnemonicExtInfo* getMnemonicExtInfo(Opcode::Cond mnemonicExt);

std::string formatMnemonic(DecodedInstruction instruction);
