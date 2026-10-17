//
// Created by daniel on 10/1/26.
//

#include <format>
#include <iostream>

#include "arm7tdmi.h"
#include "instruction_util.h"

void CRASH_THIS_FUCKER(std::string message) {
    std::cerr << message << std::endl;
    std::abort();
}

uint8_t DP::Data::rotateImm() const {
    return static_cast<uint8_t>((shifterOperand & ROTATE_MASK) >> ROTATE_SHIFT);
}

uint8_t DP::Data::immed8() const {
    return static_cast<uint8_t>(shifterOperand & IMM_MASK);
}

uint8_t DP::Data::rm() const {
    return static_cast<uint8_t>(shifterOperand & RM_MASK);
}

bool DP::Data::isShftReg() const {
    return (shifterOperand & BIT7_MASK) == 0 && (shifterOperand & REG_MASK) != 0;
}

uint8_t DP::Data::rs() const {
    return static_cast<uint8_t>((shifterOperand & RS_MASK) >> RS_SHIFT);
}

uint8_t DP::Data::shiftImm() const {
    return (shifterOperand & SHIFT_AMOUNT_MASK) >> SHIFT_AMOUNT_SHIFT;
}

uint8_t DP::Data::shift() const {
    return (shifterOperand & SHIFT_MASK) >> SHIFT_SHIFT;
}

std::string exceptionDescription(Exception exception) {
    switch (exception) {
        case Exception::RESET: {
            return std::string{"Reset."};
            break;
        }
        case Exception::UNDEFINED_INSTRUCTION: {
            return std::string{"Attempted execution of an Undefined instruction."};
            break;
        }
        case Exception::SOFTWARE_INTERRUPT: {
            return std::string{
                    "Software interrupt (SWI) instructions, can be used to make a call to an operating system"};
            break;
        }
        case Exception::PREFETCH_ABORT: {
            return std::string{"Prefetch Abort, an instruction fetch memory abort"};
            break;
        }
        case Exception::DATA_ABORT: {
            return std::string{"Data Abort, a data access memory abort"};
            break;
        }
        case Exception::IRQ: {
            return std::string{"Normal interrupt"};
            break;
        }
        case Exception::FIQ: {
            return std::string{"Fast interrupt"};
            break;
        }
        default: {
            static_assert("Invalid exception");
            return std::string{"Invalid exception."};
        }
    }
}

uint32_t Arm7tdmi::step() const {
    return instructionSet == InstructionSet::ARM ? 4 : 2;
}

uint32_t Arm7tdmi::r(uint8_t index) const {
    if (processorMode == ProcessorMode::FIQ && index >= GPR::R8 && index <= GPR::R14) {
        return registers.fiq[index - GPR::R8];
    }
    if (index == GPR::R13 || index == GPR::R14) {
        switch (processorMode) {
        case ProcessorMode::IRQ:
            return registers.irq[index - GPR::R13];
        case ProcessorMode::SUPERVISOR:
            return registers.svc[index - GPR::R13];
        case ProcessorMode::ABORT:
            return registers.abt[index - GPR::R13];
        case ProcessorMode::UNDEFINED:
            return registers.und[index - GPR::R13];
        }
    }
    if (index == GPR::PC) {
        return registers.r[index] + (instructionSet == InstructionSet::ARM ? 8 : 4);
    }
    return registers.r[index];
}

void Arm7tdmi::r(uint8_t index, uint32_t value) {
    if (processorMode == ProcessorMode::FIQ && index >= GPR::R8 && index <= GPR::R14) {
        registers.fiq[index - GPR::R8] = value;
    }
    if (index == GPR::R13 || index == GPR::R14) {
        switch (processorMode) {
        case ProcessorMode::IRQ:
            registers.irq[index - GPR::R13] = value;
        case ProcessorMode::SUPERVISOR:
            registers.svc[index - GPR::R13] = value;
        case ProcessorMode::ABORT:
            registers.abt[index - GPR::R13] = value;
        case ProcessorMode::UNDEFINED:
            registers.und[index - GPR::R13] = value;
        }
    }
    registers.r[index] = value;
}

uint32_t Arm7tdmi::cpsr() const {
    return statusRegisters.cpsr;
}

void Arm7tdmi::cpsr(uint32_t value) {
    statusRegisters.cpsr = value;
}

uint32_t Arm7tdmi::spsr() const{
    switch (processorMode) {
    case ProcessorMode::SUPERVISOR:
        return statusRegisters.spsr_svc;
    case ProcessorMode::ABORT:
        return statusRegisters.spsr_abt;
    case ProcessorMode::UNDEFINED:
        return statusRegisters.spsr_und;
    case ProcessorMode::IRQ:
        return statusRegisters.spsr_irq;
    case ProcessorMode::FIQ:
        return statusRegisters.spsr_fiq;
    default:
        std::cerr << "Invalid mode for getSpsr" << std::endl;
        CRASH_THIS_FUCKER(UNPREDICTABLE_MSG);
    }
}
void Arm7tdmi::spsr(uint32_t value) {
    switch (processorMode) {
    case ProcessorMode::SUPERVISOR:
        statusRegisters.spsr_svc = value;
    case ProcessorMode::ABORT:
        statusRegisters.spsr_abt = value;
    case ProcessorMode::UNDEFINED:
        statusRegisters.spsr_und = value;
    case ProcessorMode::IRQ:
        statusRegisters.spsr_irq = value;
    case ProcessorMode::FIQ:
        statusRegisters.spsr_fiq = value;
    default:
        std::cerr << "Invalid mode for getSpsr" << std::endl;
        CRASH_THIS_FUCKER(UNPREDICTABLE_MSG);
    }
}

void Arm7tdmi::copySpsr2Cpsr() {
    switch (processorMode) {
    case ProcessorMode::SUPERVISOR:
        statusRegisters.cpsr = statusRegisters.spsr_svc;
    case ProcessorMode::ABORT:
        statusRegisters.cpsr = statusRegisters.spsr_abt;
    case ProcessorMode::UNDEFINED:
        statusRegisters.cpsr = statusRegisters.spsr_und;
    case ProcessorMode::IRQ:
        statusRegisters.cpsr = statusRegisters.spsr_irq;
    case ProcessorMode::FIQ:
        statusRegisters.cpsr = statusRegisters.spsr_fiq;
    default:
        std::cerr << "Invalid mode for getSpsr" << std::endl;
        CRASH_THIS_FUCKER(UNPREDICTABLE_MSG);
    }
}

void Arm7tdmi::mode(ProcessorMode mode) {
    this->processorMode = mode;
}

ProcessorMode Arm7tdmi::mode(ProcessorMode mode) const {
    return processorMode;
}

GPR & Arm7tdmi::getRegisters() {
    return registers;
}

SR& Arm7tdmi::getStatusRegisters() {
    return statusRegisters;
}

void reset(Arm7tdmi &cpu) {
    uint32_t cpsr = static_cast<uint32_t>(ProcessorMode::SUPERVISOR) |
        PSR::I |
        PSR::F;
    cpu.cpsr(cpsr);
    cpu.r(GPR::PC, ExceptionVecAddr::RESET);
}

void undefinedInstruction(Arm7tdmi &cpu) {
    // TODO: The r14 must be set to the address of the next instruction after the Undefined instruction. Check if it is
    //  PC
    cpu.mode(ProcessorMode::UNDEFINED);
    cpu.r(GPR::R14, cpu.cia + 4);      // address of next instruction after the Undefined instruction
    cpu.spsr(cpu.cpsr());

    uint32_t cpsr = (cpu.cpsr() & ~PSR::MODE_MASK) | static_cast<uint32_t>(ProcessorMode::UNDEFINED);
    cpsr &= ~PSR::T;        // Execute in ARM state
    cpsr |= PSR::I;         // Disable normal interrupts
    cpu.cpsr(cpsr);

    cpu.r(GPR::PC, ExceptionVecAddr::UNDEFINED_INSTRUCTION);
}


void softwareInterrupt(Arm7tdmi &cpu) {
    cpu.mode(ProcessorMode::SUPERVISOR);
    cpu.r(GPR::R14, cpu.cia + 4);      // address of next instruction after the SWI instruction
    cpu.spsr(cpu.cpsr());

    uint32_t cpsr = (cpu.cpsr() & ~PSR::MODE_MASK) | static_cast<uint32_t>(ProcessorMode::SUPERVISOR);
    cpsr &= ~PSR::T;        // Execute in ARM state
    cpsr |= PSR::I;         // Disable normal interrupts

    cpu.r(GPR::PC, ExceptionVecAddr::SOFTWARE_INTERRUPT);
}


void prefetchAbort(Arm7tdmi &cpu) {
    cpu.mode(ProcessorMode::ABORT);
    cpu.r(GPR::R14, cpu.cia + 4);      // address of the aborted instruction + 4
    cpu.spsr(cpu.cpsr());

    uint32_t cpsr = (cpu.cpsr() & ~PSR::MODE_MASK) | static_cast<uint32_t>(ProcessorMode::ABORT);
    cpsr &= ~PSR::T;        // Execute in ARM state
    cpsr |= PSR::I;         // Disable normal interrupts
    cpu.cpsr(cpsr);

    cpu.r(GPR::PC, ExceptionVecAddr::PREFETCH_ABORT);
}


void dataAbort(Arm7tdmi cpu) {
    // TODO: Learn about imprecise data aborts that could lead to an unrecoverable state, hang
    cpu.mode(ProcessorMode::ABORT);
    cpu.r(GPR::R14, cpu.cia + 8);      // address of the aborted instruction + 8
    cpu.spsr(cpu.cpsr());

    uint32_t cpsr = (cpu.cpsr() & ~PSR::MODE_MASK) | static_cast<uint32_t>(ProcessorMode::ABORT);
    cpsr &= ~PSR::T;        // Execute in ARM state
    cpsr |= PSR::I;         // Disable normal interrupts
    cpu.cpsr(cpsr);

    cpu.r(GPR::PC, ExceptionVecAddr::DATA_ABORT);
}


void irq(Arm7tdmi &cpu) {
    cpu.mode(ProcessorMode::IRQ);
    cpu.r(GPR::R14, cpu.cia + 8);      // address of next instruction to be executed + 4
    cpu.spsr(cpu.cpsr());

    uint32_t cpsr = (cpu.cpsr() & ~PSR::MODE_MASK) | static_cast<uint32_t>(ProcessorMode::IRQ);
    cpsr &= ~PSR::T;        // Execute in ARM state
    cpsr |= PSR::I;         // Disable normal interrupts
    cpu.cpsr(cpsr);
    cpu.r(GPR::PC, ExceptionVecAddr::IRQ);
}


void fiq(Arm7tdmi &cpu) {
    cpu.mode(ProcessorMode::FIQ);
    cpu.r(GPR::R14, cpu.cia + 8);      // address of next instruction to be executed + 4

    cpu.spsr(cpu.cpsr());

    uint32_t cpsr = (cpu.cpsr() & ~PSR::MODE_MASK) | static_cast<uint32_t>(ProcessorMode::FIQ);
    cpsr &= ~PSR::T;        // Execute in ARM state
    cpsr |= PSR::F;         // Disable fast interrupts
    cpsr |= PSR::I;         // Disable normal interrupts
    cpu.cpsr(cpsr);
    cpu.r(GPR::PC, ExceptionVecAddr::FIQ);
}

Instruction getDataProcessingMnemonic(DP::Opcode opcode) {
    switch (opcode) {
    case DP::Opcode::AND: return Instruction::AND; break;
    case DP::Opcode::EOR: return Instruction::EOR; break;
    case DP::Opcode::SUB: return Instruction::SUB; break;
    case DP::Opcode::RSB: return Instruction::RSB; break;
    case DP::Opcode::ADD: return Instruction::ADD; break;
    case DP::Opcode::ADC: return Instruction::ADC; break;
    case DP::Opcode::SBC: return Instruction::SBC; break;
    case DP::Opcode::RSC: return Instruction::RSC; break;
    case DP::Opcode::TST: return Instruction::TST; break;
    case DP::Opcode::TEQ: return Instruction::TEQ; break;
    case DP::Opcode::CMP: return Instruction::CMP; break;
    case DP::Opcode::CMN: return Instruction::CMN; break;
    case DP::Opcode::ORR: return Instruction::ORR; break;
    case DP::Opcode::MOV: return Instruction::MOV; break;
    case DP::Opcode::BIC: return Instruction::BIC; break;
    case DP::Opcode::MVN: return Instruction::MVN; break;
    default:
        std::cerr << "Invalid data processing opcode: " << static_cast<int>(opcode) << std::endl;
        std::abort();
    }
}

// 115
DecodedOperation decodeDataProcessingInstruction(uint32_t rawInstruction) {
    DP::Data dp;
    dp.i  = (rawInstruction & DP::I_FLAG) != 0;
    dp.s = (rawInstruction & DP::S_FLAG) != 0;
    dp.opcode = static_cast<DP::Opcode>((rawInstruction & DP::OPCODE_MASK) >> DP::OPCODE_SHIFT);
    dp.rn = static_cast<uint8_t>((rawInstruction & DP::RN_MASK) >> DP::RN_SHIFT);
    dp.rd = static_cast<uint8_t>((rawInstruction & DP::RD_MASK) >> DP::RD_SHIFT);
    dp.shifterOperand = static_cast<uint16_t>(rawInstruction & DP::SHIFTER_MASK);
    return DecodedOperation{
        .type = InstructionType::DATA_PROCESSING,
        .mnemonic = getDataProcessingMnemonic(dp.opcode),
        .instructionData = dp
    };
}

DecodedOperation decodeMiscellaneousInstruction(uint32_t rawInstruction) {

}

DecodedInstruction decodeInstruction(uint32_t rawInstruction) {
    DecodedInstruction instruction{};
    instruction.cond = static_cast<Opcode::Cond>((rawInstruction & IEnc::COND_MASK) >> IEnc::COND_SHIFT);
    instruction.clazz = static_cast<IEnc::Class>((rawInstruction & IEnc::CLASS_MASK) >> IEnc::CLASS_SHIFT);

    switch (instruction.clazz) {
    case IEnc::Class::C000: {
        uint32_t mult = (rawInstruction & IEnc::C000::MULT_DEC_MASK);
        if (mult == IEnc::C000::MULT_VALUE) {
            // Multiplies, extra load/stores instructions
        }
        uint32_t misc = (rawInstruction & IEnc::C000::MISC_DEC_MASK);
        if (misc == IEnc::C000::MISC_VALUE) {
            // Miscellaneous instructions
            // TODO: Miscellaneous instructions
            auto decOp = decodeMiscellaneousInstruction(rawInstruction);
            return instruction;
        }
        // Data processing instructions
        DecodedOperation decOp = decodeDataProcessingInstruction(rawInstruction);
        instruction.type = decOp.type;
        instruction.mnemonic = decOp.mnemonic;
        instruction.data = decOp.instructionData;
        return instruction;
        break;
    }
    case IEnc::Class::C001: {
        uint32_t decode = (rawInstruction & IEnc::C001::DECODE_MASK);
        if (decode == IEnc::C001::UNDEFINED_VALUE) {
            // Undefined instruction
            instruction.type = InstructionType::UNDEFINED;
            return instruction;
        }
        if (decode == IEnc::C001::MISR_VALUE) {
            // Move immediate to status register
            return instruction;
        }
        // TODO: Decode immediate data-processing instructions, including ADC. This path currently falls through to
        // UNDEFINED because immediate forms are not implemented yet.

        break;
    }
    case IEnc::Class::LSIO: {
        // Load/store immediate offset
        instruction.type = InstructionType::LOAD_STORE;
        bool p = (rawInstruction & IEnc::C010_LSIO::P_MASK) != 0;
        bool u = (rawInstruction & IEnc::C010_LSIO::U_MASK) != 0;
        bool b = (rawInstruction & IEnc::C010_LSIO::B_MASK) != 0;
        bool w = (rawInstruction & IEnc::C010_LSIO::W_MASK) != 0;
        bool l = (rawInstruction & IEnc::C010_LSIO::L_MASK) != 0;

        return instruction;
        break;
    }
    case IEnc::Class::C011: {
        auto mediaInstruction = (rawInstruction & IEnc::C011::MEDIA_MASK);
        if (mediaInstruction) {
            // Media instructions
            // NOTE: Undefined instruction on ARMv4
            instruction.type = InstructionType::UNDEFINED;
            return instruction;
        }

        auto archUndef = (rawInstruction & IEnc::C011::ARCH_UNDEF_MASK);
        if (archUndef == IEnc::C011::ARCH_UNDEF_VALUE) {
            instruction.type = InstructionType::UNDEFINED;
            return instruction;
        }
        // Load/store register offset
        instruction.type = InstructionType::LOAD_STORE;
        return instruction;
        break;
    }
    case IEnc::Class::LSMU: {
        // Load/store multiple
        instruction.type = InstructionType::LOAD_STORE;
        return instruction;
        break;
    }
    case IEnc::Class::BRANCH: {
        instruction.type = InstructionType::BRANCH;
        auto l = (rawInstruction & IEnc::C101_BRANCH::L_FLAG);
        return instruction;
        break;
    }
    case IEnc::Class::COPLSDRT: {
        // Coprocessor load/store and double register transfers
        // TODO: Coprocessor load/store and double register transfers instruction
        instruction.type = InstructionType::COPROCESSOR;

        bool p = (rawInstruction & IEnc::C110_COP::P_FLAG) != 0;
        bool u = (rawInstruction & IEnc::C110_COP::U_FLAG) != 0;
        bool n = (rawInstruction & IEnc::C110_COP::N_FLAG) != 0;
        bool w = (rawInstruction & IEnc::C110_COP::W_FLAG) != 0;
        bool l = (rawInstruction & IEnc::C110_COP::L_FLAG) != 0;

        if (!p && !u && !w) {
            // "Coprocessor instruction extension space is UNDEFINED on ARMv4"
            instruction.type = InstructionType::UNDEFINED;
            return instruction;
        }
        break;
    }
    case IEnc::Class::C111: {
        auto swi = (rawInstruction & IEnc::C111::SWI::SWI_FLAG);
        if (swi == IEnc::C111::SWI::SWI_FLAG) {
            // SWI Instruction
            instruction.type = InstructionType::EXCEPTION_GENERATING;
            return instruction;
        }
        // NOTE: Not implemented yet. I don't know if COPROCESSOR instruction is needed for DCT4
        instruction.type = InstructionType::COPROCESSOR;
        auto coprocessorRegisterTransfer = (rawInstruction & IEnc::C111::COP::RT_FLAG);
        if (coprocessorRegisterTransfer) {
            // Coprocessor register transfers
            // TODO: Coprocessor register transfers instruction
        } else {
            // Coprocessor data processing
            // TODO: Coprocessor data processing instruction
        }

        return instruction;
        break;
    }
    default:
        std::cerr << "Invalid instruction class" << std::endl;
        std::abort();
    }
    instruction.type = InstructionType::UNDEFINED;
    return instruction;
}

void executeInstruction(Arm7tdmi &cpu, DecodedInstruction& instruction);

void executeProgram(Arm7tdmi &cpu, const std::vector<uint8_t> &program) {
    // TODO: cia current instruction address. A naive approach to addressing. We need a memory address space controller
    //  where we can map the instruction address to a memory address
    cpu.r(GPR::PC, cpu.cia);
    while (true) {
        uint32_t rawInstruction{};
        for (uint8_t j = 0; j < cpu.step(); j++) {
            // NOTE: Endianess dependent
            rawInstruction |= static_cast<uint32_t>(program[cpu.cia + j]) << (j * 8);
        }

        auto decodedInstruction = decodeInstruction(rawInstruction);

        bool execute = true;
        if (decodedInstruction.cond != Opcode::Cond::AL) {
            switch (decodedInstruction.cond) {
            case Opcode::Cond::EQ: {
                execute = (cpu.cpsr() & PSR::Z) != 0;
                break;
            }
            case Opcode::Cond::NE: {
                execute = (cpu.cpsr() & PSR::Z) == 0;
                break;
            }
            case Opcode::Cond::CS_HS: {
                execute = (cpu.cpsr() & PSR::C) != 0;
                break;
            }
            case Opcode::Cond::CC_LO: {
                execute = (cpu.cpsr() & PSR::C) == 0;
                break;
            }
            case Opcode::Cond::MI: {
                execute = (cpu.cpsr() & PSR::N) != 0;
                break;
            }
            case Opcode::Cond::PL: {
                execute = (cpu.cpsr() & PSR::N) == 0;
                break;
            }
            case Opcode::Cond::VS: {
                execute = (cpu.cpsr() & PSR::V) != 0;
                break;
            }
            case Opcode::Cond::VC: {
                execute = (cpu.cpsr() & PSR::V) == 0;
                break;
            }
            case Opcode::Cond::HI: {
                execute = (cpu.cpsr() & PSR::C) != 0 && (cpu.cpsr() & PSR::Z) == 0;
                break;
            }
            case Opcode::Cond::LS: {
                execute = (cpu.cpsr() & PSR::C) == 0 || (cpu.cpsr() & PSR::Z) != 0;
                break;
            }
            case Opcode::Cond::GE: {
                execute = ((cpu.cpsr() & PSR::N) != 0) == ((cpu.cpsr() & PSR::V) != 0);
                break;
            }
            case Opcode::Cond::LT: {
                execute = ((cpu.cpsr() & PSR::N) != 0) != ((cpu.cpsr() & PSR::V) != 0);
                break;
            }
            case Opcode::Cond::GT: {
                execute = (cpu.cpsr() & PSR::Z) == 0 && ((cpu.cpsr() & PSR::N) != 0) == ((cpu.cpsr() & PSR::V) != 0);
                break;
            }
            case Opcode::Cond::LE: {
                execute = (cpu.cpsr() & PSR::Z) != 0 || ((cpu.cpsr() & PSR::N) != 0) != ((cpu.cpsr() & PSR::V) != 0);
                break;
            }
            default:
            case Opcode::Cond::UNCONDITIONAL: {
                // Continue with NOP?
                std::cerr << "Unconditional instruction prior to ARMv5 is UNPREDICTABLE" << std::endl;
                std::abort();
            }
            }
        }
        // NOTE: If the flags do not satisfy the condition, the instruction acts as a NOP that is, execution
        //  advances to the next instruction as normal, including any relevant checks for interrupts and
        //  Prefetch Aborts, but has no other effect.
        if (execute) {
            executeInstruction(cpu, decodedInstruction);
        } else {
            // TODO: NOP
        }
        cpu.cia += cpu.step();
        cpu.r(GPR::PC, cpu.cia);
    }
}

static DP::ShftOperand executeShftOperand(const Arm7tdmi &cpu, const DP::Data &dp) {
    DP::ShftOperand sftOp{};
    if (dp.i) {
        // A5.1.3 Data-processing operands - Immediate
        uint8_t rotateImm = dp.rotateImm();
        sftOp.operand = std::rotr(static_cast<uint32_t>(dp.immed8()), 2 * dp.rotateImm());
        if (rotateImm == 0) {
            sftOp.shftCarryOut = (cpu.cpsr() & PSR::C) != 0;
        } else {
            sftOp.shftCarryOut = (sftOp.operand & DP::SHFT_OP_BIT31_FLAG) != 0;
        }
    } else {
        uint32_t rm = cpu.r(dp.rm());
        bool isShftReg= dp.isShftReg();
        uint8_t shift = dp.shift();
        uint8_t shiftImm = dp.shiftImm();
        bool isRegister = !isShftReg && !shift && !shiftImm;
        if (isRegister) {
            sftOp.operand = rm;
            sftOp.shftCarryOut = (cpu.cpsr() & PSR::C) != 0;
        } else if (isShftReg) {
            // Shift by register;
            if (dp.rd == GPR::PC || dp.rm() == GPR::PC || dp.rn == GPR::PC || dp.rs() == GPR::PC) {
                CRASH_THIS_FUCKER(UNPREDICTABLE_MSG);
            }
            uint8_t rs = static_cast<uint8_t>(cpu.r(dp.rs()));      // Register shift
            switch (shift) {
            case DP::Shift::LSL: {
                if (rs == 0) {
                    sftOp.operand = rm;
                    sftOp.shftCarryOut = (cpu.cpsr() & PSR::C) != 0;
                } else if (rs < 32) {
                    sftOp.operand = rm << rs;
                    sftOp.shftCarryOut = (rm & (1 << (32 - rs))) != 0;
                } else if (rs == 32) {
                    sftOp.operand = 0;
                    sftOp.shftCarryOut = (rm & 0b1) != 0;
                } else if (rs > 32) {
                    sftOp.operand = 0;
                    sftOp.shftCarryOut = false;
                }
                break;
            }
            case DP::Shift::LSR: {
                if (rs == 0) {
                    sftOp.operand = rm;
                    sftOp.shftCarryOut = (cpu.cpsr() & PSR::C) != 0;
                } else if (rs < 32) {
                    sftOp.operand = rm >> rs;
                    sftOp.shftCarryOut = (rm & (1 << (rs - 1))) != 0;
                } else if (rs == 32) {
                    sftOp.operand = 0;
                    sftOp.shftCarryOut = (rm & (0b1 << 31)) != 0;
                } else if (rs > 32) {
                    sftOp.operand = 0;
                    sftOp.shftCarryOut = false;
                }
                break;
            }
            case DP::Shift::ASR: {
                if (rs == 0) {
                    sftOp.operand = rm;
                    sftOp.shftCarryOut = (cpu.cpsr() & PSR::C) != 0;
                } else if (rs < 32) {
                    sftOp.operand = static_cast<uint32_t>(
                        static_cast<int32_t>(rm) >> rs
                    );
                    sftOp.shftCarryOut = (rm & (1 << (rs - 1))) != 0;
                } else {
                    bool sign = (rm & Bit::WORD_MSB_MASK) != 0;
                    if (sign) {
                        sftOp.operand = 0xFFFFFFFF;
                    } else {
                        sftOp.operand = 0;
                    }
                    sftOp.shftCarryOut = sign;
                }
                break;
            }
            case DP::Shift::RTR: {
                uint8_t rot = rs & 0b11111;
                if (rs == 0) {
                    sftOp.operand = rm;
                    sftOp.shftCarryOut = (cpu.cpsr() & PSR::C) != 0;
                } else if (rot == 0) {
                    sftOp.operand = rm;
                    sftOp.shftCarryOut = (rm & (0b1 << 31)) != 0;
                } else if (rot > 0) {
                    sftOp.operand = std::rotr(rm, rot);
                    sftOp.shftCarryOut = (rm & (1 << (rot - 1))) != 0;
                }
                break;
            }
            default:
                CRASH_THIS_FUCKER(UNPREDICTABLE_MSG);
            }
        } else {
            // Shift by immediate;
            switch (shift) {
            case DP::Shift::LSL: {
                sftOp.operand = rm << shiftImm;
                sftOp.shftCarryOut = (rm & (1 << (32 - shiftImm))) != 0;
                break;
            }
            case DP::Shift::LSR: {
                if (shiftImm == 0) {
                    sftOp.operand = 0;
                    sftOp.shftCarryOut = (rm & Bit::WORD_MSB_MASK) != 0;
                } else {
                    sftOp.operand = rm >> shiftImm;
                    sftOp.shftCarryOut = (rm & (1 << (shiftImm - 1))) != 0;
                }
                break;
            }
            case DP::Shift::ASR: {
                if (shiftImm == 0) {
                    bool sign = (rm & Bit::WORD_MSB_MASK) != 0;
                    sftOp.shftCarryOut = sign;
                    if (sign) {
                        sftOp.operand = 0xFFFFFFFF;
                    } else {
                        sftOp.operand = 0;
                    }
                } else {
                    sftOp.operand = static_cast<uint32_t>(
                        static_cast<int32_t>(rm) >> shiftImm
                    );
                    sftOp.shftCarryOut = (rm & (1 << (shiftImm - 1))) != 0;
                }
                break;
            }
            case DP::Shift::RTR: {
                if (shiftImm == 0) {
                    sftOp.operand = ((cpu.cpsr() & PSR::C) != 0 ? Bit::WORD_MSB_MASK : 0) | (rm >> 1);
                    sftOp.shftCarryOut = (rm & 1) != 0;
                } else {
                    sftOp.operand = std::rotr(rm, shiftImm);
                    sftOp.shftCarryOut = (rm & (1 << (shiftImm - 1))) != 0;
                }
                break;
            }
            default:
                CRASH_THIS_FUCKER(UNPREDICTABLE_MSG);
            }
        }
    }
    return sftOp;
}

void executeInstruction(Arm7tdmi &cpu, DecodedInstruction& instruction) {
    switch (instruction.mnemonic) {
    case Instruction::AND: {
        auto& dp = std::get<DP::Data>(instruction.data);
        DP::ShftOperand shftOperand = executeShftOperand(cpu, dp);
        uint32_t operand1 = cpu.r(dp.rn);
        uint32_t operand2 = shftOperand.operand;
        uint32_t dest     = operand1 & operand2;
        if (dp.s) {
            if (dp.rd == GPR::PC) {
                cpu.copySpsr2Cpsr();
            } else {
                const uint32_t nzc = (dest & PSR::N)
                    | ((dest == 0) << PSR::Z_SHIFT)
                    | (shftOperand.shftCarryOut << PSR::C_SHIFT);
                cpu.cpsr((cpu.cpsr() & ~PSR::NZC) | nzc);
            }
        }
        cpu.r(dp.rd, dest);
        break;
    }
    case Instruction::EOR: {
        auto& dp = std::get<DP::Data>(instruction.data);
        DP::ShftOperand shftOperand = executeShftOperand(cpu, dp);
        uint32_t operand1 = cpu.r(dp.rn);
        uint32_t operand2 = shftOperand.operand;
        uint32_t dest     = operand1 ^ operand2;
        if (dp.s) {
            if (dp.rd == GPR::PC) {
                cpu.copySpsr2Cpsr();
            } else {
                const uint32_t nzc = (dest & PSR::N)
                    | ((dest == 0) << PSR::Z_SHIFT)
                    | (shftOperand.shftCarryOut << PSR::C_SHIFT);
                cpu.cpsr((cpu.cpsr() & ~PSR::NZC) | nzc);
            }
        }
        cpu.r(dp.rd, dest);
        break;
    }
    case Instruction::SUB: {
        auto& dp = std::get<DP::Data>(instruction.data);
        DP::ShftOperand shftOperand = executeShftOperand(cpu, dp);
        uint32_t operand1 = cpu.r(dp.rn);
        uint32_t operand2 = shftOperand.operand;
        uint32_t dest     = operand1 - operand2;
        if (dp.s) {
            if (dp.rd == GPR::PC) {
                cpu.copySpsr2Cpsr();
            } else {
                const bool n1 = (operand1 & Bit::WORD_MSB_MASK) != 0;
                const bool n2 = (operand2 & Bit::WORD_MSB_MASK) != 0;
                const bool nResult = (dest & Bit::WORD_MSB_MASK) != 0;

                const uint32_t nzcv = (dest & PSR::N)
                    | ((dest == 0) << PSR::Z_SHIFT)
                    | ((operand1 >= operand2) << PSR::C_SHIFT)
                    | (((n1 != n2) && (nResult != n1)) << PSR::V_SHIFT);

                cpu.cpsr((cpu.cpsr() & ~PSR::NZCV) | nzcv);
            }
        }
        cpu.r(dp.rd, dest);
        break;
    }
    case Instruction::RSB: {
        auto& dp = std::get<DP::Data>(instruction.data);
        DP::ShftOperand shftOperand = executeShftOperand(cpu, dp);
        uint32_t operand1 = shftOperand.operand;
        uint32_t operand2 = cpu.r(dp.rn);
        uint32_t dest     = operand1 - operand2;
        if (dp.s) {
            if (dp.rd == GPR::PC) {
                cpu.copySpsr2Cpsr();
            } else {
                const bool n1 = (operand1 & Bit::WORD_MSB_MASK) != 0;
                const bool n2 = (operand2 & Bit::WORD_MSB_MASK) != 0;
                const bool nResult = (dest & Bit::WORD_MSB_MASK) != 0;

                const uint32_t nzcv = (dest & PSR::N)
                    | ((dest == 0) << PSR::Z_SHIFT)
                    | ((operand1 >= operand2) << PSR::C_SHIFT)
                    | (((n1 != n2) && (nResult != n1)) << PSR::V_SHIFT);

                cpu.cpsr((cpu.cpsr() & ~PSR::NZCV) | nzcv);
            }
        }
        cpu.r(dp.rd, dest);
        break;
    }
    case Instruction::ADD: {
        auto& dp = std::get<DP::Data>(instruction.data);
        DP::ShftOperand shftOperand = executeShftOperand(cpu, dp);
        uint32_t operand1 = cpu.r(dp.rn);
        uint32_t operand2 = shftOperand.operand;
        uint32_t dest     = operand1 + operand2;
        if (dp.s) {
            if (dp.rd == GPR::PC) {
                cpu.copySpsr2Cpsr();
            } else {
                const bool n1 = (operand1 & Bit::WORD_MSB_MASK) != 0;
                const bool n2 = (operand2 & Bit::WORD_MSB_MASK) != 0;
                const bool nResult = (dest & Bit::WORD_MSB_MASK) != 0;

                const uint32_t nzcv = (dest & PSR::N)
                    | ((dest == 0) << PSR::Z_SHIFT)
                    | ((dest < operand1) << PSR::C_SHIFT)
                    | (((n1 == n2) && (nResult != n1)) << PSR::V_SHIFT);

                cpu.cpsr((cpu.cpsr() & ~PSR::NZCV) | nzcv);
            }
        }
        cpu.r(dp.rd, dest);
        break;
    }
    case Instruction::ADC: {
        auto& dp = std::get<DP::Data>(instruction.data);
        DP::ShftOperand shftOperand = executeShftOperand(cpu, dp);
        uint32_t operand1 = cpu.r(dp.rn);
        uint32_t operand2 = shftOperand.operand;
        uint32_t dest     = 0;
        const bool c = (cpu.cpsr() & PSR::C) != 0;
        uint32_t partial = operand1 + operand2;
        dest = partial + c;
        if (dp.s) {
            if (dp.rd == GPR::PC) {
                cpu.copySpsr2Cpsr();
            } else {
                const bool n1 = (operand1 & Bit::WORD_MSB_MASK) != 0;
                const bool n2 = (operand2 & Bit::WORD_MSB_MASK) != 0;
                const bool nResult = (dest & Bit::WORD_MSB_MASK) != 0;

                const uint32_t nzcv = (dest & PSR::N)
                    | ((dest == 0) << PSR::Z_SHIFT)
                    | (((partial < operand1) || (dest < partial)) << PSR::C_SHIFT)
                    | (((n1 == n2) && (nResult != n1)) << PSR::V_SHIFT);

                cpu.cpsr((cpu.cpsr() & ~PSR::NZCV) | nzcv);
            }
        }
        cpu.r(dp.rd, dest);
        break;
    }
    case Instruction::SBC: {
        const bool c = (cpu.cpsr() & PSR::C) != 0;
        auto& dp = std::get<DP::Data>(instruction.data);
        DP::ShftOperand shftOperand = executeShftOperand(cpu, dp);
        uint32_t operand1 = cpu.r(dp.rn);
        uint32_t operand2 = shftOperand.operand;
        uint32_t dest     = operand1 - operand2 - !c;
        if (dp.s) {
            if (dp.rd == GPR::PC) {
                cpu.copySpsr2Cpsr();
            } else {
                const bool n1 = (operand1 & Bit::WORD_MSB_MASK) != 0;
                const bool n2 = (operand2 & Bit::WORD_MSB_MASK) != 0;
                const bool nResult = (dest & Bit::WORD_MSB_MASK) != 0;

                const uint32_t nzcv = (dest & PSR::N)
                    | ((dest == 0) << PSR::Z_SHIFT)
                    | (((operand1 > operand2) || (operand1 == operand2 && c)) << PSR::C_SHIFT)
                    | (((n1 != n2) && (nResult != n1)) << PSR::V_SHIFT);

                cpu.cpsr((cpu.cpsr() & ~PSR::NZCV) | nzcv);
            }
        }
        cpu.r(dp.rd, dest);
        break;
    }
    case Instruction::RSC: {
        const bool c = (cpu.cpsr() & PSR::C) != 0;
        auto& dp = std::get<DP::Data>(instruction.data);
        DP::ShftOperand shftOperand = executeShftOperand(cpu, dp);
        uint32_t operand1 = shftOperand.operand;
        uint32_t operand2 = cpu.r(dp.rn);
        uint32_t dest     = operand1 - operand2 - !c;
        if (dp.s) {
            if (dp.rd == GPR::PC) {
                cpu.copySpsr2Cpsr();
            } else {
                const bool n1 = (operand1 & Bit::WORD_MSB_MASK) != 0;
                const bool n2 = (operand2 & Bit::WORD_MSB_MASK) != 0;
                const bool nResult = (dest & Bit::WORD_MSB_MASK) != 0;

                const uint32_t nzcv = (dest & PSR::N)
                    | ((dest == 0) << PSR::Z_SHIFT)
                    | (((operand1 > operand2) || (operand1 == operand2 && c)) << PSR::C_SHIFT)
                    | (((n1 != n2) && (nResult != n1)) << PSR::V_SHIFT);

                cpu.cpsr((cpu.cpsr() & ~PSR::NZCV) | nzcv);
            }
        }
        cpu.r(dp.rd, dest);
        break;
    }
    case Instruction::TST:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::TEQ:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::CMP:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::CMN:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::ORR:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::MOV:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::BIC:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::MVN:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::MLA:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::MUL:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::SMLA:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::SMLAD:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::SMLAL:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::SMLAL_XY:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::SMLALD:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::SMLAW:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::SMLSD:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::SMLSLD:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::SMMLA:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::SMMLS:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::SMMUL:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::SMUAD:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::SMUL:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::SMULL:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::SMULW:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::SMUSD:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::UMAAL:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::UMLAL:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::UMULL:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::MRS:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::MSR:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::CPS:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::SETEND:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::LDR:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::LDRB:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::LDRBT:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::LDRD:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::LDREX:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::LDRH:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::LDRSB:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::LDRSH:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::LDRT:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::STR:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::STRB:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::STRBT:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::STRD:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::STREX:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::STRH:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::STRT:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::LDM:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::STM:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::B_BL:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::BX:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::SWP:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::SWPB:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::BKPT:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::SWI:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::CDP:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::LDC:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::MCR:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::MCRR:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::MRC:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::MRRC:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    case Instruction::STC:
        std::cerr << "Instruction NOT implemented!\n";
        break;
    }
}

std::string showRegisterState(Arm7tdmi &cpu) {
    auto gpr = cpu.getRegisters();
    auto sr = cpu.getStatusRegisters();

    return std::format(
    R"(r0:     {:08x}
r1:     {:08x}
r2:     {:08x}
r3:     {:08x}
r4:     {:08x}
r5:     {:08x}
r6:     {:08x}
r7:     {:08x}

r8:     {:08x},                                                                                 r8_fiq:   {:08x}
r9:     {:08x},                                                                                 r9_fiq:   {:08x}
r10:    {:08x},                                                                                 r10_fiq:  {:08x}
r11:    {:08x},                                                                                 r11_fiq:  {:08x}
r12:    {:08x},                                                                                 r12_fiq:  {:08x}

r13_sp: {:08x}, r13_svc:  {:08x}, r13_abt:  {:08x}, r13_und:  {:08x}, r13_irq:  {:08x}, r13_fiq:  {:08x}
r14_lr: {:08x}, r14_svc:  {:08x}, r14_abt:  {:08x}, r14_und:  {:08x}, r14_irq:  {:08x}, r14_fiq:  {:08x}
r(GPR::PC: {:08x}

cpsr:   {:08x}, spsr_svc: {:08x}, spsr_abt: {:08x}, spsr_und: {:08x}, spsr_irq: {:08x}, spsr_fiq: {:08x}
)",
        gpr.r[0],
        gpr.r[1],
        gpr.r[2],
        gpr.r[3],
        gpr.r[4],
        gpr.r[5],
        gpr.r[6],
        gpr.r[7],
        gpr.r[8], gpr.fiq[0],
        gpr.r[9], gpr.fiq[1],
        gpr.r[10], gpr.fiq[2],
        gpr.r[11], gpr.fiq[3],
        gpr.r[12], gpr.fiq[4],
        gpr.r[13], gpr.svc[0], gpr.abt[0], gpr.und[0], gpr.irq[0], gpr.fiq[0],
        gpr.r[14], gpr.svc[1], gpr.abt[1], gpr.und[1], gpr.irq[1],gpr.fiq[1],
        gpr.r[15],
        sr.cpsr, sr.spsr_svc, sr.spsr_abt, sr.spsr_und, sr.spsr_irq, sr.spsr_fiq
        );
}
