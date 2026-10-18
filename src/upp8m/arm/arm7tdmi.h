//
// Created by daniel on 10/1/26.
//

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

void CRASH_THIS_FUCKER(std::string message);

const std::string UNPREDICTABLE_MSG = "UNPREDICTABLE. OH MY GOOOOOOOOOOOD!\n";

enum class Endianness { LITTLE, BIG };

namespace AddressSpace {
    constexpr uint32_t MIN = 0x00000000;
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
    constexpr uint32_t RESET                 = 0x00000000;
    constexpr uint32_t UNDEFINED_INSTRUCTION = 0x00000004;
    constexpr uint32_t SOFTWARE_INTERRUPT    = 0x00000008;
    constexpr uint32_t PREFETCH_ABORT        = 0x0000000C;
    constexpr uint32_t DATA_ABORT            = 0x00000010;
    constexpr uint32_t IRQ                   = 0x00000018;
    constexpr uint32_t FIQ                   = 0x00000001;
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

enum class Instruction {
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
        UNCONDITIONAL = 0b1111  // Unpredictable prior to ARMv5
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
 *   constexpr uint32_t RN_MASK  = 0b1111 << RN_SHIFT;
 *   auto rn = (raw & RN_MASK) >> RN_SHIFT;
 *
 * Raw instruction pattern:
 *   - Use MASK + VALUE when the expected pattern contains specific 0 and 1 bits.
 *
 *   constexpr uint32_t MASK  = 0b1111 << 24;
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
 *   - Use unsigned literals for bit operations: 1u, 0b111, etc.
 *   - Normalized enum values do not need the 'u' suffix.
 */
namespace IEnc {
    // Common instruction fields
    constexpr uint32_t COND_SHIFT  = 28;
    constexpr uint32_t COND_MASK   = 0b1111 << COND_SHIFT;
    constexpr uint32_t CLASS_SHIFT = 25;
    constexpr uint32_t CLASS_MASK  = 0b111 << CLASS_SHIFT;

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
        constexpr uint32_t MASK_MASK  = 0b1111 << MASK_SHIFT;
        constexpr uint32_t SBO_SHIFT  = 12;
        constexpr uint32_t SBO_MASK   = 0b1111 << SBO_SHIFT;

        // Undefined instruction flags
        constexpr uint32_t UNDEFINED_VALUE = 0b10 << 23;       // Undefined instruction decode pattern

        // Data processing immediate flags
        constexpr uint32_t OPCODE_SHIFT = 21;
        constexpr uint32_t OPCODE_MASK = 0b1111 << OPCODE_SHIFT;
        constexpr uint32_t RN_FLAG = 0b1 << 20;
        constexpr uint32_t RN_MASK = MASK_MASK;
        constexpr uint32_t RD_MASK = SBO_MASK;

        // Common flags
        constexpr uint32_t ROTATE_SHIFT = 8;
        constexpr uint32_t ROTATE_MASK  = 0b1111 << ROTATE_SHIFT;
        constexpr uint32_t IMMEDIATE_MASK = 0xFF;
    }

    namespace C010_LSIO {
        constexpr uint32_t P_MASK = 0b1 << 24;
        constexpr uint32_t U_MASK = 0b1 << 23;
        constexpr uint32_t B_MASK = 0b1 << 22;
        constexpr uint32_t W_MASK = 0b1 << 21;
        constexpr uint32_t L_MASK = 0b1 << 20;

        constexpr uint32_t RN_SHIFT = 16;
        constexpr uint32_t RN_MASK  = 0b1111 << RN_SHIFT;
        constexpr uint32_t RD_SHIFT = 12;
        constexpr uint32_t RD_MASK  = 0b1111 << RD_SHIFT;

        constexpr uint32_t IMMEDIATE_MASK = 0xFFF;
    }

    namespace C011 {
        constexpr uint32_t ARCH_UNDEF_MASK  = (0b11111 << 20) | (0b1111 << 4);
        constexpr uint32_t ARCH_UNDEF_VALUE = (0b11111 << 20) | (0b1111 << 4);

        constexpr uint32_t P_MASK = 0b1 << 24;
        constexpr uint32_t U_MASK = 0b1 << 23;
        constexpr uint32_t B_MASK = 0b1 << 22;
        constexpr uint32_t W_MASK = 0b1 << 21;
        constexpr uint32_t L_MASK = 0b1 << 20;

        constexpr uint32_t RN_SHIFT = 16;
        constexpr uint32_t RN_MASK  = 0b1111 << RN_SHIFT;
        constexpr uint32_t RD_SHIFT = 12;
        constexpr uint32_t RD_MASK  = 0b1111 << RD_SHIFT;
        constexpr uint32_t SHIFT_AMOUNT_SHIFT = 7;
        constexpr uint32_t SHIFT_AMOUNT_MASK  = 0b11111u << SHIFT_AMOUNT_SHIFT;
        constexpr uint32_t SHIFT_SHIFT = 5;
        constexpr uint32_t SHIFT_MASK  = 0b11 << SHIFT_SHIFT;

        // Media instruction when set and load/store register offset when clear
        constexpr uint32_t MEDIA_MASK = 0b1 << 4;

        constexpr uint32_t RM_MASK = 0b1111;
    }

    namespace C100_LSMU {
        constexpr uint32_t P_FLAG = 0b1 << 24;
        constexpr uint32_t U_FLAG = 0b1 << 23;
        constexpr uint32_t S_FLAG = 0b1 << 22;
        constexpr uint32_t W_FLAG = 0b1 << 21;
        constexpr uint32_t L_FLAG = 0b1 << 20;

        constexpr uint32_t RN_SHIFT = 16;
        constexpr uint32_t RN_MASK  = 0b1111 << RN_SHIFT;

        constexpr uint32_t REG_LIST_MASK = 0xFF'FF;
    }

    namespace C110_COP {
        constexpr uint32_t P_FLAG = 0b1 << 24;
        constexpr uint32_t U_FLAG = 0b1 << 23;
        constexpr uint32_t N_FLAG = 0b1 << 22;
        constexpr uint32_t W_FLAG = 0b1 << 21;
        constexpr uint32_t L_FLAG = 0b1 << 20;

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
            constexpr uint32_t SWI_FLAG     = 0b1 << 24;
            constexpr uint32_t NUMBER_MASK  = 0xFF'FF'FF;
            constexpr uint32_t PREFIX_SHIFT = 24;
            constexpr uint32_t PREFIX_MASK  = 0b1111 << PREFIX_SHIFT;
        }
        namespace COP {
            // Register transfer
            constexpr uint32_t RT_OPCODE1_SHIFT = 21;
            constexpr uint32_t RT_OPCODE1_MASK  = 0b111 << RT_OPCODE1_SHIFT;
            constexpr uint32_t L_FLAG           = 0b1 << 20;
            constexpr uint32_t RD_SHIFT         = 12;
            constexpr uint32_t RD_MASK          = 0b1111 << RD_SHIFT;

            // Data Transfer
            constexpr uint32_t DP_OPCODE1_SHIFT = 20;
            constexpr uint32_t DP_OPCODE1_MASK  = 0b1111 << DP_OPCODE1_SHIFT;
            constexpr uint32_t CRD_MASK         = RD_MASK;

            // Common flags
            constexpr uint32_t CRN_SHIFT     = 16;
            constexpr uint32_t CRN_MASK      = 0b1111 << CRN_SHIFT;
            constexpr uint32_t OPCODE2_SHIFT = 6;
            constexpr uint32_t OPCODE2_MASK  = 0b111 << OPCODE2_SHIFT;
            constexpr uint32_t RT_FLAG       = 0b1 << 4;      // Register transfer when set, data processing when clear
            constexpr uint32_t CRM_MASK      = 0b1111;
        }
    }
}

// Data processing
namespace DP {
    // Common flags
    constexpr uint32_t I_FLAG       = 0b1 << 25;       // Distinguishes between immediate and register operands
    constexpr uint32_t OPCODE_SHIFT = 21;
    constexpr uint32_t OPCODE_MASK  = 0b1111 << OPCODE_SHIFT;
    constexpr uint32_t S_FLAG       = 0b1 << 20;
    constexpr uint32_t RN_SHIFT     = 16;
    constexpr uint32_t RN_MASK      = 0b1111 << RN_SHIFT;
    constexpr uint32_t RD_SHIFT     = 12;
    constexpr uint32_t RD_MASK      = 0b1111 << RD_SHIFT;
    // TODO: Verify if is necessary shifter mask to get shifter fields
    constexpr uint32_t SHIFTER_MASK = 0xFFF;
    constexpr uint32_t SHFT_OP_BIT31_FLAG = 0b1 << 31;       // Shifter operand 31bit flag

    // Data processing register flags
    constexpr uint32_t RS_SHIFT     = 8;
    constexpr uint32_t RS_MASK      = 0b1111 << RS_SHIFT;

    constexpr uint32_t SHIFT_AMOUNT_SHIFT = 7;
    constexpr uint32_t SHIFT_AMOUNT_MASK  = 0b11111 << SHIFT_AMOUNT_SHIFT;
    constexpr uint32_t BIT7_MASK = 0b1 << 7;
    constexpr uint32_t REG_MASK  = 0b1 << 4;

    constexpr uint32_t SHIFT_SHIFT = 5;
    constexpr uint32_t SHIFT_MASK  = 0b11 << SHIFT_SHIFT;
    constexpr uint32_t RM_MASK     = 0b1111;

    // Data processing immediate shift flags

    // Data processing immediate
    constexpr uint32_t ROTATE_SHIFT = 8;
    constexpr uint32_t ROTATE_MASK  = 0b1111 << ROTATE_SHIFT;
    constexpr uint32_t IMM_MASK  = 0xFF;

    enum class Opcode : uint8_t {
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

    namespace Shift {
        constexpr uint8_t LSL = 0b00;   // Logical Shift Left
        constexpr uint8_t LSR = 0b01;   // Logical Shift Right
        constexpr uint8_t ASR = 0b10;   // Arithmetic Shift Right
        constexpr uint8_t RTR = 0b11;   // Rotate Right
    }

    struct Data {
        Opcode opcode;
        bool i;         // Is immediate
        bool s;         // Updates CPSR when set
        uint8_t rn;
        uint8_t rd;
        uint16_t shifterOperand;

        uint8_t rotateImm() const;
        uint8_t immed8() const;

        uint8_t rm() const;

        bool isShftReg() const;

        uint8_t rs() const;
        uint8_t shiftImm() const;
        uint8_t shift() const;
    };

    struct ShftOperand {
        uint32_t operand{};
        bool shftCarryOut{};
    };
}

// Branch
namespace B {
    constexpr uint32_t L_FLAG = 0b1 << 24;
    constexpr uint32_t OFFSET_MASK     = 0xFFFFFF;
    constexpr uint32_t OFFSET_MSB_MASK = 0x800000;

    struct Data {
        bool l;         // L bit 24
        uint32_t signedImmed24;
    };
}

struct Miscellaneous {
    // TODO: Check if miscellaneous instruction has usable fields on ARMv4T
};

using InstructionData = std::variant<
    DP::Data,
    B::Data,
    Miscellaneous
>;

struct DecodedOperation {
    InstructionType type;
    Instruction mnemonic;
    InstructionData instructionData;
};

struct DecodedInstruction {
    uint32_t rawInstruction;
    Instruction mnemonic;
    Opcode::Cond cond;   // 31-28 - condition
    IEnc::Class clazz;  // 27-24 - class
    InstructionData data;
    InstructionType type;   // Maybe is not needed
};

enum class ProcessorMode : uint8_t {
    USER       = 0b10000,
    FIQ        = 0b10001,
    IRQ        = 0b10010,
    SUPERVISOR = 0b10011,
    ABORT      = 0b10111,
    UNDEFINED  = 0b11011,
    SYSTEM     = 0b11111,
};

enum class InstructionSet { ARM, THUMB };

std::string exceptionDescription(Exception exception);

// CPSR / SPSR Flags
namespace PSR {
    constexpr uint32_t N_SHIFT = 31;
    constexpr uint32_t Z_SHIFT = 30;
    constexpr uint32_t C_SHIFT = 29;
    constexpr uint32_t V_SHIFT = 28;
    constexpr uint32_t N = 0b1 << N_SHIFT;    // Negative, condition code, user-writable bit
    constexpr uint32_t Z = 0b1 << Z_SHIFT;    // Zero,     condition code, user-writable bit
    constexpr uint32_t C = 0b1 << C_SHIFT;    // Carry,    condition code, user-writable bit
    constexpr uint32_t V = 0b1 << V_SHIFT;    // OVerflow, condition code, user-writable bit
    constexpr uint32_t NZCV = N | Z | C | V;
    constexpr uint32_t NZC = N | Z | C;
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
    constexpr uint32_t MODE_MASK = 0b11111;

    constexpr uint32_t RESERVED_MASK = 0b11111111111111111111100000000;
}

namespace Bit {
    constexpr uint32_t WORD_MSB = 31;
    constexpr uint32_t WORD_MSB_MASK = 0b1 << WORD_MSB;     // Arm Word (32-bit) MSB bit
}


/* Thrirty-one General-purpose registers */
struct GPR {
    std::array<uint32_t, 16> r{};
    std::array<uint32_t, 2>  svc{};
    std::array<uint32_t, 2>  abt{};
    std::array<uint32_t, 2>  und{};
    std::array<uint32_t, 2>  irq{};
    std::array<uint32_t, 7>  fiq{};

    static constexpr uint8_t R0  = 0;
    static constexpr uint8_t R1  = 1;
    static constexpr uint8_t R2  = 2;
    static constexpr uint8_t R3  = 3;
    static constexpr uint8_t R4  = 4;
    static constexpr uint8_t R5  = 5;
    static constexpr uint8_t R6  = 6;
    static constexpr uint8_t R7  = 7;
    static constexpr uint8_t R8  = 8;
    static constexpr uint8_t R9  = 9;
    static constexpr uint8_t R10 = 10;
    static constexpr uint8_t R11 = 11;
    static constexpr uint8_t R12 = 12;
    static constexpr uint8_t R13 = 13;
    static constexpr uint8_t R14 = 14;
    static constexpr uint8_t R15 = 15;

    static constexpr uint8_t SP = R13;
    static constexpr uint8_t LR = R14;
    static constexpr uint8_t PC = R15;
};

/* Six status register */
struct SR {
    uint32_t cpsr{};  // Current Program Status Register
    uint32_t spsr_svc{};  // Saved Program Status Register
    uint32_t spsr_abt{};
    uint32_t spsr_und{};
    uint32_t spsr_irq{};
    uint32_t spsr_fiq{};
};

// ARMv4T ISA Implementation
class Arm7tdmi {
public:
    uint32_t instructionStep() const;
    uint32_t r(uint8_t index) const;
    void r(uint8_t index, uint32_t value);
    uint32_t fetchPC() const;
    uint32_t cpsr() const;
    void cpsr(uint32_t value);
    uint32_t spsr() const;
    void spsr(uint32_t value);
    void copySpsr2Cpsr();
    void clearPcWritten();
    bool pcWritten() const;

    void mode(ProcessorMode mode);
    ProcessorMode mode(ProcessorMode mode) const;

    GPR& getRegisters();
    SR& getStatusRegisters();

private:
    GPR registers{};
    SR statusRegisters{};
    Endianness endianness = Endianness::LITTLE;
    ProcessorMode processorMode = ProcessorMode::USER;
    InstructionSet instructionSet = InstructionSet::ARM;
    bool _pcWritten = false;
};

void reset(Arm7tdmi& cpu);

void undefinedInstruction(Arm7tdmi& cpu);

void softwareInterrupt(Arm7tdmi& cpu);

void prefetchAbort(Arm7tdmi& cpu);

void dataAbort(Arm7tdmi cpu);

void irq(Arm7tdmi& cpu);

void fiq(Arm7tdmi& cpu);

DecodedInstruction decodeInstruction(uint32_t rawInstruction);

void executeProgram(Arm7tdmi& cpu, const std::vector<uint8_t> &program);

std::string showRegisterState(Arm7tdmi& cpu);
