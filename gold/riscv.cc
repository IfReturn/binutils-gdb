// riscv.cc -- riscv target support for gold.
// RISC-V is a clean RISC ISA. It supports PC-relative load/store for
// position-independent code. Its 32-bit and 64-bit ISAs are almost
// identical. That is, you can think RV32 as a RV64 without 64-bit
// operations. In this file, we support both RV64 and RV32.

// RISC-V is essentially little-endian, but the big-endian version is
// available as an extension. GCC supports `-mbig-endian` to generate
// big-endian code. Even in big-endian mode, machine instructions are
// defined to be encoded in little-endian, though. Only the behavior of
// load/store instructions are different between LE RISC-V and BE RISC-V.
// Copyright (C) 2014-2025 Free Software Foundation, Inc.
// Written by Zhichao Yang <yzc2004.12@qq.com>
//https://github.com/riscv-non-isa/riscv-elf-psabi-doc/blob/master/riscv-elf.adoc
#include <copy-relocs.h>
#include <riscv.h>
#include <gold.h>
#include <elfcpp.h>
#include <layout.h>
#include <target.h>
#include <target-select.h>
#include <target-reloc.h>
#include <output.h>
namespace
{

//risc-v related addr process
inline uint64_t bit(uint64_t val, uint64_t pos) {
  return (val >> pos) & 1;
};
inline uint64_t bits(uint64_t val, uint64_t hi, uint64_t lo) {
  return (val >> lo) & ((1LL << (hi - lo + 1)) - 1);
}
static void write_itype(unsigned char *loc, uint32_t val) {
  *(uint32_t *)loc &= 0b000000'00000'11111'111'11111'1111111;
  *(uint32_t *)loc |= bits(val, 11, 0) << 20;
}

static void write_stype(unsigned char *loc, uint32_t val) {
  *(uint32_t *)loc &= 0b000000'11111'11111'111'00000'1111111;
  *(uint32_t *)loc |= bits(val, 11, 5) << 25 | bits(val, 4, 0) << 7;
}

static void write_btype(unsigned char *loc, uint32_t val) {
  *(uint32_t *)loc &= 0b000000'11111'11111'111'00000'1111111;
  *(uint32_t *)loc |= bit(val, 12) << 31   | bits(val, 10, 5) << 25 |
                  bits(val, 4, 1) << 8 | bit(val, 11) << 7;
}

static void write_utype(unsigned char *loc, uint32_t val) {
  *(uint32_t *)loc &= 0b000000'00000'00000'000'11111'1111111;

  // U-type instructions are used in combination with I-type
  // instructions. U-type insn sets an immediate to the upper 20-bits
  // of a register. I-type insn sign-extends a 12-bits immediate and
  // adds it to a register value to construct a complete value. 0x800
  // is added here to compensate for the sign-extension.
  *(uint32_t *)loc |= (val + 0x800) & 0xffff'f000;
}

static void write_jtype(unsigned char *loc, uint32_t val) {
  *(uint32_t *)loc &= 0b000000'00000'00000'000'11111'1111111;
  *(uint32_t *)loc |= bit(val, 20) << 31 | bits(val, 10, 1)  << 21 |
                  bit(val, 11) << 20 | bits(val, 19, 12) << 12;
}

static void write_citype(unsigned char *loc, uint32_t val) {
  *(uint16_t *)loc &= 0b111'0'11111'00000'11;
  *(uint16_t *)loc |= bit(val, 5) << 12 | bits(val, 4, 0) << 2;
}

static void write_cbtype(unsigned char *loc, uint32_t val) {
  *(uint16_t *)loc &= 0b111'000'111'00000'11;
  *(uint16_t *)loc |= bit(val, 8) << 12 | bit(val, 4) << 11 | bit(val, 3) << 10 |
                  bit(val, 7) << 6  | bit(val, 6) << 5  | bit(val, 2) << 4  |
                  bit(val, 1) << 3  | bit(val, 5) << 2;
}

static void write_cjtype(unsigned char *loc, uint32_t val) {
  *(uint16_t *)loc &= 0b111'00000000000'11;
  *(uint16_t *)loc |= bit(val, 11) << 12 | bit(val, 4)  << 11 | bit(val, 9) << 10 |
                  bit(val, 8)  << 9  | bit(val, 10) << 8  | bit(val, 6) << 7  |
                  bit(val, 7)  << 6  | bit(val, 3)  << 5  | bit(val, 2) << 4  |
                  bit(val, 1)  << 3  | bit(val, 5)  << 2;
}

static void set_rs1(unsigned char *loc, uint32_t rs1) {
  gold_assert(rs1 < 32);
  *(uint32_t *)loc &= 0b111111'11111'00000'111'11111'1111111;
  *(uint32_t *)loc |= rs1 << 15;
}

static uint32_t get_rd(unsigned char *loc) {
  return bits(*(uint32_t *)loc, 11, 7);
};
static constexpr uint32_t plt_entry_64[] = {
  0x0000'0e17, // auipc   t3, %pcrel_hi(function@.got.plt)
  0x000e'3e03, // ld      t3, %pcrel_lo(1b)(t3)
  0x000e'0367, // jalr    t1, t3
  0x0010'0073, // ebreak
};

static constexpr uint32_t plt_entry_32[] = {
  0x0000'0e17, // auipc   t3, %pcrel_hi(function@.got.plt)
  0x000e'2e03, // lw      t3, %pcrel_lo(1b)(t3)
  0x000e'0367, // jalr    t1, t3
  0x0010'0073, // ebreak
};
using namespace gold;
template<int size,bool big_endian>
class Output_data_plt_riscv : public Output_section_data
{
public:
    typedef Output_data_reloc<elfcpp::SHT_RELA,true,size,big_endian> Reloc_section;
    typedef typename elfcpp::Elf_types<size>::Elf_Addr Address;
    Output_data_plt_riscv(Layout* layout,
                            uint64_t addralign,
                            Output_data_got<size,big_endian>* got,
                            Output_data_space* got_plt,
                            Output_data_space* got_irelative)
    :Output_section_data(addralign),tlsdesc_rel_(NULL), irelative_rel_(NULL),
      got_(got), got_plt_(got_plt), got_irelative_(got_irelative),
      count_(0), irelative_count_(0), tlsdesc_got_offset_(-1U)
    {
        this->init(layout);
    }
    void init(Layout* layout)
    {
        rel_ = new Reloc_section(false);
        layout->add_output_section_data("rela.plt",elfcpp::SHT_RELA,
            elfcpp::SHF_ALLOC,rel_,ORDER_DYNAMIC_PLT_RELOCS,false);
    };
    void add_entry(Symbol* symtab,Layout* layout,Symbol* gsym)
    {
        gold_assert(!gsym->has_plt_offset());

        unsigned int* pcount;
        unsigned int plt_reserved;
        Output_data_space* got;

        if (gsym->type() == elfcpp::STT_GNU_IFUNC)
        {
            //todo
            return;
        }
        else
        {
            pcount = &count_;
            plt_reserved = PLT_RESERVED_SPACE;
            got = got_plt_;
        }
        gsym->set_plt_offset(plt_reserved + *pcount * 16);
        ++*pcount;
        section_offset_type got_offset = got->current_data_size();
        //32位4字节，64位8字节
        got->set_current_data_size(got_offset+size/8);
        add_relocation(symtab,layout,gsym,got_offset);
    };





    uint add_local_ifunc_entry(Symbol* symtab,
        Layout* layout,Sized_relobj_file<size,big_endian>* relobj,uint local_sym_index);

    void add_relocation(Symbol_table*symtab,Layout*layout,
                        Symbol*gsym,uint got_offset)
    {
        if (gsym->type() == elfcpp::STT_GNU_IFUNC)
        {
            //todo
            return;
        }
        else
        {
            gsym->set_needs_dynsym_entry();
            this->rel_->add_global(gsym, elfcpp::R_RISCV_JUMP_SLOT, this->got_plt_,
                       got_offset, 0);
        }
    };

    ///@param pov 输出文件的视图
    void write_plt_header(unsigned char* pov)
    {
        constexpr uint32_t insn_64[] = {
            0x0000'0397, // auipc  t2, %pcrel_hi(.got.plt)
            0x41c3'0333, // sub    t1, t1, t3               # .plt entry + hdr + 12
            0x0003'be03, // ld     t3, %pcrel_lo(1b)(t2)    # _dl_runtime_resolve
            0xfd43'0313, // addi   t1, t1, -44              # .plt entry
            0x0003'8293, // addi   t0, t2, %pcrel_lo(1b)    # &.got.plt
            0x0013'5313, // srli   t1, t1, 1                # .plt entry offset
            0x0082'b283, // ld     t0, 8(t0)                # link map
            0x000e'0067, // jr     t3
        };

        constexpr uint32_t insn_32[] = {
            0x0000'0397, // auipc  t2, %pcrel_hi(.got.plt)
            0x41c3'0333, // sub    t1, t1, t3               # .plt entry + hdr + 12
            0x0003'ae03, // lw     t3, %pcrel_lo(1b)(t2)    # _dl_runtime_resolve
            0xfd43'0313, // addi   t1, t1, -44              # .plt entry
            0x0003'8293, // addi   t0, t2, %pcrel_lo(1b)    # &.got.plt
            0x0023'5313, // srli   t1, t1, 2                # .plt entry offset
            0x0042'a283, // lw     t0, 4(t0)                # link map
            0x000e'0067, // jr     t3
        };

        memcpy(pov, size == 64 ? insn_64 : insn_32, sizeof(insn_64));
        Address got_plt_addr = got_plt_->address();
        Address plt_addr = this->address();
        write_utype(pov,got_plt_addr-plt_addr);
        write_itype(pov+8,got_plt_addr-plt_addr);
        write_itype(pov+16,got_plt_addr-plt_addr);
    };

private:
    // The reloc section.
    Reloc_section* rel_;

    // The TLSDESC relocs, if necessary.  These must follow the regular
    // PLT relocs.
    Reloc_section* tlsdesc_rel_;

    // The IRELATIVE relocs, if necessary.  These must follow the
    // regular PLT relocations.
    Reloc_section* irelative_rel_;

    // The .got section.
    Output_data_got<size, big_endian>* got_;

    // The .got.plt section.
    Output_data_space* got_plt_;

    // The part of the .got.plt section used for IRELATIVE relocs.
    Output_data_space* got_irelative_;

    // The number of PLT entries.
    unsigned int count_;

    // Number of PLT entries with IRELATIVE relocs.  These
    // follow the regular PLT entries.
    unsigned int irelative_count_;

    // GOT offset of the reserved TLSDESC_GOT entry for the lazy trampoline.
    // Communicated to the loader via DT_TLSDESC_GOT. The magic value -1
    // indicates an offset is not allocated.
    unsigned int tlsdesc_got_offset_;

    constexpr int PLT_RESERVED_SPACE = 32;

    constexpr int PLT_ENTRY_SIZE = 16;

    constexpr int GOT_ENTRY_SIZE = 8;
};


template<int size,bool big_endian>
    class Target_riscv : public Sized_target<size,big_endian>
{
public:
    Target_riscv(std::string_view dynamic_linker)
        :Sized_target<size,big_endian>([dynamic_linker]()
            {const Target::Target_info info={
            size,big_endian,elfcpp::EM_RISCV,false,false
            ,false,false,false, false
            ,dynamic_linker.data(),0x10000,0x1000,0x1000
            ,false,0,elfcpp::SHN_UNDEF,elfcpp::SHN_UNDEF
            ,0,0,".riscv.attributes","riscv"
            ,"_start",32,elfcpp::SHT_PROGBITS};
            return info;})
    {  }

    // due to the relevance of HI20 and LO12, it seems better to override the whole scan_relocs instead of
    // just do the scan class.
    void
    scan_relocs(Symbol_table* symtab,
          Layout* layout,
          Sized_relobj_file<size, false>* object,
          unsigned int data_shndx,
          unsigned int sh_type,
          const unsigned char* prelocs,
          size_t reloc_count,
          Output_section* output_section,
          bool needs_special_offset_handling,
          size_t local_symbol_count,
          const unsigned char* plocal_symbols);

    typedef Output_data_reloc<elfcpp::SHT_RELA,true,size,big_endian> Reloc_section;
    Reloc_section* rela_dyn_section(Layout* layout)
    {
        if (rela_dyn_ == nullptr)
        {
            gold_assert(layout!=nullptr);
            rela_dyn_ = new Reloc_section(parameters->options().combreloc());
            layout->add_output_section_data(".rela.dyn",elfcpp::SHT_RELA
                ,elfcpp::SHF_ALLOC,rela_dyn_,ORDER_DYNAMIC_RELOCS,false);
        }
        return this->rela_dyn_;
    }
private:Reloc_section* rela_dyn_{nullptr};
public:Reloc_section* rela_plt_section(Layout* layout)
    {
        if (rela_plt_ == nullptr)
        {
            gold_assert(layout!=nullptr);
            rela_plt_ = new Reloc_section(parameters->options().combreloc());
            layout->add_output_section_data(".rela.plt",elfcpp::SHT_RELA
                ,elfcpp::SHF_ALLOC,rela_plt_,ORDER_DYNAMIC_PLT_RELOCS,false);
        }
        return rela_plt_;
    }
private:Reloc_section* rela_plt_{nullptr};
public:
    //.GOT表
    Output_data_got<size,big_endian>* got_section(Symbol_table* symtab,Layout* layout)
    {
        if (got_ == nullptr)
        {
            gold_assert(layout!=nullptr&&symtab!=nullptr);
            got_=new Output_data_got<size, big_endian>();
            bool is_got_plt_relro = parameters->options().now();
            Output_section_order got_order=(is_got_plt_relro
                    ? ORDER_RELRO
                    : ORDER_RELRO_LAST);
            Output_section_order got_plt_order=(is_got_plt_relro
                    ? ORDER_RELRO
                    : ORDER_NON_RELRO_FIRST);
            this->got_=new Output_data_got<size, big_endian>();
            layout->add_output_section_data(".got",elfcpp::SHT_PROGBITS,
                                (elfcpp::SHF_ALLOC|elfcpp::SHF_WRITE),
                                got_,got_order,true);
            this ->got_plt_ = new Output_data_space(size / 8, "** GOT PLT");
            layout->add_output_section_data(".got.plt", elfcpp::SHT_PROGBITS,
                      (elfcpp::SHF_ALLOC
                       | elfcpp::SHF_WRITE),
                      this->got_plt_, got_plt_order,
                      is_got_plt_relro);
            this->got_plt_->set_current_data_size(3 * (size / 8));
            this->got_irelative_ = new Output_data_space(size / 8,
                           "** GOT IRELATIVE PLT");
            layout->add_output_section_data(".got.plt", elfcpp::SHT_PROGBITS,
                            (elfcpp::SHF_ALLOC
                             | elfcpp::SHF_WRITE),
                            this->got_irelative_,
                            got_plt_order,
                            is_got_plt_relro);
            this->got_tlsdesc_ = new Output_data_got<size, big_endian>();
            layout->add_output_section_data(".got.plt", elfcpp::SHT_PROGBITS,
                            (elfcpp::SHF_ALLOC
                             | elfcpp::SHF_WRITE),
                            this->got_tlsdesc_,
                            got_plt_order,
                            is_got_plt_relro);
            if (!is_got_plt_relro)
            {
                layout->increase_relro(3 * (size / 8));
            }
        }

        return got_;
    }
    Output_data_space * got_plt_section()
    {
        if (got_plt_ == nullptr)
            got_section();
        gold_assert(got_plt_!=nullptr);
        return got_plt_;
    }
    Output_data_space * got_irelative_section()
    {
        if (got_irelative_==nullptr)
            got_section();
        gold_assert(got_irelative_!=nullptr);
        return got_irelative_;
    }
    Output_data_got<size,big_endian>* got_tlsdesc_section()
    {
        if (got_tlsdesc_==nullptr)
            got_section();
        gold_assert(got_tlsdesc_!=nullptr);
        return got_tlsdesc_;
    }

    Output_data_plt_riscv<size,big_endian>* plt_section(Layout* layout)
    {
        if (plt_ == nullptr)
        {
            gold_assert(layout!=nullptr);
            plt_ = new Output_data_plt_riscv<size,big_endian>(layout,
                16,got_,got_plt_,got_irelative_);
            layout->add_output_section_data(".plt",elfcpp::SHT_PROGBITS,
                (elfcpp::SHF_ALLOC|elfcpp::SHF_EXECINSTR),plt_,ORDER_PLT,true);
        }
        return plt_;
    }
private:
    Output_data_got<size,big_endian>* got_{nullptr};
    Output_data_space* got_plt_{nullptr};
    Output_data_space* got_irelative_{nullptr};
    Output_data_got<size,big_endian>* got_tlsdesc_{nullptr};
    // tls表
    Output_data_plt_riscv<size,big_endian>* plt_{nullptr};
public:
    class Scan
    {
    void
    local(
        Symbol_table* symtab,
        Layout* layout,
        Target_riscv<size, big_endian>* target,
        Sized_relobj_file<size, big_endian>* object,
        unsigned int data_shndx,
        Output_section* output_section,
        const elfcpp::Rela<size, big_endian>& rela,
        unsigned int r_type,
        const elfcpp::Sym<size, big_endian>& lsym,
        bool is_discarded)
    {
        if (is_discarded)
            return;

        bool is_ifunc = lsym.get_st_type() == elfcpp::STT_GNU_IFUNC;

        unsigned int r_sym = elfcpp::elf_r_sym<size>(rela.get_r_info());
        switch (r_type)
        {
        using namespace elfcpp;
        case elfcpp::R_RISCV_32:
        case elfcpp::R_RISCV_64:
            if (parameters->options().output_is_position_independent())
            {
                Reloc_section* rela_dyn = target->rela_dyn_section(layout);
                rela_dyn->add_local_relative(
                    object,
                    r_sym,
                    elfcpp::R_RISCV_RELATIVE,
                    output_section,
                    data_shndx,
                    rela.get_r_offset(),
                    rela.get_r_addend(),
                    is_ifunc);
            }
            break;
            // things need plt
        case R_RISCV_CALL:
        case R_RISCV_CALL_PLT:
        case R_RISCV_PLT32:
            target->plt_section();

        case R_RISCV_GOT_HI20:
        case R_RISCV_TLS_GOT_HI20:
        case R_RISCV_TLS_GD_HI20:
            target->got_section();

        }
    }
    void
    global(Symbol_table* symtab,
        Layout* layout,
        Target_riscv<size, big_endian>* target,
        Sized_relobj_file<size, big_endian> * object,
        unsigned int data_shndx,
        Output_section* output_section,
        const elfcpp::Rela<size, big_endian>& rela,
        unsigned int r_type,
        Symbol* gsym)
    {
        typedef Output_data_reloc<elfcpp::SHT_RELA, true, size, big_endian>
        Reloc_section;
        switch (r_type)
        {
            using namespace elfcpp;
            case R_RISCV_NONE:
                break;
            case R_RISCV_32:
            case R_RISCV_64:
                if (gsym->needs_plt_entry())
                {
                    target->plt_section();
                    if (gsym -> is_from_dynobj()&& !parameters->options().shared())
                    {
                        gsym->set_needs_dynsym_value();
                    }
                }
                if (gsym->needs_dynamic_reloc(Symbol::ABSOLUTE_REF))
                {

                }

        }
    }
    };

};// end class Target_riscv

template <int size,bool big_endian>
    void Target_riscv<size,big_endian>::scan_relocs(Symbol_table* symtab,
          Layout* layout,
          Sized_relobj_file<size, false>* object,
          unsigned int data_shndx,
          unsigned int sh_type,
          const unsigned char* prelocs,
          size_t reloc_count,
          Output_section* output_section,
          bool needs_special_offset_handling,
          size_t local_symbol_count,
          const unsigned char* plocal_symbols)
{
    typedef typename Default_classify_reloc<elfcpp::SHT_RELA,size,big_endian> Classify_reloc;
    gold::scan_relocs<size,big_endian,Target_riscv,Scan,Classify_reloc>(
        symtab,layout,this,object,data_shndx,prelocs,reloc_count,output_section,
        needs_special_offset_handling,local_symbol_count,plocal_symbols);
}

template<int size,bool big_endian>
class Target_selector_riscv : public Target_selector
{
public:
    Target_selector_riscv(const char* bfd_name,const char* emulation,const char* dynamic_linker,const char* extensions):Target_selector(
        elfcpp::EM_RISCV,size,big_endian
        ,bfd_name,emulation),dynamic_linker_(dynamic_linker){};
protected:
    const char* dynamic_linker_;
    virtual
    Target* do_recognize(Input_file* input_file, off_t offset,
            int machine, int osabi , int abiversion)
    {
        const int ehdr_size = sizeof(elfcpp::internal::Ehdr_data<size>);
        const unsigned char* buffer = input_file->file().get_view(offset, 0, ehdr_size, false, true);
        elfcpp::Ehdr<size,big_endian> ehdr{buffer};
        auto rv_flag = ehdr.get_e_flags();

        if ((rv_flag&elfcpp::EF_RISCV_FLOAT_ABI_QUAD) == elfcpp::EF_RISCV_FLOAT_ABI_QUAD)
            return NULL; /// quad is not supported in GC extension
        return do_instantiate_target();
    };
    Target* do_instantiate_target() override
    {
        return new Target_riscv<size,big_endian>(dynamic_linker_);
    };
};

Target_selector_riscv<32,false> riscv32GC_selector{"",""
    ,"","GC"};
Target_selector_riscv<64,false> riscv64GC_selector{"",""
    ,"","GC"};



}