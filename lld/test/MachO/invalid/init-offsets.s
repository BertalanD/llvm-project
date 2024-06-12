# REQUIRES: x86

# RUN: llvm-mc -filetype=obj -triple=x86_64-apple-darwin %s -o %t.o
# RUN: not %lld -lSystem -init_offsets  %t.o -o %t.out 2>&1 | FileCheck %s

# CHECK-NOT: foo

.globl _main
.text
_main:
  leaq _init_slot(%rip), %rax
  ret

.section __DATA,__mod_init_func,mod_init_funcs
_init_slot:
  .quad _main

.subsections_via_symbols
