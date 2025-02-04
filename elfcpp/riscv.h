//
// Created by ifreturn on 25-2-4.
//

#ifndef ELFCPP_RISCV_H
#define ELFCPP_RISCV_H

namespace elfcpp{
/* RISC-V ELF Flags */
enum {
    EF_RISCV_RVC = 0x0001,
    EF_RISCV_FLOAT_ABI = 0x0006,
    EF_RISCV_FLOAT_ABI_SOFT = 0x0000,
    EF_RISCV_FLOAT_ABI_SINGLE = 0x0002,
    EF_RISCV_FLOAT_ABI_DOUBLE = 0x0004,
    EF_RISCV_FLOAT_ABI_QUAD = 0x0006,
    EF_RISCV_RVE = 0x0008,
    EF_RISCV_TSO = 0x0010
};
/* RISC-V relocations.  */
enum {
    R_RISCV_NONE = 0,
    R_RISCV_32 = 1,
    R_RISCV_64 = 2,
    R_RISCV_RELATIVE = 3,
    R_RISCV_COPY = 4,
    R_RISCV_JUMP_SLOT = 5,
    R_RISCV_TLS_DTPMOD32 = 6,
    R_RISCV_TLS_DTPMOD64 = 7,
    R_RISCV_TLS_DTPREL32 = 8,
    R_RISCV_TLS_DTPREL64 = 9,
    R_RISCV_TLS_TPREL32 = 10,
    R_RISCV_TLS_TPREL64 = 11,
    R_RISCV_BRANCH = 16,
    R_RISCV_JAL = 17,
    R_RISCV_CALL = 18,
    R_RISCV_CALL_PLT = 19,
    R_RISCV_GOT_HI20 = 20,
    R_RISCV_TLS_GOT_HI20 = 21,
    R_RISCV_TLS_GD_HI20 = 22,
    R_RISCV_PCREL_HI20 = 23,
    R_RISCV_PCREL_LO12_I = 24,
    R_RISCV_PCREL_LO12_S = 25,
    R_RISCV_HI20 = 26,
    R_RISCV_LO12_I = 27,
    R_RISCV_LO12_S = 28,
    R_RISCV_TPREL_HI20 = 29,
    R_RISCV_TPREL_LO12_I = 30,
    R_RISCV_TPREL_LO12_S = 31,
    R_RISCV_TPREL_ADD = 32,
    R_RISCV_ADD8 = 33,
    R_RISCV_ADD16 = 34,
    R_RISCV_ADD32 = 35,
    R_RISCV_ADD64 = 36,
    R_RISCV_SUB8 = 37,
    R_RISCV_SUB16 = 38,
    R_RISCV_SUB32 = 39,
    R_RISCV_SUB64 = 40,
    R_RISCV_GNU_VTINHERIT = 41,
    R_RISCV_GNU_VTENTRY = 42,
    R_RISCV_ALIGN = 43,
    R_RISCV_RVC_BRANCH = 44,
    R_RISCV_RVC_JUMP = 45,
    R_RISCV_RVC_LUI = 46,
    R_RISCV_GPREL_I = 47,
    R_RISCV_GPREL_S = 48,
    R_RISCV_TPREL_I = 49,
    R_RISCV_TPREL_S = 50,
    R_RISCV_RELAX = 51,
    R_RISCV_SUB6 = 52,
    R_RISCV_SET6 = 53,
    R_RISCV_SET8 = 54,
    R_RISCV_SET16 = 55,
    R_RISCV_SET32 = 56,
    R_RISCV_32_PCREL = 57,
    R_RISCV_IRELATIVE = 58,
    R_RISCV_PLT32 = 59,
    R_RISCV_SET_ULEB128 = 60,
    R_RISCV_SUB_ULEB128 = 61,
    R_RISCV_NUM = 62
};
    /* RISC-V specific values for the st_other field.  */
enum{STO_RISCV_VARIANT_CC=0x80}	/* Function uses variant calling convention */

//    /* RISC-V specific values for the sh_type field.  */
//#define SHT_RISCV_ATTRIBUTES	(SHT_LOPROC + 3)
//
//    /* RISC-V specific values for the p_type field.  */
//#define PT_RISCV_ATTRIBUTES	(PT_LOPROC + 3)
//
//    /* RISC-V specific values for the d_tag field.  */
//#define DT_RISCV_VARIANT_CC	(DT_LOPROC + 1)
}


#endif //ELFCPP_RISCV_H
