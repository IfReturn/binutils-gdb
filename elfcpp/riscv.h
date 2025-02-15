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
/*Name                         Type	  Field	    Calc      Desc*/  
enum {
    R_RISCV_NONE = 0,	      
    R_RISCV_32 = 1,	      //BOTH	 word32	    S+A	    32-bit relocation
    R_RISCV_64 = 2,	      //BOTH	word64	    S+A	    64-bit relocation
    R_RISCV_RELATIVE = 3,     //Dynamic	wordclass   B+A	    Adjust a link address (A) 
							    // to its load address(B + A)
    R_RISCV_COPY = 4,	      //Dynamic		    NONE    exec only,not allowed in shared lib	
    R_RISCV_JUMP_SLOT = 5,    //Dynamic	wordclass   S	    Indicates the symbol associated with
							    //a PLT entry
    R_RISCV_TLS_DTPMOD32 = 6, //Dynamic	word32	    TLSMODULE
    R_RISCV_TLS_DTPMOD64 = 7, //Dynamic	word64	    TLSMODULE
    R_RISCV_TLS_DTPREL32 = 8, //Dynamic	word32	    S+A-TLS_DTV_OFFSET
    R_RISCV_TLS_DTPREL64 = 9, //Dynamic	word64	    S+A-TLS_DTV_OFFSET
    R_RISCV_TLS_TPREL32 = 10, //Dynamic	word32	    S+A-TLS_OFFSET
    R_RISCV_TLS_TPREL64 = 11, //Dynamic	word64	    S+A-TLS_OFFSET
    //12-bit PC-relative branch offset
    R_RISCV_BRANCH = 16,      //Static	B-Type	    S+A-P   
    //20-bit PC-relative jump offset
    R_RISCV_JAL = 17,	      //Static	J-Type	    S+A-P   
    //Deprecated, please use CALL_PLT instead
    R_RISCV_CALL = 18,	      //Static	U+I-Type    S+A-P   
    //32-bit PC-relative function call,macros call, tail(PIC)
    R_RISCV_CALL_PLT = 19,    //Static	U+I-Type    S+A-P
    //High 20 bits of 32-bit PC-relative GOT access, % got_pcrel_hi(symbol)
    R_RISCV_GOT_HI20 = 20,    //Static	U-Type	    S+A-P
    //High 20 bits of 32-bit PC-relative TLS IE GOT access, macro la.tls.ie
    R_RISCV_TLS_GOT_HI20 = 21,//Static	U-Type
    //High 20 bits of 32-bit PC-relative TLS GD GOT reference, macro la.tls.gd
    R_RISCV_TLS_GD_HI20 = 22, //Static	U-Type
    //High 20 bits of 32-bit PC-relative reference, % pcrel_hi(symbol)
    R_RISCV_PCREL_HI20 = 23,  //Static	U-Type	    S+A-P
    //Low 12 bits of 32-bit PC-relative reference, % pcrel_lo(address of %pcrel_hi) the addend must be 0
    R_RISCV_PCREL_LO12_I = 24,//Static	I-Type	    S-P
    R_RISCV_PCREL_LO12_S = 25,//Static	S-Type	    S+A-P
    //High 20 bits of 32-bit absolute address, %hi(symbol)
    R_RISCV_HI20 = 26,	      //Static	U-Type	    S+A
    //Low 12 bits of 32 - bit absolute address,
    R_RISCV_LO12_I = 27,      //Static	I-Type	    S+A
    R_RISCV_LO12_S = 28,      //Static	S-Type	    S+A
    //High 20 bits of TLS LE thread pointeroffset
    R_RISCV_TPREL_HI20 = 29,  //Static	U-Type	    
    //Low 12 bits of TLS LE thread pointer offset
    R_RISCV_TPREL_LO12_I = 30,//Static	I-Type
    R_RISCV_TPREL_LO12_S = 31,//Static	S-Type
    //TLS LE thread pointer usage,
    R_RISCV_TPREL_ADD = 32,   //Static
    R_RISCV_ADD8 = 33,	      //Static	word8	    V+S+A
    R_RISCV_ADD16 = 34,	      //Static	word16	    V+S+A
    R_RISCV_ADD32 = 35,	      //Static	word32	    V+S+A
    R_RISCV_ADD64 = 36,	      //Static	word64	    V+S+A
    R_RISCV_SUB8 = 37,	      //Static	word8	    V-S-A
    R_RISCV_SUB16 = 38,	      //Static	word16	    V-S-A
    R_RISCV_SUB32 = 39,	      //Static	word32	    V-S-A
    R_RISCV_SUB64 = 40,	      //Static	word64	    V-S-A
    R_RISCV_GNU_VTINHERIT = 41,
    R_RISCV_GNU_VTENTRY = 42,
    //Alignment statement. The addend  indicates the number of bytes
    //occupied by nop instructions at the  relocation offset.The alignment
    //boundary is specified by the addend rounded up to the next power of two.
    R_RISCV_ALIGN = 43,	      //Static
    //8-bit PC-relative branch offset
    R_RISCV_RVC_BRANCH = 44,  //Static	CB-Type	    S+A-P
    //11-bit PC-relative jump offset
    R_RISCV_RVC_JUMP = 45,    //Static	CJ-Type	    S+A-P
    R_RISCV_RVC_LUI = 46,     //Static	CU-Type	    S+A
    R_RISCV_GPREL_I = 47,
    R_RISCV_GPREL_S = 48,
    R_RISCV_TPREL_I = 49,
    R_RISCV_TPREL_S = 50,
    R_RISCV_RELAX = 51,	      //Static
    R_RISCV_SUB6 = 52,	      //Static	word6	    V-S-A
    R_RISCV_SET6 = 53,	      //Static	word6	    S+A
    R_RISCV_SET8 = 54,	      //Static	word8	    S+A
    R_RISCV_SET16 = 55,	      //Static	word16	    S+A
    R_RISCV_SET32 = 56,	      //Static	word32	    S+A
    R_RISCV_32_PCREL = 57,    //Static	word32	    S+A-P
    R_RISCV_IRELATIVE = 58,   //Dynamic	wordclass   ifunc_resovler(B+A)
    R_RISCV_PLT32 = 59,
    R_RISCV_SET_ULEB128 = 60,
    R_RISCV_SUB_ULEB128 = 61,
    R_RISCV_NUM = 62
};
    /* RISC-V specific values for the st_other field.  */
enum{STO_RISCV_VARIANT_CC=0x80};/* Function uses variant calling convention */

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
