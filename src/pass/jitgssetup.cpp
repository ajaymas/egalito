#include <cassert>
#include "jitgssetup.h"
#include "conductor/conductor.h"
#include "operation/find2.h"
#include "log/log.h"

void JitGSSetup::visit(Program *program) {
    makeResolverGSEntries(program->getEgalito());
    makeSupportGSEntries(program);
}

void JitGSSetup::makeResolverGSEntries(Module *egalito) {
    const auto resolvers = {
        "egalito_jit_gs_fixup",
        "_ZN7GSTable13offsetToIndexEj",
        "_ZN8ManageGS7resolveEP7GSTablej",
        "_ZN9Generator21copyFunctionToSandboxEP8FunctionP7Sandbox",
        "_ZN8ManageGS8setEntryEP7GSTablejm",
        "_ZN10ChunkFind2C1EP9Conductor",
        "_ZN10ChunkFind222findFunctionContainingEm",
        "_ZN7GSTable10getAtIndexEj",
        "_ZN12GSTableEntry15setLazyResolverEP5Chunk",
        "_ZNK9ChunkImpl10getAddressEv",
        "_ZNK22ChunkPositionDecoratorI9ChunkImplE11getPositionEv",
        "_ZNK16AbsolutePosition3getEv",
        //"_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE12_M_constructIPKcEEvT_S8_St20forward_iterator_tag.isra.249",
        "_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE12_M_constructIPKcEEvT_S8_St20forward_iterator_tag.isra.296",
        "_ZNK11GSTableLink16getTargetAddressEv",
        "_ZNK12GSTableEntry9getOffsetEv",
        "_ZN10ChunkFind228findFunctionContainingHelperEmP6Module",
        "_ZN13ChunkListImplI8FunctionE13createSpatialEv",
        "_ZN9Emulation24function_not_implementedEv",
        "_ZNSt8_Rb_treeImSt4pairIKmP8FunctionESt10_Select1stIS4_ESt4lessImESaIS4_EE29_M_get_insert_hint_unique_posESt23_Rb_tree_const_iteratorIS4_ERS1_",
        "_ZNSt8_Rb_treeImSt4pairIKmP8FunctionESt10_Select1stIS4_ESt4lessImESaIS4_EE24_M_get_insert_unique_posERS1_",
        "_ZNK9ChunkImpl8getRangeEv",
        "_ZNK21ComputedSizeDecoratorI22ChunkPositionDecoratorI9ChunkImplEE7getSizeEv",
        "_ZNK5Range8containsEm",
        "_ZN7GSTable12makeEntryForEP5Chunkb",
        "_ZN7GSTable11getEntryForEP5Chunk",
        "_ZNK7GSTable9nextIndexEv",
        "_ZN13ChunkListImplI12GSTableEntryE3addEPS0_",
        "_ZNSt8_Rb_treeIP5ChunkSt4pairIKS1_P12GSTableEntryESt10_Select1stIS6_ESt4lessIS1_ESaIS6_EE29_M_get_insert_hint_unique_posESt23_Rb_tree_const_iteratorIS6_ERS3_",
        "_ZN12MakeSemantic13getDispOffsetEP8Assemblyi",
        "_ZN22ControlFlowInstruction6acceptEP18InstructionVisitor",
        "_ZN22ControlFlowInstruction7writeToEPcb",
        "_ZN22ControlFlowInstruction21calculateDisplacementEv",
        "_ZNK10NormalLink16getTargetAddressEv",
        "_ZNK22ControlFlowInstruction7getSizeEv",
        "_ZN23IndirectCallInstruction6acceptEP18InstructionVisitor",
        "_ZN18InstructionVisitor5visitEP23IndirectCallInstruction",
        "_ZN17ReturnInstruction6acceptEP18InstructionVisitor",
        "_ZN18InstructionVisitor5visitEP17ReturnInstruction",

        "_ZN11SandboxImplI13MemoryBacking18WatermarkAllocatorIS0_EE6reopenEv",
        "_ZN13MemoryBacking6reopenEv",
        "_ZN11SandboxImplI13MemoryBacking18WatermarkAllocatorIS0_EE8finalizeEv",
        //"_ZN12SemanticImplI19DisassembledStorageE6acceptEP18InstructionVisitor",
        //"_ZN18InstrWriterCString5visitEP12SemanticImplI19DisassembledStorageE",
        //"_ZNK12SemanticImplI19DisassembledStorageE7getSizeEv",
        "_ZN17LinkedInstruction6acceptEP18InstructionVisitor",
        "_ZN18InstrWriterCString5visitEP17LinkedInstruction",
        "_ZN18InstrWriterCString5visitEP22ControlFlowInstruction",
        "_ZN18InstrWriterCString5visitEP21StackFrameInstruction",
        "_ZN18InstrWriterCString5visitEP18LiteralInstruction",
        "_ZN17LinkedInstruction7writeToEPcb",
        "_ZN17LinkedInstruction11getDispSizeEv",
        "_ZN12MakeSemantic25determineDisplacementSizeEP8Assembly",
        "_ZN17LinkedInstruction21calculateDisplacementEv",
        "_ZNK14DataOffsetLink16getTargetAddressEv",
        "_ZNK14OffsetPosition3getEv",

        "_ZN13MemoryBacking8finalizeEv",
        // for debugging
        "egalito_printf",
        "egalito_vfprintf",
        "write_decimal",
        "write_hex",
        "write_string",
        "_ZNK8Function7getNameB5cxx11Ev",
        "ifunc_resolver",
        "ifunc_select",
        "_ZNK9IFuncList6getForEm",
        "_ZNK13PLTTrampoline7getNameB5cxx11Ev",
        "_ZNK11Instruction7getNameB5cxx11Ev",
        "_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE12_M_constructIPKcEEvT_S8_St20forward_iterator_tag.isra.91",
        "_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE12_M_constructIPcEEvT_S7_St20forward_iterator_tag.isra.23",
        "_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE12_M_constructIPcEEvT_S7_St20forward_iterator_tag.isra.51",
    };
    for(auto name : resolvers) {
        makeResolvedEntry(name, egalito);
    }
}

void JitGSSetup::makeSupportGSEntries(Program *program) {
    for(auto module : CIter::children(program)) {
        if(module->getName() == "module-libc.so.6") {
            makeResolvedEntry("write", module);
            makeResolvedEntry("memcpy", module);
            makeResolvedEntry("malloc", module);
            makeResolvedEntry("malloc_hook_ini", module);
            makeResolvedEntry("ptmalloc_init.part.3", module);
            makeResolvedEntry("_dl_addr", module);
            makeResolvedEntry("malloc_consolidate", module);
            makeResolvedEntry("malloc_init_state", module);
            makeResolvedEntry("tcache_init.part.9", module);
            makeResolvedEntry("_int_malloc", module);
            makeResolvedEntry("sysmalloc", module);
            makeResolvedEntry("print_and_abort", module);
            makeResolvedEntry("__default_morecore", module);
            makeResolvedEntry("sbrk", module);
            makeResolvedEntry("brk", module);
            makeResolvedEntry("mprotect", module);
            makeResolvedEntry("mmap", module);
            makeResolvedEntry("strlen", module);
            makeResolvedEntry("memmove", module);
            makeResolvedEntry("newlocale", module);
            makeResolvedEntry("uselocale", module);
            makeResolvedEntry("wctob", module);
            makeResolvedEntry("btowc", module);
            makeResolvedEntry("_dl_mcount_wrapper_check", module);
            makeResolvedEntry("__gconv_btwoc_ascii", module);
            makeResolvedEntry("wctype_l", module);
            makeResolvedEntry("memcmp", module);
            makeResolvedEntry("strcmp", module);
            makeResolvedEntry("free", module);
            makeResolvedEntry("_int_free", module);
            makeResolvedEntry("tcache_put", module);
            makeResolvedEntry("tcache_get", module);
        }
        else if(module->getName() == "module-libstdc++.so.6") {
            makeResolvedEntry("__dynamic_cast", module);
            makeResolvedEntry("_ZdlPv", module);
            makeResolvedEntry("_Znwm", module);
            makeResolvedEntry("_ZSt29_Rb_tree_insert_and_rebalancebPSt18_Rb_tree_node_baseS0_RS_", module);
            makeResolvedEntry("_ZSt18_Rb_tree_decrementPSt18_Rb_tree_node_base", module);
            makeResolvedEntry("_ZStL23local_Rb_tree_decrementPSt18_Rb_tree_node_base", module);
            makeResolvedEntry("_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE10_M_replaceEmmPKcm", module);
            makeResolvedEntry("_ZNK10__cxxabiv117__class_type_info12__do_dyncastElNS0_10__sub_kindEPKS0_PKvS3_S5_RNS0_16__dyncast_resultE", module);
            // for debugging?
            makeResolvedEntry("_ZNSt8ios_baseC1Ev", module);
            makeResolvedEntry("_ZNSt6localeC1Ev", module);
            makeResolvedEntry("_ZNSt6locale13_S_initializeEv", module);
            makeResolvedEntry("_ZNSt6locale18_S_initialize_onceEv", module);
            makeResolvedEntry("_ZNSt6locale5_ImplC2Em", module);
            makeResolvedEntry("_ZNSt6locale5facet13_S_get_c_nameEv", module);
            makeResolvedEntry("_ZNSt5ctypeIcEC1EPKtbm", module);
            makeResolvedEntry("_ZNSt6locale5facet15_S_get_c_localeEv", module);
            makeResolvedEntry("_ZNSt6locale5facet18_S_initialize_onceEv", module);
            makeResolvedEntry("_ZNSt6locale5facet18_S_create_c_localeERP15__locale_structPKcS2_", module);
            makeResolvedEntry("_ZNSt6locale5_Impl16_M_install_facetEPKNS_2idEPKNS_5facetE", module);
            makeResolvedEntry("_ZNKSt6locale2id5_M_idEv", module);
            makeResolvedEntry("_ZNSt7codecvtIcc11__mbstate_tEC2Em", module);
            makeResolvedEntry("_ZNSt7__cxx118numpunctIcE22_M_initialize_numpunctEP15__locale_struct", module);
            makeResolvedEntry("_ZNSt7__cxx1110moneypunctIcLb0EE24_M_initialize_moneypunctEP15__locale_structPKc", module);
            makeResolvedEntry("_ZNSt7__cxx1110moneypunctIcLb1EE24_M_initialize_moneypunctEP15__locale_structPKc", module);
            makeResolvedEntry("_ZNSt11__timepunctIcEC1EPSt17__timepunct_cacheIcEm", module);
            makeResolvedEntry("_ZNSt11__timepunctIcE23_M_initialize_timepunctEP15__locale_struct", module);
            makeResolvedEntry("_ZNSt7__cxx118messagesIcEC2Em", module);
            makeResolvedEntry("_ZNSt5ctypeIwEC1Em", module);
            makeResolvedEntry("_ZNSt5ctypeIwE19_M_initialize_ctypeEv", module);
            makeResolvedEntry("_ZNKSt5ctypeIwE19_M_convert_to_wmaskEt", module);
            makeResolvedEntry("_ZNSt7codecvtIwc11__mbstate_tEC2Em", module);
            makeResolvedEntry("_ZNSt7__cxx118numpunctIwE22_M_initialize_numpunctEP15__locale_struct", module);
            makeResolvedEntry("_ZNSt7__cxx1110moneypunctIwLb0EE24_M_initialize_moneypunctEP15__locale_structPKc", module);
            makeResolvedEntry("_ZNSt7__cxx1110moneypunctIwLb1EE24_M_initialize_moneypunctEP15__locale_structPKc", module);
            makeResolvedEntry("_ZNSt11__timepunctIwEC1EPSt17__timepunct_cacheIwEm", module);
            makeResolvedEntry("_ZNSt11__timepunctIwE23_M_initialize_timepunctEP15__locale_struct", module);
            makeResolvedEntry("_ZNSt7__cxx118messagesIwEC1Em", module);
            makeResolvedEntry("_ZNSt6locale5_Impl13_M_init_extraEPPNS_5facetE", module);
            makeResolvedEntry("_ZNSt8numpunctIcE22_M_initialize_numpunctEP15__locale_struct", module);
            makeResolvedEntry("_ZNSt10moneypunctIcLb0EE24_M_initialize_moneypunctEP15__locale_structPKc", module);
            makeResolvedEntry("_ZNSt10moneypunctIcLb1EE24_M_initialize_moneypunctEP15__locale_structPKc", module);
            makeResolvedEntry("_ZNSt8messagesIcEC1Em", module);
            makeResolvedEntry("_ZNSt8numpunctIwE22_M_initialize_numpunctEP15__locale_struct", module);
            makeResolvedEntry("_ZNSt10moneypunctIwLb0EE24_M_initialize_moneypunctEP15__locale_structPKc", module);
            makeResolvedEntry("_ZNSt10moneypunctIwLb1EE24_M_initialize_moneypunctEP15__locale_structPKc", module);
            makeResolvedEntry("_ZNSt8messagesIwEC2Em", module);
            makeResolvedEntry("_ZNSt9basic_iosIcSt11char_traitsIcEE4initEPSt15basic_streambufIcS1_E", module);
            makeResolvedEntry("_ZNSt8ios_base7_M_initEv", module);
            makeResolvedEntry("_ZNSt6localeaSERKS_", module);
            makeResolvedEntry("_ZNSt6localeD1Ev", module);
            makeResolvedEntry("_ZNSt9basic_iosIcSt11char_traitsIcEE15_M_cache_localeERKSt6locale", module);
            makeResolvedEntry("_ZSt9has_facetISt5ctypeIcEEbRKSt6locale", module);
            makeResolvedEntry("_ZNK10__cxxabiv121__vmi_class_type_info12__do_dyncastElNS_17__class_type_info10__sub_kindEPKS1_PKvS4_S6_RNS1_16__dyncast_resultE", module);
            makeResolvedEntry("_ZSt9use_facetISt5ctypeIcEERKT_RKSt6locale", module);
            makeResolvedEntry("_ZSt9has_facetISt7num_putIcSt19ostreambuf_iteratorIcSt11char_traitsIcEEEEbRKSt6locale", module);
            makeResolvedEntry("_ZNK10__cxxabiv120__si_class_type_info12__do_dyncastElNS_17__class_type_info10__sub_kindEPKS1_PKvS4_S6_RNS1_16__dyncast_resultE", module);
            makeResolvedEntry("_ZSt9use_facetISt7num_putIcSt19ostreambuf_iteratorIcSt11char_traitsIcEEEERKT_RKSt6locale", module);
            makeResolvedEntry("_ZSt9has_facetISt7num_getIcSt19istreambuf_iteratorIcSt11char_traitsIcEEEEbRKSt6locale", module);
            makeResolvedEntry("_ZSt9use_facetISt7num_getIcSt19istreambuf_iteratorIcSt11char_traitsIcEEEERKT_RKSt6locale", module);
            makeResolvedEntry("_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l", module);
            makeResolvedEntry("_ZNSo6sentryC1ERSo", module);
            makeResolvedEntry("_ZNSt15basic_streambufIcSt11char_traitsIcEE6xsputnEPKcl", module);
            makeResolvedEntry("_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE8overflowEi", module);
            makeResolvedEntry("_ZNSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE8_M_pbumpEPcS5_l", module);
            makeResolvedEntry("_ZNSo6sentryD2Ev", module);
            makeResolvedEntry("_ZNSo9_M_insertImEERSoT_", module);
            makeResolvedEntry("_ZNKSt5ctypeIcE13_M_widen_initEv", module);
            makeResolvedEntry("_ZNKSt7num_putIcSt19ostreambuf_iteratorIcSt11char_traitsIcEEE6do_putES3_RSt8ios_basecm", module);
            makeResolvedEntry("_ZNKSt7num_putIcSt19ostreambuf_iteratorIcSt11char_traitsIcEEE13_M_insert_intImEES3_S3_RSt8ios_basecT_", module);
            makeResolvedEntry("_ZNKSt11__use_cacheISt16__numpunct_cacheIcEEclERKSt6locale", module);
            makeResolvedEntry("_ZSt13__int_to_charIcmEiPT_T0_PKS0_St13_Ios_Fmtflagsb", module);
            makeResolvedEntry("_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_assignERKS4_", module);
            makeResolvedEntry("_ZNSt8ios_baseD2Ev", module);
            makeResolvedEntry("_ZNSt8ios_base17_M_call_callbacksENS_5eventE", module);
            makeResolvedEntry("_ZNSt8ios_base20_M_dispose_callbacksEv", module);
            makeResolvedEntry("_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_createERmm", module);
            makeResolvedEntry("_ZNKSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE5c_strEv", module);
            makeResolvedEntry("_ZNKSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE7compareEPKc", module);
        }
        else if(module->getName() == "module-libdistorm3.so") {
            makeResolvedEntry("distorm_decompose64", module);
            makeResolvedEntry("decode_internal", module);
            makeResolvedEntry("prefixes_is_valid", module);
            makeResolvedEntry("prefixes_decode", module);
            makeResolvedEntry("prefixes_ignore", module);
            makeResolvedEntry("inst_lookup", module);
            makeResolvedEntry("operands_extract", module);
            makeResolvedEntry("operands_extract_modrm", module);
            makeResolvedEntry("prefixes_use_segment", module);
            makeResolvedEntry("prefixes_set_unused_mask", module);
        }
    }
    makeResolvedEntryForPLT("memcpy@plt", program);
    makeResolvedEntryForPLT("_dl_find_dso_for_object@plt", program);
    makeResolvedEntryForPLT("distorm_decompose64@plt", program);
    makeResolvedEntryForPLT("strlen@plt", program);
    makeResolvedEntryForPLT("memmove@plt", program);
    makeResolvedEntryForPLT("memcmp@plt", program);
    makeResolvedEntryForPLT("strcmp@plt", program);
}

void JitGSSetup::makeResolvedEntry(const char *name, Module *module) {
    bool found = false;
    for(auto f : CIter::functions(module)) {
        if(f->hasName(name)) {
            gsTable->makeEntryFor(f, true);
            found = true;
        }
    }
    assert(found);
}

void JitGSSetup::makeResolvedEntryForPLT(std::string name, Program *program) {
    bool found = false;
    for(auto module : CIter::children(program)) {
        for(auto plt : CIter::children(module->getPLTList())) {
            if(plt->getName() == name) {
                found = true;
                gsTable->makeEntryFor(plt, true);
                break;
            }
        }
    }
    assert(found);
}