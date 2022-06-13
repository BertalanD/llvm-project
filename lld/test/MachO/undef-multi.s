# REQUIRES: aarch64

# RUN: llvm-mc -filetype=obj -triple=arm64-apple-darwin %s -o %t.o
# RUN: not %lld -arch arm64 %t.o -o /dev/null 2>&1 | FileCheck -DFILE=%t.o %s

# CHECK: error: undefined symbol: _undef
# CHECK-NEXT: >>> referenced by [[FILE]]:(symbol _main+0x0)
# CHECK-NEXT: >>> referenced by [[FILE]]:(symbol _foo+0x0)
# CHECK-NEXT: >>> referenced by [[FILE]]:(symbol _bar+0x0)
# CHECK-NEXT: >>> referenced 1 more times

.file "multiple-undef.s"
// FIXME: This returns totally incorrect values if subsections_via_symbols is not set!
//.subsections_via_symbols

.globl _main
_main:
    b _undef

.globl _foo
.p2align 2
_foo:
    b _undef

.global _bar
.p2align 2
_bar:
    b _undef

.globl _baz
.p2align 2
_baz:
    b _undef
