//
// Created by ifreturn on 25-2-2.
//
#include <riscv.h>
#include <gold.h>
#include <elfcpp.h>
#include <target.h>
#include <target-select.h>
#include <target-reloc.h>
namespace
{
    using namespace gold;
    /// Target for riscv GC extend, which means it has ISA of "IMAFDC_Zicsr_Zifencei"
    /// ABI for big-endian is NOT included in the specification currently, so it is not supported for now,
    /// when the specification is updated, it will be easy to add support for big-endian
    template<int size,bool big_endian>
    class Target_riscv:public Sized_target<size,big_endian>
    {
    public:
        /// use default
        Target_riscv():Sized_target<size,big_endian>(&info_){};
        /// to specify the dynamic linker, for abi compability
        explicit Target_riscv(const char* dynamic_linker):
        Sized_target<size,big_endian>([this, dynamic_linker]()->Target::Target_info*
        {
            const auto info = new Target::Target_info(info_);
            info->dynamic_linker = dynamic_linker;
            return info;
        })
        {  }
        ~Target_riscv() override =default;
    protected:
        const Target::Target_info info_{
            size,                 // size
            big_endian,           // is_big_endian
            elfcpp::EM_RISCV,     // machine_code
            false,                // has_make_symbol
            false,                // has_resolve
            true,                // has_code_fill
            false,                // is_default_stack_executable
            false,                // can_icf_inline_merge_sections
            '\0',                 // wrap_char
            "/lib/ld-linux-riscv64-lp64d.so.1", // dynamic_linker
            0x10000,              // default_text_segment_address
            0x1000,               // abi_pagesize (4KB)
            0x1000,               // common_pagesize (4KB)
            false,                // isolate_execinstr
            0,                    // rosegment_gap
            elfcpp::SHN_UNDEF,    // small_common_shndx
            elfcpp::SHN_UNDEF,    // large_common_shndx
            0,                    // small_common_section_flags
            0,                    // large_common_section_flags
            ".riscv.attributes",  // attributes_section
            "riscv",              // attributes_vendor
            "_start",             // entry_symbol_name
            32,                   // hash_entry_size
            elfcpp::SHT_PROGBITS  // unwind_section_type
        };
    public:
        void gc_process_relocs(Symbol_table* symtab, Layout* layout, Sized_relobj_file<size, big_endian>* object,
            unsigned data_shndx, unsigned sh_type, const unsigned char* prelocs, size_t reloc_count,
            Output_section* output_section, bool needs_special_offset_handling, size_t local_symbol_count,
            const unsigned char* plocal_symbols) override;
        // Scan the relocs for a section, and record any information
        // required for the symbol.  SYMTAB is the symbol table.  OBJECT is
        // the object in which the section appears.  DATA_SHNDX is the
        // section index that these relocs apply to.  SH_TYPE is the type of
        // the relocation section, SHT_REL or SHT_RELA.  PRELOCS points to
        // the relocation data.  RELOC_COUNT is the number of relocs.
        // LOCAL_SYMBOL_COUNT is the number of local symbols.
        // OUTPUT_SECTION is the output section.
        // NEEDS_SPECIAL_OFFSET_HANDLING is true if offsets to the output
        // sections are not mapped as usual.  PLOCAL_SYMBOLS points to the
        // local symbol data from OBJECT.  GLOBAL_SYMBOLS is the array of
        // pointers to the global symbol table from OBJECT.
        void scan_relocs(Symbol_table* symtab, Layout* layout, Sized_relobj_file<size, big_endian>* object,
            unsigned data_shndx, unsigned sh_type, const unsigned char* prelocs, size_t reloc_count,
            Output_section* output_section, bool needs_special_offset_handling, size_t local_symbol_count,
            const unsigned char* plocal_symbols) override;

        void scan_relocatable_relocs(Symbol_table* symtab, Layout* layout, Sized_relobj_file<size, big_endian>* object,
            unsigned data_shndx, unsigned sh_type, const unsigned char* prelocs, size_t reloc_count,
            Output_section* output_section, bool needs_special_offset_handling, size_t local_symbol_count,
            const unsigned char* plocal_symbols, Relocatable_relocs*) override;

        void emit_relocs_scan(Symbol_table* symtab, Layout* layout, Sized_relobj_file<size, big_endian>* object,
            unsigned data_shndx, unsigned sh_type, const unsigned char* prelocs, size_t reloc_count,
            Output_section* output_section, bool needs_special_offset_handling, size_t local_symbol_count,
            const unsigned char* plocal_syms, Relocatable_relocs* rr) override;
    protected:
        // The template parameter Scan must be a class type which provides two
        // functions: local() and global(). Those functions implement the
        // machine specific part of scanning.
        class Scan
        {
        public:
            void local();
            void global();
        };
        // A class for inquiring about properties of a relocation,
        // used while scanning relocs during a relocatable link and
        // garbage collection.
        class Classify_reloc
        {
        public:
            Default_classify_reloc<>
        };
        // Return a string to use to fill out a code section.  This is
        // basically one or more NOPS which must fill out the specified
        // length in bytes.
        std::string do_code_fill(section_size_type length) const override;
    };

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
            int machine, int osabi , int abiversion) override
    {
        const int ehdr_size = sizeof(elfcpp::internal::Ehdr_data<size>);
        const unsigned char* buffer = input_file->file().get_view(offset, 0, ehdr_size, false, false);
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
template<int size,bool big_endian>
void Target_riscv<size,big_endian>::scan_relocs(Symbol_table* symtab, Layout* layout, Sized_relobj_file<size, big_endian>* object,
            unsigned data_shndx, unsigned sh_type, const unsigned char* prelocs, size_t reloc_count,
            Output_section* output_section, bool needs_special_offset_handling, size_t local_symbol_count,
            const unsigned char* plocal_symbols) override
{

    unsigned int reloc_size;
    if (sh_type == elfcpp::SHT_REL)
        reloc_size = elfcpp::Elf_sizes<size>::rel_size;
    else
    {
        gold_assert(sh_type == elfcpp::SHT_RELA);
        reloc_size = elfcpp::Elf_sizes<size>::rela_size;
    }
    for (auto i = 0; i < reloc_count; i++)
    {
        const unsigned char* p = prelocs + i * reloc_size;
        elfcpp::Rel<32, big_endian> reloc{p};
        unsigned int r_type = elfcpp::elf_r_type<size>(reloc.get_r_info());

        switch (r_type)
        {
        case elfcpp::R_RISCV_32:
            // 处理32位绝对地址重定位
                break;
        case elfcpp::R_RISCV_BRANCH:
            // 处理分支指令重定位
                break;
            // 其他RISC-V重定位类型...
        default:
            gold_error(_("unsupported RISC-V relocation type %u"), r_type);
        }

    }
};
template <int size, bool big_endian>
std::string Target_riscv<size,big_endian>::do_code_fill(section_size_type length) const override
{
    if (length % 4 != 0)
        gold_error(_("Riscv code fill length must be a multiple of 4"));
    std::string fill;
    const uint32_t nop = 0x00000013; // addi x0, x0, 0
    for (section_size_type i = 0; i < length; i += 4)
        fill.append(reinterpret_cast<const char*>(&nop), 4);
    return fill;
}