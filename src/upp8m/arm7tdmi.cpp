//
// Created by daniel on 10/1/26.
//

#include "arm7tdmi.h"

#include <format>
#include <iostream>

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
void reset(Arm7tdmi &cpu) {
    cpu.cpsr = static_cast<uint32_t>(ProcessorMode::SUPERVISOR) |
        PSR::I |
        PSR::F;
    cpu.r15_pc = ExceptionVecAddr::RESET;
}

void undefinedInstruction(Arm7tdmi &cpu) {
    // TODO: The r14 must be set to the address of the next instruction after the Undefined instruction. Check if it is
    //  PC
    cpu.r14_und = cpu.currentInstAddr + 4;      // address of next instruction after the Undefined instruction
    cpu.spsr_und = cpu.cpsr;

    cpu.cpsr = (cpu.cpsr & ~PSR::MODE_MASK) | static_cast<uint32_t>(ProcessorMode::UNDEFINED);
    cpu.cpsr &= ~PSR::T;        // Execute in ARM state
    cpu.cpsr |= PSR::I;         // Disable normal interrupts

    cpu.r15_pc = ExceptionVecAddr::UNDEFINED_INSTRUCTION;
}


void softwareInterrupt(Arm7tdmi &cpu) {
    cpu.r14_svc = cpu.currentInstAddr + 4;      // address of next instruction after the SWI instruction
    cpu.spsr_svc = cpu.cpsr;

    cpu.cpsr = (cpu.cpsr & ~PSR::MODE_MASK) | static_cast<uint32_t>(ProcessorMode::SUPERVISOR);
    cpu.cpsr &= ~PSR::T;        // Execute in ARM state
    cpu.cpsr |= PSR::I;         // Disable normal interrupts

    cpu.r15_pc = ExceptionVecAddr::SOFTWARE_INTERRUPT;
}


void prefetchAbort(Arm7tdmi &cpu) {
    cpu.r14_abt = cpu.currentInstAddr + 4;      // address of the aborted instruction + 4
    cpu.spsr_abt = cpu.cpsr;

    cpu.cpsr = (cpu.cpsr & ~PSR::MODE_MASK) | static_cast<uint32_t>(ProcessorMode::ABORT);
    cpu.cpsr &= ~PSR::T;        // Execute in ARM state
    cpu.cpsr |= PSR::I;         // Disable normal interrupts

    cpu.r15_pc = ExceptionVecAddr::PREFETCH_ABORT;
}


void dataAbort(Arm7tdmi cpu) {
    // TODO: Learn about imprecise data aborts that could lead to an unrecoverable state, hang
    cpu.r14_abt = cpu.currentInstAddr + 8;      // address of the aborted instruction + 8
    cpu.spsr_abt = cpu.cpsr;

    cpu.cpsr = (cpu.cpsr & ~PSR::MODE_MASK) | static_cast<uint32_t>(ProcessorMode::ABORT);
    cpu.cpsr &= ~PSR::T;        // Execute in ARM state
    cpu.cpsr |= PSR::I;         // Disable normal interrupts

    cpu.r15_pc = ExceptionVecAddr::DATA_ABORT;
}


void irq(Arm7tdmi &cpu) {
    cpu.r14_irq = cpu.currentInstAddr + 8;      // address of next instruction to be executed + 4
    cpu.spsr_irq = cpu.cpsr;

    cpu.cpsr = (cpu.cpsr & ~PSR::MODE_MASK) | static_cast<uint32_t>(ProcessorMode::IRQ);
    cpu.cpsr &= ~PSR::T;        // Execute in ARM state
    cpu.cpsr |= PSR::I;         // Disable normal interrupts

    cpu.r15_pc = ExceptionVecAddr::IRQ;
}


void fiq(Arm7tdmi &cpu) {
    cpu.r14_fiq = cpu.currentInstAddr + 8;      // address of next instruction to be executed + 4

    cpu.spsr_fiq = cpu.cpsr;

    cpu.cpsr = (cpu.cpsr & ~PSR::MODE_MASK) | static_cast<uint32_t>(ProcessorMode::FIQ);
    cpu.cpsr &= ~PSR::T;        // Execute in ARM state
    cpu.cpsr |= PSR::F;         // Disable fast interrupts
    cpu.cpsr |= PSR::I;         // Disable normal interrupts

    cpu.r15_pc = ExceptionVecAddr::FIQ;
}

// 115

DecodedInstruction decodeInstruction(uint32_t rawInstruction) {
    DecodedInstruction instruction{};
    instruction.cond = (rawInstruction & IEnc::COND_MASK) >> IEnc::COND_SHIFT;
    instruction.iclazz = (rawInstruction & IEnc::CLAZZ_MASK) >> IEnc::CLAZZ_SHIFT;
    auto iclazz = static_cast<IEnc::Class>(instruction.iclazz);

    if (instruction.cond == IEnc::Cond::UNCONDITIONAL_VALUE) {
        std::cerr << "Unconditional instruction prior to ARMv5 is UNPREDICTABLE" << std::endl;
        std::abort();
    }

    switch (iclazz) {
    case IEnc::Class::C000: {
        uint32_t decode = (rawInstruction & IEnc::C000::MISC_DEC_MASK);
        if (decode == IEnc::C000::MISC_VALUE) {
            // Miscellaneous instructions
            // TODO: Miscellaneous instructions
        }
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
        // Data processing immediate

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
    }
        break;
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


void execute(Arm7tdmi &cpu, uint32_t rawInstruction) {
    decodeInstruction(rawInstruction);
}
std::string showRegisterState(Arm7tdmi &cpu) {
    return std::format(
    R"(r0:     {:08x}
r1:     {:08X}
r2:     {:08X}
r3:     {:08X}
r4:     {:08X}
r5:     {:08X}
r6:     {:08X}
r7:     {:08X}

r8:     {:08X},                                                                                 r8_fiq:   {:08X}
r9:     {:08X},                                                                                 r9_fiq:   {:08X}
r10:    {:08X},                                                                                 r10_fiq:  {:08X}
r11:    {:08X},                                                                                 r11_fiq:  {:08X}
r12:    {:08X},                                                                                 r12_fiq:  {:08X}

r13_sp: {:08X}, r13_svc:  {:08X}, r13_abt:  {:08X}, r13_und:  {:08X}, r13_irq:  {:08X}, r13_fiq:  {:08X}
r14_lr: {:08X}, r14_svc:  {:08X}, r14_abt:  {:08X}, r14_und:  {:08X}, r14_irq:  {:08X}, r14_fiq:  {:08X}
r15_pc: {:08X}

cpsr:   {:08X}, spsr_svc: {:08X}, spsr_abt: {:08X}, spsr_und: {:08X}, spsr_irq: {:08X}, spsr_fiq: {:08X}
)",
        cpu.r0,
        cpu.r1,
        cpu.r2,
        cpu.r3,
        cpu.r4,
        cpu.r5,
        cpu.r6,
        cpu.r7,
        cpu.r8, cpu.r8_fiq,
        cpu.r9, cpu.r9_fiq,
        cpu.r10, cpu.r10_fiq,
        cpu.r11, cpu.r11_fiq,
        cpu.r12, cpu.r12_fiq,
        cpu.r13_sp, cpu.r13_svc, cpu.r13_abt, cpu.r13_und, cpu.r13_irq, cpu.r13_fiq,
        cpu.r14_lr, cpu.r14_svc, cpu.r14_abt, cpu.r14_und, cpu.r14_irq,cpu.r14_fiq,
        cpu.r15_pc,
        cpu.cpsr, cpu.spsr_svc, cpu.spsr_abt, cpu.spsr_und, cpu.spsr_irq, cpu.spsr_fiq
        );
}


