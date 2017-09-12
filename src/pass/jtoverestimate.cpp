#include <sys/mman.h>
#include "jtoverestimate.h"
#include "elf/elfspace.h"
#include "analysis/jumptable.h"
#include "jumptablepass.h"

#undef DEBUG_GROUP
#define DEBUG_GROUP djumptable
#include "log/log.h"

void JumpTableOverestimate::visit(Module *module) {
    this->module = module;
    visit(module->getJumpTableList());
}

void JumpTableOverestimate::visit(JumpTableList *jumpTableList) {
    recurse(jumpTableList);

#ifdef ARCH_X86_64
    auto tableSection = module->getElfSpace()->getElfMap()
        ->findSection(".text");
    auto tableReadPtr = module->getElfSpace()->getElfMap()
        ->getSectionReadPtr<unsigned char *>(tableSection);
#else
    auto tableSection = module->getElfSpace()->getElfMap()
        ->findSection(".rodata");
    if(!tableSection) return;
    auto tableReadPtr = module->getElfSpace()->getElfMap()
        ->getSectionReadPtr<unsigned char *>(tableSection);
#endif

    for(auto it = tableMap.begin(); it != tableMap.end(); it ++) {
        JumpTable *currentTable = (*it).second;
        if(currentTable->getDescriptor()->getEntries() >= 0) {
            // bounds for this table are already known
            continue;
        }

        address_t address = currentTable->getDescriptor()->getAddress();
        int scale = currentTable->getDescriptor()->getScale();
        for(int count = 1; ; count ++) {
            address_t computed = address + count*scale;
            if(tableMap.find(computed) != tableMap.end()) {
                // reached another jump table's entries, stop looking
                setEntries(currentTable, count);
                break;
            }

            address_t offset = tableSection->convertVAToOffset(computed);
            int value = *reinterpret_cast<int *>(tableReadPtr + offset);

            //LOG(1, "looks like value is " << std::hex << value);

            // for relative jump tables
            value += address;

            auto function = currentTable->getDescriptor()->getFunction();
            if(!function->getRange().contains(value)) {
                // this entry would be outside the function, stop looking
                setEntries(currentTable, count);
                break;
            }
        }
    }
}

void JumpTableOverestimate::visit(JumpTable *jumpTable) {
    tableMap[jumpTable->getAddress()] = jumpTable;
}

void JumpTableOverestimate::setEntries(JumpTable *jumpTable, int count) {
    LOG(5, "APPARENTLY, table " << std::hex << jumpTable->getAddress()
            << " in [" << jumpTable->getFunction()->getName()
            << "] has " << std::dec << count << " entries");
    jumpTable->getDescriptor()->setEntries(count);
    JumpTablePass(module).makeChildren(jumpTable, count);
}
