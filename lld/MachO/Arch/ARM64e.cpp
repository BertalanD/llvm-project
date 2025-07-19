//===- ARM64.cpp ----------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Arch/ARM64Common.h"
#include "InputFiles.h"
#include "Symbols.h"
#include "SyntheticSections.h"
#include "Target.h"

#include "lld/Common/ErrorHandler.h"
#include "mach-o/compact_unwind_encoding.h"
#include "llvm/BinaryFormat/MachO.h"

using namespace llvm;
using namespace llvm::MachO;
using namespace llvm::support::endian;
using namespace lld;
using namespace lld::macho;

namespace {

struct ARM64e : ARM64Common {
  ARM64e();
  void writeStub(uint8_t *buf, const Symbol &, uint64_t) const override;
  void writeStubHelperHeader(uint8_t *buf) const override;
  void writeStubHelperEntry(uint8_t *buf, const Symbol &,
                            uint64_t entryAddr) const override;

  void writeObjCMsgSendStub(uint8_t *buf, Symbol *sym, uint64_t stubsAddr,
                            uint64_t &stubOffset, uint64_t selrefVA,
                            Symbol *objcMsgSend) const override;
  void populateThunk(InputSection *thunk, Symbol *funcSym) override;

  void initICFSafeThunkBody(InputSection *thunk,
                            Symbol *targetSym) const override;
  Symbol *getThunkBranchTarget(InputSection *thunk) const override;
  //uint32_t getICFSafeThunkSize() const override;
};

} // namespace

// These are identical to ARM64's relocation attributes, except for
// the new AUTHENTICATED_POINTER reloc type.
static constexpr std::array<RelocAttrs, 12> relocAttrsArray{{
#define B(x) RelocAttrBits::x
    {"UNSIGNED",
     B(UNSIGNED) | B(ABSOLUTE) | B(EXTERN) | B(LOCAL) | B(BYTE4) | B(BYTE8)},
    {"SUBTRACTOR", B(SUBTRAHEND) | B(EXTERN) | B(BYTE4) | B(BYTE8)},
    {"BRANCH26", B(PCREL) | B(EXTERN) | B(BRANCH) | B(BYTE4)},
    {"PAGE21", B(PCREL) | B(EXTERN) | B(BYTE4)},
    {"PAGEOFF12", B(ABSOLUTE) | B(EXTERN) | B(BYTE4)},
    {"GOT_LOAD_PAGE21", B(PCREL) | B(EXTERN) | B(GOT) | B(BYTE4)},
    {"GOT_LOAD_PAGEOFF12",
     B(ABSOLUTE) | B(EXTERN) | B(GOT) | B(LOAD) | B(BYTE4)},
    {"POINTER_TO_GOT", B(PCREL) | B(EXTERN) | B(GOT) | B(POINTER) | B(BYTE4)},
    {"TLVP_LOAD_PAGE21", B(PCREL) | B(EXTERN) | B(TLV) | B(BYTE4)},
    {"TLVP_LOAD_PAGEOFF12",
     B(ABSOLUTE) | B(EXTERN) | B(TLV) | B(LOAD) | B(BYTE4)},
    {"ADDEND", B(ADDEND)},
    {"AUTHENTICATED_POINTER",
      B(UNSIGNED) | B(AUTH) | B(ABSOLUTE) | B(EXTERN) | B(LOCAL) | B(BYTE8)},
#undef B
}};

static constexpr uint32_t stubCode[] = {
  0x90000011, // 0x0: adrp  x17, pointer@page
  0x91000231, // 0x4: add   x17, x17, pointer@pageoff
  0xf9400230, // 0x8: ldr   x16, [x17]
  0xd71f0a11, // 0xC: braa  x16, x17
};

void ARM64e::writeStub(uint8_t *buf8, const Symbol &sym,
                      uint64_t pointerVA) const {
  auto *buf32 = reinterpret_cast<uint32_t *>(buf8);
  SymbolDiagnostic d = {&sym, "stub"};
  uint64_t pcPageBits =
       pageBits(in.stubs->addr + sym.stubsIndex * sizeof(stubCode));

  encodePage21(buf32, d, stubCode[0], pageBits(pointerVA) - pcPageBits);
  encodePageOff12(buf32 + 1, d, stubCode[1], pointerVA);
  write32le(buf32 + 2, stubCode[2]);
  write32le(buf32 + 3, stubCode[3]);
}

void ARM64e::writeStubHelperHeader(uint8_t *buf8) const {
  assert(false && "writeStubHelperHeader: lazy binding is not supported for ARM64e");
}

void ARM64e::writeStubHelperEntry(uint8_t *buf8, const Symbol &sym,
                                 uint64_t entryVA) const {
  assert(false && "writeStubHelperEntry: lazy binding is not supported for ARM64e");
}

void ARM64e::writeObjCMsgSendStub(uint8_t *buf, Symbol *sym, uint64_t stubsAddr,
                                 uint64_t &stubOffset, uint64_t selrefVA,
                                 Symbol *objcMsgSend) const {
  assert(false && "implement writeObjCMsgSendStub for arm64e");
}

static constexpr uint32_t thunkCode[] = {};

void ARM64e::populateThunk(InputSection *thunk, Symbol *funcSym) {
  assert(false && "implement populateThunk for arm64e");
}

void ARM64e::initICFSafeThunkBody(InputSection *thunk, Symbol *targetSym) const {
  assert(false && "implement initICFSafeThunkBody for arm64e");
}

Symbol *ARM64e::getThunkBranchTarget(InputSection *thunk) const {
  assert(thunk->relocs.size() == 1 &&
         "expected a single reloc on ARM64 ICF thunk");
  auto &reloc = thunk->relocs[0];
  assert(isa<Symbol *>(reloc.referent) &&
         "ARM64 thunk reloc is expected to point to a Symbol");

  return cast<Symbol *>(reloc.referent);
}

// uint32_t ARM64e::getICFSafeThunkSize() const { return sizeof(icfSafeThunkCode); }

ARM64e::ARM64e() : ARM64Common(LP64()) {
  cpuType = CPU_TYPE_ARM64;
  cpuSubtype = CPU_SUBTYPE_ARM64E;

  stubSize = sizeof(stubCode);
  thunkSize = sizeof(thunkCode);

  // Branch immediate is two's complement 26 bits, which is implicitly
  // multiplied by 4 (since all functions are 4-aligned: The branch range
  // is -4*(2**(26-1))..4*(2**(26-1) - 1).
  backwardBranchRange = 128 * 1024 * 1024;
  forwardBranchRange = backwardBranchRange - 4;

  modeDwarfEncoding = UNWIND_ARM64_MODE_DWARF;
  subtractorRelocType = ARM64_RELOC_SUBTRACTOR;
  unsignedRelocType = ARM64_RELOC_UNSIGNED;

  /*
  stubHelperHeaderSize = sizeof(stubHelperHeaderCode);
  stubHelperEntrySize = sizeof(stubHelperEntryCode);
  */

  relocAttrs = {relocAttrsArray.data(), relocAttrsArray.size()};
}

TargetInfo *macho::createARM64eTargetInfo() {
  static ARM64e t;
  return &t;
}
