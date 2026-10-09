//
// Created by daniel on 10/1/26.
//

#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

enum class Endianness { LITTLE, BIG };

namespace AddressSpace {
    constexpr uint32_t MIN = 0x00000000u;
    constexpr uint32_t MAX = 0xFFFFFFFF;
    constexpr uint32_t W_ALIGN_MASK = 0b11;
    constexpr uint32_t HW_ALIGN_MASK = 0b1;

}

/*
 * Highest 1    Reset
 *         2    Data Abort (including data TLB miss)
 *         3    FIQ
 *         4    IRQ
 *         5    Imprecise Abort (external abort) - ARMv6
 *         6    Prefetch Abort (including prefetch TLB miss)
 * Lowest  7    Undefined instruction and SWI
 */
enum class Exception {
    RESET,
    UNDEFINED_INSTRUCTION,
    SOFTWARE_INTERRUPT,
    PREFETCH_ABORT,
    DATA_ABORT,
    IRQ,
    FIQ
};

namespace ExceptionVecAddr {
    // Only normal address vector is supported by ARM7TDMI, DDI0210C 2.8.9
    constexpr uint32_t RESET                 = 0x00000000u;
    constexpr uint32_t UNDEFINED_INSTRUCTION = 0x00000004u;
    constexpr uint32_t SOFTWARE_INTERRUPT    = 0x00000008u;
    constexpr uint32_t PREFETCH_ABORT        = 0x0000000Cu;
    constexpr uint32_t DATA_ABORT            = 0x00000010u;
    constexpr uint32_t IRQ                   = 0x00000018u;
    constexpr uint32_t FIQ                   = 0x00000001u;
}

/*
 * Instruction Type
 * Branch instructions
 * Data-processing instructions | page A1-7
 * Status register transfer instructions | page A1-8
 * Load and store instructions | page A1-8
 * Coprocessor instructions | page A1-10
 * Exception-generating instructions | page A1-10.
*/
enum class InstructionType {
    DATA_PROCESSING,
    LOAD_STORE,
    BRANCH,
    STATUS_REGISTER_TRANSFER,
    COPROCESSOR,
    EXCEPTION_GENERATING,
    UNDEFINED
};

enum class Mnemonic {
    AND,        // Data processing instructions
    EOR,
    SUB,
    RSB,
    ADD,
    ADC,
    SBC,
    RSC,
    TST,
    TEQ,
    CMP,
    CMN,
    ORR,
    MOV,
    BIC,
    MVN,
    MLA,        // Multiply instructions
    MUL,
    SMLA,
    SMLAD,
    SMLAL,
    SMLAL_XY,
    SMLALD,
    SMLAW,
    SMLSD,
    SMLSLD,
    SMMLA,
    SMMLS,
    SMMUL,
    SMUAD,
    SMUL,
    SMULL,
    SMULW,
    SMUSD,
    UMAAL,
    UMLAL,
    UMULL,
    MRS,        // Status register access instructions
    MSR,
    CPS,
    SETEND,
    LDR,        // Load and store instructions
    LDRB,
    LDRBT,
    LDRD,
    LDREX,
    LDRH,
    LDRSB,
    LDRSH,
    LDRT,
    STR,
    STRB,
    STRBT,
    STRD,
    STREX,
    STRH,
    STRT,
    LDM,        // Load and store multiple
    STM,
    B_BL,       // Branch instructions
    BX,
    // BLX: Version 5 and above.
    // BXJ: Version 6 and above, plus ARMv5TEJ.
    SWP,        // Semaphore instructions
    SWPB,
    BKPT,       // Exception-generating instructions
    SWI,
    CDP,        // Coprocessor instructions
    LDC,
    MCR,
    MCRR,
    MRC,
    MRRC,
    STC,
};

namespace Opcode {
    enum class Cond : uint8_t {
        EQ    = 0b0000, // Z == 1
        NE    = 0b0001, // Z == 0
        CS_HS = 0b0010, // C == 1
        CC_LO = 0b0011, // C == 0
        MI    = 0b0100, // N == 1
        PL    = 0b0101, // N == 0
        VS    = 0b0110, // V == 1
        VC    = 0b0111, // V == 0
        HI    = 0b1000, // C == 1 && Z == 0
        LS    = 0b1001, // C == 0 || Z == 1
        GE    = 0b1010, // N == V
        LT    = 0b1011, // N != V
        GT    = 0b1100, // Z == 0 && N == V
        LE    = 0b1101, // Z == 1 || N != V
        AL    = 0b1110, // Always execute
        UNCONDITIONAL = 0b1111u  // Unpredictable prior to ARMv5
    };

    enum class DP : uint8_t {
        AND = 0b0000,
        EOR = 0b0001,
        SUB = 0b0010,
        RSB = 0b0011,
        ADD = 0b0100,
        ADC = 0b0101,
        SBC = 0b0110,
        RSC = 0b0111,
        TST = 0b1000,
        TEQ = 0b1001,
        CMP = 0b1010,
        CMN = 0b1011,
        ORR = 0b1100,
        MOV = 0b1101,
        BIC = 0b1110,
        MVN = 0b1111
    };
}

/*
 * Data processing instruction codec
 *
 * Instruction encoding conventions:
 *
 * 1-bit boolean flag:
 *   - Use only FLAG.
 *   - No SHIFT or VALUE is normally needed.
 *   - Use the *_FLAG suffix instead of *_MASK to make it explicit that
 *     the constant represents a single boolean bit.
 *
 *   constexpr uint32_t P_FLAG = 1u << 24;
 *   bool p = (raw & P_FLAG) != 0;
 *
 * Multi-bit field that can hold multiple enumerated values:
 *   - Use MASK + SHIFT.
 *   - Extract and normalize the value before using it.
 *
 *   constexpr uint32_t RN_SHIFT = 16;
 *   constexpr uint32_t RN_MASK  = 0b1111u << RN_SHIFT;
 *   auto rn = (raw & RN_MASK) >> RN_SHIFT;
 *
 * Raw instruction pattern:
 *   - Use MASK + VALUE when the expected pattern contains specific 0 and 1 bits.
 *
 *   constexpr uint32_t MASK  = 0b1111u << 24;
 *   constexpr uint32_t VALUE = 0b1110u << 24;
 *
 *   if ((raw & MASK) == VALUE) { ... }
 *
 * Pattern where every relevant bit must be 1:
 *   - VALUE is redundant; compare directly against MASK.
 *
 *   if ((raw & MASK) == MASK) { ... }
 *
 * Important:
 *   if (raw & MASK)
 *
 * only checks whether AT LEAST ONE masked bit is set.
 * It does not check whether all masked bits match.
 *
 * Naming rules:
 *   *_FLAG       -> single-bit boolean flag
 *   *_MASK       -> selects multiple bits
 *   *_SHIFT      -> normalizes/extracts a multi-bit field
 *   *_VALUE      -> expected raw bit pattern
 *
 * Literal suffix:
 *   - Use unsigned literals for bit operations: 1u, 0b111u, etc.
 *   - Normalized enum values do not need the 'u' suffix.
 */
namespace IEnc {
    // Common instruction fields
    constexpr uint32_t COND_SHIFT  = 28;
    constexpr uint32_t COND_MASK   = 0b1111u << COND_SHIFT;
    constexpr uint32_t CLASS_SHIFT = 25;
    constexpr uint32_t CLASS_MASK  = 0b111u << CLASS_SHIFT;

    namespace Cond {
        constexpr uint8_t UNCONDITIONAL_VALUE = 0b1111;
    }

    enum class Class : uint8_t {
        C000     = 0b000,
        C001     = 0b001,
        LSIO     = 0b010,    // Load/store immediate offset
        C011     = 0b011,
        LSMU     = 0b100,    // Load/store multiple
        BRANCH   = 0b101,    // Branch and branch with link
        COPLSDRT = 0b110,    // Coprocessor load/store and double register transfers
        C111     = 0b111,
    };

    namespace C000 {
        // Decode patterns
        constexpr uint32_t MULT_DEC_MASK = (0b1 << 7) | (0b1 << 4);
        constexpr uint32_t MULT_VALUE    = MULT_DEC_MASK;     // Multiplies/Extra load/store instructions decode pattern

        constexpr uint32_t MISC_DEC_MASK = (0b11 << 23) | (0b1 << 20);
        constexpr uint32_t MISC_VALUE    = 0b1 << 24;     // Miscellaneous instructions decode pattern
    }

    namespace C001 {
        constexpr uint32_t DECODE_MASK = (0b11 << 23) | (0b11 << 20);

        // Move immediate to status register flags
        constexpr uint32_t MISR_VALUE = (0b10 << 23) | (0b10 << 20);  // Move immediate to status register decode pattern
        constexpr uint32_t MASK_SHIFT = 16;
        constexpr uint32_t MASK_MASK  = 0b1111u << MASK_SHIFT;
        constexpr uint32_t SBO_SHIFT  = 12;
        constexpr uint32_t SBO_MASK   = 0b1111u << SBO_SHIFT;

        // Undefined instruction flags
        constexpr uint32_t UNDEFINED_VALUE = 0b10 << 23;       // Undefined instruction decode pattern

        // Data processing immediate flags
        constexpr uint32_t OPCODE_SHIFT = 21;
        constexpr uint32_t OPCODE_MASK = 0b1111u << OPCODE_SHIFT;
        constexpr uint32_t RN_FLAG = 0b1 << 20;
        constexpr uint32_t RN_MASK = MASK_MASK;
        constexpr uint32_t RD_MASK = SBO_MASK;

        // Common flags
        constexpr uint32_t ROTATE_SHIFT = 8;
        constexpr uint32_t ROTATE_MASK  = 0b1111u << ROTATE_SHIFT;
        constexpr uint32_t IMMEDIATE_MASK = 0xFF;
    }

    namespace C010_LSIO {
        constexpr uint32_t P_MASK = 0b1u << 24;
        constexpr uint32_t U_MASK = 0b1u << 23;
        constexpr uint32_t B_MASK = 0b1u << 22;
        constexpr uint32_t W_MASK = 0b1u << 21;
        constexpr uint32_t L_MASK = 0b1u << 20;

        constexpr uint32_t RN_SHIFT = 16;
        constexpr uint32_t RN_MASK  = 0b1111u << RN_SHIFT;
        constexpr uint32_t RD_SHIFT = 12;
        constexpr uint32_t RD_MASK  = 0b1111u << RD_SHIFT;

        constexpr uint32_t IMMEDIATE_MASK = 0xFFF;
    }

    namespace C011 {
        constexpr uint32_t ARCH_UNDEF_MASK  = (0b11111 << 20) | (0b1111 << 4);
        constexpr uint32_t ARCH_UNDEF_VALUE = (0b11111 << 20) | (0b1111 << 4);

        constexpr uint32_t P_MASK = 0b1u << 24;
        constexpr uint32_t U_MASK = 0b1u << 23;
        constexpr uint32_t B_MASK = 0b1u << 22;
        constexpr uint32_t W_MASK = 0b1u << 21;
        constexpr uint32_t L_MASK = 0b1u << 20;

        constexpr uint32_t RN_SHIFT = 16;
        constexpr uint32_t RN_MASK  = 0b1111u << RN_SHIFT;
        constexpr uint32_t RD_SHIFT = 12;
        constexpr uint32_t RD_MASK  = 0b1111u << RD_SHIFT;
        constexpr uint32_t SHIFT_AMOUNT_SHIFT = 7;
        constexpr uint32_t SHIFT_AMOUNT_MASK  = 0b11111u << SHIFT_AMOUNT_SHIFT;
        constexpr uint32_t SHIFT_SHIFT = 5;
        constexpr uint32_t SHIFT_MASK  = 0b11u << SHIFT_SHIFT;

        // Media instruction when set and load/store register offset when clear
        constexpr uint32_t MEDIA_MASK = 0b1 << 4;

        constexpr uint32_t RM_MASK = 0b1111u;
    }

    namespace C100_LSMU {
        constexpr uint32_t P_FLAG = 0b1u << 24;
        constexpr uint32_t U_FLAG = 0b1u << 23;
        constexpr uint32_t S_FLAG = 0b1u << 22;
        constexpr uint32_t W_FLAG = 0b1u << 21;
        constexpr uint32_t L_FLAG = 0b1u << 20;

        constexpr uint32_t RN_SHIFT = 16;
        constexpr uint32_t RN_MASK  = 0b1111 << RN_SHIFT;

        constexpr uint32_t REG_LIST_MASK = 0xFF'FF;
    }

    namespace C101_BRANCH {
        constexpr uint32_t L_FLAG = 0b1 << 24;
        constexpr uint32_t OFFSET_MASK = 0xFF'FF'FF;
    }

    namespace C110_COP {
        constexpr uint32_t P_FLAG = 0b1u << 24;
        constexpr uint32_t U_FLAG = 0b1u << 23;
        constexpr uint32_t N_FLAG = 0b1u << 22;
        constexpr uint32_t W_FLAG = 0b1u << 21;
        constexpr uint32_t L_FLAG = 0b1u << 20;

        constexpr uint32_t RN_SHIFT = 16;
        constexpr uint32_t RN_MASK  = 0b1111 << RN_SHIFT;
        constexpr uint32_t CRD_SHIFT = 12;
        constexpr uint32_t CRD_MASK  = 0b1111 << CRD_SHIFT;
        constexpr uint32_t CPN_SHIFT = 8;
        constexpr uint32_t CPN_MASK  = 0b1111 << CPN_SHIFT;

        constexpr uint32_t OFFSET_MASK  = 0xFF;
    }

    namespace C111 {
        namespace SWI {
            constexpr uint32_t SWI_FLAG     = 0b1u << 24;
            constexpr uint32_t NUMBER_MASK  = 0xFF'FF'FF;
            constexpr uint32_t PREFIX_SHIFT = 24;
            constexpr uint32_t PREFIX_MASK  = 0b1111u << PREFIX_SHIFT;
        }
        namespace COP {
            // Register transfer
            constexpr uint32_t RT_OPCODE1_SHIFT = 21;
            constexpr uint32_t RT_OPCODE1_MASK  = 0b111u << RT_OPCODE1_SHIFT;
            constexpr uint32_t L_FLAG           = 0b1u << 20;
            constexpr uint32_t RD_SHIFT         = 12;
            constexpr uint32_t RD_MASK          = 0b1111u << RD_SHIFT;

            // Data Transfer
            constexpr uint32_t DP_OPCODE1_SHIFT = 20;
            constexpr uint32_t DP_OPCODE1_MASK  = 0b1111u << DP_OPCODE1_SHIFT;
            constexpr uint32_t CRD_MASK         = RD_MASK;

            // Common flags
            constexpr uint32_t CRN_SHIFT     = 16;
            constexpr uint32_t CRN_MASK      = 0b1111 << CRN_SHIFT;
            constexpr uint32_t OPCODE2_SHIFT = 6;
            constexpr uint32_t OPCODE2_MASK  = 0b111u << OPCODE2_SHIFT;
            constexpr uint32_t RT_FLAG       = 0b1 << 4;      // Register transfer when set, data processing when clear
            constexpr uint32_t CRM_MASK      = 0b1111;
        }
    }

    namespace DP {
        // Common flags
        constexpr uint32_t I_FLAG       = 0b1u << 25;       // Distinguishes between immediate and register operands
        constexpr uint32_t OPCODE_SHIFT = 21;
        constexpr uint32_t OPCODE_MASK  = 0b1111u << OPCODE_SHIFT;
        constexpr uint32_t S_FLAG       = 0b1u << 20;
        constexpr uint32_t RN_SHIFT     = 16;
        constexpr uint32_t RN_MASK      = 0b1111u << RN_SHIFT;
        constexpr uint32_t RD_SHIFT     = 12;
        constexpr uint32_t RD_MASK      = 0b1111u << RD_SHIFT;
        // TODO: Verify if is necessary shifter mask to get shifter fields
        constexpr uint32_t SHIFTER_MASK = 0xFFF;

        // Data processing register flags
        constexpr uint32_t RS_SHIFT     = 8;
        constexpr uint32_t RS_MASK      = 0b1111u << RS_SHIFT;

        constexpr uint32_t SHIFT_AMOUNT_SHIFT = 7;
        constexpr uint32_t SHIFT_AMOUNT_MASK  = 0b11u << SHIFT_AMOUNT_SHIFT;
        constexpr uint32_t BIT7_MASK = 0b1u << 7;
        constexpr uint32_t BIT4_MASK = 0b1u << 4;

        constexpr uint32_t SHIFT_SHIFT = 5;
        constexpr uint32_t SHIFT_MASK  = 0b11u << SHIFT_SHIFT;
        constexpr uint32_t RM_MASK     = 0b1111u;

        // Data processing immediate shift flags

        // Data processing immediate
        constexpr uint32_t ROTATE_SHIFT = 8;
        constexpr uint32_t ROTATE_MASK  = 0b1111 << ROTATE_SHIFT;
        constexpr uint32_t IMM_MASK  = 0xFF;
    }

}

// Data processing immediate shift, shifter operand data
struct DPIS {
    uint8_t shiftAmount;
    uint8_t shift;
    uint8_t rm;
};

// Data processing register shift, shifter operand data
struct DPRS {
    uint8_t rs;
    uint8_t shift;
    uint8_t rm;
};

// Data processing immediate, shifter operand data
struct DPI {
    uint8_t rotate;
    uint8_t immediate;
};

using DPShifter = std::variant<
    DPIS,
    DPRS,
    DPI
>;

struct DataProcessing {
    Opcode::DP opcode;
    bool isImmediate;
    bool isUpdateCond;
    uint8_t rn;
    uint8_t rd;

    DPShifter shifterOperand;
};

struct Miscellaneous {
    // TODO: Check if miscellaneous instruction has usable fields on ARMv4T
};

using InstructionData = std::variant<
    DataProcessing,
    Miscellaneous
>;

struct DecodedOperation {
    InstructionType type;
    Mnemonic mnemonic;
    InstructionData instructionData;
};

struct DecodedInstruction {
    uint32_t rawInstruction;

    InstructionType type;
    Mnemonic mnemonic;

    Opcode::Cond cond;   // 31-28 - condition
    IEnc::Class clazz;  // 27-24 - class

    // Data processing instruction
    InstructionData instructionData;
};

enum class ProcessorMode : uint8_t {
    USER        = 0b10000u,
    FIQ         = 0b10001u,
    IRQ         = 0b10010u,
    SUPERVISOR  = 0b10011u,
    ABORT       = 0b10111u,
    UNDEFINED   = 0b11011u,
    SYSTEM      = 0b11111u,
};

std::string exceptionDescription(Exception exception);

// CPSR / SPSR Flags
namespace PSR {
    constexpr uint32_t N = 1u << 31;    // Negative, condition code, user-writable bit
    constexpr uint32_t Z = 1u << 30;    // Zero,     condition code, user-writable bit
    constexpr uint32_t C = 1u << 29;    // Carry,    condition code, user-writable bit
    constexpr uint32_t V = 1u << 28;    // OVerflow, condition code, user-writable bit
    // 27 Q Prior to ARMv5 must be treated as reserved bit
    // 26-25 RESERVED
    // 24 J Prior to ARMv5TEJ must be treated as reserved bit
    // 19-16 GE[3:0] Prior to ARMv6 must be treated as reserved bit
    // 15-10 RESERVED
    // 9 E Prior to ARMv6 must be treated as reserved bit
    // 8 A Prior to ARMv6 must be treated as reserved bit
    constexpr uint32_t I = 1u << 7;     // Disables IRQ interrupt, Privileged bit
    constexpr uint32_t F = 1u << 6;     // Disables FIQ interrupt, Privileged bit
    constexpr uint32_t T = 1u << 5;     // Select the current instruction set, Execution state bit
    // 4-0 M[4:0]
    constexpr uint32_t MODE_MASK = 0b11111u;

    constexpr uint32_t RESERVED_MASK = 0b11111111111111111111100000000u;
}

// ARMv4T ISA Implementation
struct Arm7tdmi {
    // General, visible, user and system mode registers
    uint32_t r0{};        // r0 - r7 unbanked
    uint32_t r1{};
    uint32_t r2{};
    uint32_t r3{};
    uint32_t r4{};
    uint32_t r5{};
    uint32_t r6{};
    uint32_t r7{};
    uint32_t r8{};        // r8 - r14 banked
    uint32_t r9{};
    uint32_t r10{};
    uint32_t r11{};
    uint32_t r12{};
    uint32_t r13_sp{};    // Stack Pointer
    uint32_t r14_lr{};    // Link Register
    uint32_t r15_pc{};    // Program Counter

    uint32_t cpsr{};  // Current Program Status Register

    // Invisible Registers
    // For more details on status registers, refer to Program status registers on page A1.1.3 and A2-11.
    // Supervisor banked register
    uint32_t r13_svc{};
    uint32_t r14_svc{};
    uint32_t spsr_svc{};  // Saved Program Status Register

    // Abort banked register
    uint32_t r13_abt{};
    uint32_t r14_abt{};
    uint32_t spsr_abt{};

    // Undefined banked register
    uint32_t r13_und{};
    uint32_t r14_und{};
    uint32_t spsr_und{};

    // Interrupt banked register
    uint32_t r13_irq{};
    uint32_t r14_irq{};
    uint32_t spsr_irq{};

    // Fast Interrupt banked register
    uint32_t r8_fiq{};
    uint32_t r9_fiq{};
    uint32_t r10_fiq{};
    uint32_t r11_fiq{};
    uint32_t r12_fiq{};
    uint32_t r13_fiq{};
    uint32_t r14_fiq{};
    uint32_t spsr_fiq{};

    uint32_t currentInstAddr{};

    Endianness endianness = Endianness::BIG;
};

void reset(Arm7tdmi& cpu);

void undefinedInstruction(Arm7tdmi& cpu);

void softwareInterrupt(Arm7tdmi& cpu);

void prefetchAbort(Arm7tdmi& cpu);

void dataAbort(Arm7tdmi cpu);

void irq(Arm7tdmi& cpu);

void fiq(Arm7tdmi& cpu);

DecodedInstruction decodeInstruction(uint32_t rawInstruction);

void executeProgram(Arm7tdmi& cpu, std::vector<uint32_t> &program);

void executeInstruction(Arm7tdmi& cpu, uint32_t instruction);

std::string showRegisterState(Arm7tdmi& cpu);