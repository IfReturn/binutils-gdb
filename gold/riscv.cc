//
// Created by ifreturn on 25-2-2.
//
#include <riscv.h>
#include <gold.h>
#include <elfcpp.h>
#include <target.h>
#include <target-select.h>
#include <target-reloc.h>
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