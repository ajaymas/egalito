#include <iostream>  // for std::cout.flush()
#include <iomanip>
#include <cstdio>  // for std::fflush
#include "generator.h"
#include "operation/mutator.h"
#include "pass/clearspatial.h"
#include "instr/semantic.h"
#include "instr/writer.h"

#undef DEBUG_GROUP
#define DEBUG_GROUP dassign
#include "log/log.h"
#include "log/temp.h"

void Generator::pickAddressesInSandbox(Module *module, Sandbox *sandbox) {
    for(auto f : CIter::functions(module)) {
        //auto slot = sandbox->allocate(std::max((size_t)0x1000, f->getSize()));
        auto slot = sandbox->allocate(f->getSize());
        LOG(2, "    alloc 0x" << std::hex << slot.getAddress()
            << " for [" << f->getName()
            << "] size " << std::dec << f->getSize());
        ChunkMutator(f).setPosition(slot.getAddress());
    }

    if(module->getPLTList()) {
        // these don't have to be contiguous
        const size_t pltSize = PLTList::getPLTTrampolineSize();
        for(auto plt : CIter::plts(module)) {
            auto slot = sandbox->allocate(pltSize);
            LOG(2, "    alloc 0x" << std::hex << slot.getAddress()
                << " for [" << plt->getName()
                << "] size " << std::dec << pltSize);
            ChunkMutator(plt).setPosition(slot.getAddress());
        }
    }

    ClearSpatialPass clearSpatial;
    module->accept(&clearSpatial);
}

void Generator::copyCodeToSandbox(Module *module, Sandbox *sandbox) {
    LOG(1, "Copying code into sandbox");
    for(auto f : CIter::functions(module)) {
        LOG(2, "    writing out [" << f->getName() << "] at 0x"
            << std::hex << f->getAddress());

#if 0
        if(f->getName() != "puts") {
            copyFunctionToSandbox(f);
        }
#else
        copyFunctionToSandbox(f);
#endif
    }

    copyPLTEntriesToSandbox(module, sandbox);
}

void Generator::copyPLTEntriesToSandbox(Module *module, Sandbox *sandbox) {
    if(module->getPLTList()) {
        LOG(1, "Copying PLT entries into sandbox");
        for(auto plt : CIter::plts(module)) {
            copyPLTToSandbox(plt);
        }
    }
}

void Generator::copyFunctionToSandbox(Function *function) {
    char *output = reinterpret_cast<char *>(function->getAddress());
    for(auto b : CIter::children(function)) {
        for(auto i : CIter::children(b)) {
            LOG(10, " at " << std::hex << i->getAddress());
            if(useDisps) {
                InstrWriterCString writer(output);
                i->getSemantic()->accept(&writer);
            }
            else {
                InstrWriterForObjectFile writer(output);
                i->getSemantic()->accept(&writer);
            }
            output += i->getSemantic()->getSize();
        }
    }
}

void Generator::copyPLTToSandbox(PLTTrampoline *trampoline) {
    char *output = reinterpret_cast<char *>(trampoline->getAddress());
    trampoline->writeTo(output);
}

void Generator::pickFunctionAddressInSandbox(Function *function,
    Sandbox *sandbox) {

    auto slot = sandbox->allocate(function->getSize());
    ChunkMutator(function).setPosition(slot.getAddress());
#if 0
    ClearSpatialPass clearSpatial;
    module->accept(&clearSpatial);
#endif
}

void Generator::pickPLTAddressInSandbox(PLTTrampoline *trampoline,
    Sandbox *sandbox) {

    auto slot = sandbox->allocate(PLTList::getPLTTrampolineSize());
    ChunkMutator(trampoline).setPosition(slot.getAddress());
#if 0
    ClearSpatialPass clearSpatial;
    module->accept(&clearSpatial);
#endif
}

void Generator::instantiate(Function *function, Sandbox *sandbox) {
    pickFunctionAddressInSandbox(function, sandbox);
    copyFunctionToSandbox(function);
}

void Generator::instantiate(PLTTrampoline *trampoline, Sandbox *sandbox) {
    pickPLTAddressInSandbox(trampoline, sandbox);
    copyPLTToSandbox(trampoline);
}

void Generator::jumpToSandbox(Sandbox *sandbox, Module *module,
    const char *function) {

    auto f = CIter::named(module->getFunctionList())->find(function);
    if(!f) return;

    LOG(1, "jumping to [" << function << "] at 0x"
        << std::hex << f->getAddress());
    int (*mainp)(int, char **) = (int (*)(int, char **))f->getAddress();

    int argc = 1;
    char *argv[] = {(char *)"/dev/null", NULL};

    std::cout.flush();
    std::fflush(stdout);
    mainp(argc, argv);

    LOG(1, "RETURNED from target");
}
