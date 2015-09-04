README: Code pointer protection via tagged memory on LowRISC
============================================================

URGENT BUGS
-----------

Tests break:
$ cd test/LowRISCTaggedMemory/MemcpyEtc/MemcpyMemmoveRandomTags
$ ./build.sh 8 clang
...

=> Hangs forever in llc, in RISCVBranchSelector, adding more and more branches.
This appears to originate in commit 53e896f55ed05adc341ee6b78ce5eaf794f70cbd,
which must trigger some corner-case in the branch selector. Earlier commits
work fine.

Introduction
------------

This branch based on RISCV-LLVM provides:
- Stack protection: Registers spilled in function prologue e.g. SP, RA are read only.
- Code pointer protection: Uses tags to guarantee when we read a code pointer it was previously written as one.

See docs/TagCodePointers/ for more details, including design decisions, security implications, compatibility, urgent bugfixes and possible larger extensions.
(Flat XML openoffice documents, you may need to install filters)

This file explains how to set up the prerequisites, which involve several different repositories, since we rely on some ISA extensions (e.g. atomic tagged memory operations).

Base LowRISC (with new ABI)
---------------------------

First set up LowRISC. This guide is helpful but seriously out of date:
http://www.lowrisc.org/docs/tutorial/setup/

The above advises to use GCC 4.6 (the old "riscv-gcc" repo) for building Linux, and Linux binaries. This is seriously outdated and uses a different ABI. RISCV-LLVM uses the new ABI (and uses binutils, Glibc etc), so to generate Linux binaries using RISCV-LLVM we need the newer GCC.

Check out the LowRISC repository including submodules, but ignore the bit about riscv-gcc.
Then install the modern version:

# Download the source
$ git clone https://github.com/lowrisc/lowrisc-chip.git
$ cd lowrisc-chip
$ git submodule update --init --recursive
# Set environment variables
$ source lowrisc-chip/set_riscv_env.sh
$ cd $TOP/riscv-tools
... # Compiles the riscv64-unknown-elf compiler (using newlib, for bare and proxy kernel binaries)
$ cd riscv-gnu-toolchain/build
$ make -j9 linux
... # Compiles the riscv64-unknown-linux-* compiler (using glibc, for linux and linux binaries)

The above does not include my code yet...

Note to hackers: the new ABI for gcc 4.9.2+ is here:
https://blog.riscv.org/2015/01/announcing-the-risc-v-gcc-4-9-port-and-new-abi/
https://blog.riscv.org/wp-content/uploads/2015/01/riscv-calling.pdf

To get Linux working with the new ABI, you need my changes to register save/restore. This is based on the upstream RISCV (not LowRISC) branch new_privileged_isa, commit 043bb5db922bb57749c61e7e2daec5c63e4574e1. Also you need to run Spike with the "bbl" option.

linux: Upstream RISCV-Linux, new_privileged_isa branch, unpacked on top of 3.14.49.
See https://github.com/riscv/riscv-linux/blob/master/README.md for installation instructions.
$ tar xf linux-3.14.49.tar
$ cd linux-3.14.49
$ git init
# Use upstream RISCV, not LowRISC
$ git remote add origin https://github.com/toad/riscv-linux.git
$ git fetch
# Use the save-tagged-regs branch.
$ git checkout -f -t origin/save-tagged-regs
$ git log
commit 1b0e8e77404542a3b2e0c843aeb87c70b95c9016
...
# Building requires ARCH=riscv
$ make ARCH=riscv defconfig
$ make ARCH=riscv menuconfig
# Build the kernel
$ make ARCH=riscv -j vmlinux
$ cd ..
# You've already unpacked busybox as per the instructions above, right?
$ ./make_root.sh
# Spike now needs the "bbl" option (Berkeley Boot Loader).
$ spike +disk=root.bin bbl linux-3.14.49/vmlinux

Also, we need upstream fesvr, merged with the tag_demo branch from LowRISC:
LowRISC: https://github.com/lowrisc/riscv-fesvr.git branch tag_demo commit 3bb4837d717dbc289802cc5bba40a0b89270c298
RISCV: https://github.com/riscv/riscv-fesvr.git branch master commit ceb56f29aacf1ff8a9997e9a57b855936ba00fd6
You can get this from my repository:
$ git remote add toad https://github.com/toad/riscv-fesvr.git
$ git fetch toad
$ git checkout toad/master
The only change I have made here is merging Wei's tag_demo with upstream ISA-related changes.

riscv-isa-sim and riscv-tests also need to be updated from upstream, see below.

Repositories and branches: riscv-tools, Spike, etc
--------------------------------------------------

Now you need to install the patched versions of Spike, the compiler, glibc, and newlib.
This includes new instructions for tagged registers (in binutils, gcc, spike, etc) and memcpy/memmove changes (in glibc/newlib).

lowrisc-chip: Unchanged from LowRISC:
https://github.com/lowrisc/lowrisc-chip.git
master branch, commit ad881e1a72a257f9e3ff4bff2928e7f9f0672344

riscv-tools: Forked from upstream RISCV:
Origin: https://github.com/lowRISC/riscv-tools.git
Upstream: https://github.com/riscv/riscv-tools.git
My changes: https://github.com/toad/riscv-tools.git (branch fix-make-root)
Lucas's changes: https://github.com/lucas-sonnabend/riscv-tools.git
$ cd $TOP
$ source lowrisc-chip/set_riscv_env.sh
$ git add toad https://github.com/toad/riscv-tools.git
$ git fetch toad
$ git checkout -b fix-make-root toad/fix-make-root
$ git log
commit ed4d8d27abe0101b652530c5dbde5d95e4f3707e
...
Major changes:
- You need genext2fs, but don't need root, to run make_root.sh.
- run_test_linux_spike.sh provides some convenient test automation for Linux Spike tests.
- Updates for new ABI and new Linux.
- Also don't build qemu for now.

linux, riscv-fesvr: See above (new_privileged_isa changes).

riscv-pk: Proxy kernel
Origin: https://github.com/riscv/riscv-pk.git (branch master)
Lucas: https://github.com/lucas-sonnabend/riscv-pk.git
My changes: https://github.com/toad/riscv-pk.git (branch save-tagged-regs)
$ cd riscv-pk
$ git remote add toad https://github.com/toad/riscv-pk.git
$ git fetch toad
$ git checkout -b save-tagged-regs toad/save-tagged-regs
$ git log
commit 18cbb9a5091b871ae9dd8d4bc2f559c8396c9342
...
Major changes here:
- Save tagged registers in the proxy kernel on an interrupt etc. (Needed for tagged registers to not break!)
- Logging when a trap occurs. (Useful for debugging)

riscv-opcodes:
Origin: https://github.com/riscv/riscv-opcodes.git
Lucas: https://github.com/lucas-sonnabend/riscv-opcodes.git (branch master)
My copy: https://github.com/toad/riscv-opcodes.git (branch master, just a copy of Lucas)
$ cd riscv-opcodes
$ git remote add lucas https://github.com/lucas-sonnabend/riscv-opcodes.git
$ git fetch lucas
$ git checkout lucas/master
$ git log
commit e91ee9dc4e7467cb1c903ce41ca55e46dcbe148e
...
The new instructions are added here, but are also needed elsewhere. This is not really needed to build.

riscv-isa-sim: (Spike)
LowRISC: https://github.com/lowrisc/riscv-isa-sim.git (irrelevant)
Upstream: https://github.com/lowrisc/riscv-isa-sim.git (branch master)
Lucas: https://github.com/lucas-sonnabend/riscv-isa-sim.git (branch check_on_all_no_propagation)
Hongyan: https://github.com/Jerryxia32/riscv-isa-sim.git
My changes: https://github.com/toad/riscv-isa-sim.git (branch check_on_all_no_propagation)
$ cd riscv-isa-sim
$ git remote add upstream https://github.com/lowrisc/riscv-isa-sim.git
$ git fetch upstream
$ git checkout upstream/master
# At this point, you can build the system *without* tagged registers support
$ git remote add toad https://github.com/toad/riscv-isa-sim.git
$ git fetch toad
$ git checkout toad/check_on_all_no_propagation
$ git log
commit 7f74bff57a18ca23cb71ce3b19339be617aa0e7b
...
Major changes:
- Support for new instructions, tagged registers and trap CSR's.
- Check for trap on all memory accesses (except in supervisor mode).
- Clear tags on normal writes.
- Hack to allow writing to tags on read-only pages (will eventually be replaced with loader changes, see the big document for explanation).
Branches and older code:
- tagged-spike*: My initial code, hard-coded bits for trap on read, write, clear on lazy. Not sufficient due to race conditions.
- Initial tagged registers support: Propagate tags to/from regs on all memory operations. Not a good idea, but might be useful example for implementing e.g. tagged atomic ops later if needed. Don't check except on LR/SC.

Repositories and branches: Tests
--------------------------------

The bulk of the tests for code pointer tagging are in riscv-llvm, later.
However, these are useful for testing the underlying instructions etc, and for low-level code examples too.

lowrisc-tag-tests:
LowRISC: https://github.com/lowrisc/lowrisc-tag-tests.git
Lucas: https://github.com/lucas-sonnabend/lowrisc-tag-tests.git (branch master)
My trivial cleanups: https://github.com/toad/lowrisc-tag-tests.git (branch tagged-memory-enforcement-tests)
$ cd lowrisc-tag-tests
$ git remote add toad https://github.com/toad/lowrisc-tag-tests.git
$ git fetch toad
$ git checkout toad/tagged-memory-enforcement-tests
$ git log
commit b4f11fcf68a75c3f2dea6718dbf24bc8fbe1618b
...
Major changes:
- Lots of tests for the new tagged registers.
- Lots of C code that could be useful.
- Many of the tests are for specific use cases e.g. full/empty bits.

riscv-tests:
LowRISC: https://github.com/lowrisc/riscv-tests.git
Upstream: https://github.com/riscv/riscv-tests.git (branch master)
Lucas: https://github.com/lucas-sonnabend/riscv-tests.git (branch master)
Mine: https://github.com/toad/lowrisc-tag-tests.git (branch master-upstream-merge-tags)
$ cd riscv-tests
$ git remote add toad https://github.com/toad/lowrisc-tag-tests.git
$ git fetch toad
$ git checkout toad/master-upstream-merge-tags
Major changes:
- Some tag tests here too.
- Upstream RISCV merged because of ISA change. Messy! May be unnecessary.

Repositories and branches: riscv-gnu-toolchain
----------------------------------------------

This contains binutils, gcc, glibc and newlib. Most of which need changes to support the new instructions. Also, there are changes in memcpy/memmove in glibc and newlib to support copying tagged memory (sometimes, see the background document).

riscv-gnu-toolchain:
LowRISC: https://github.com/lowrisc/riscv-gnu-toolchain.git
Upstream: https://github.com/riscv/riscv-gnu-toolchain.git
Lucas: https://github.com/lucas-sonnabend/riscv-gnu-toolchain.git
Hongyan: https://github.com/Jerryxia32/riscv-gnu-toolchain.git
My changes: https://github.com/toad/riscv-gnu-toolchain.git (branch memcpy-memmove-tags-if-aligned-merged-lucas)
$ git remote add toad https://github.com/toad/riscv-gnu-toolchain.git
$ git fetch toad
$ git checkout toad/memcpy-memmove-tags-if-aligned-merged-lucas
$ git log
commit 5e1417215239c3bdceb71cdfda2e53dd22b60963
...
Major changes here:
- Merged upstream for new ISA, new versions of GCC etc.
- Support new instructions in binutils (gas etc).
- Partially fix the insecure build process (fix-evil-downloads-merged-upstream).
- New platform-specific header, sys/platform/tag.h, with various tag-related operations and default tag values.
- memcpy(), memmove():
-- Variants for copying and not copying tags in sys/platform/tag.h.
-- Copy only if all three arguments are aligned.
-- Both in glibc and newlib.

You can delete the riscv-gcc repository if you have it. You do NOT need it.

You probably do need to build the Linux toolchain. See the "Base LowRISC" section above for how to build riscv64-unknown-linux-gnu-gcc etc.

LLVM
----

Code pointer tagging is actually implemented in LLVM. Lots more detail on this in the background document.
This is based on a relatively old version of riscv-llvm.

My changes: https://github.com/toad/riscv-llvm.git
Upstream: https://github.com/riscv/riscv-llvm.git
$ cd $TOP
$ git clone https://github.com/toad/riscv-llvm.git
$ cd riscv-llvm
$ git checkout toad/tag-code-pointers-tagged-registers-atomic
$ git submodule update --init --recursive
$ git log
commit 1635e3277bcb6080bf4aa735b7788000882ac749
...
$ cd riscv-clang
$ git checkout arch-features-lowrisc
$ git log
commit deab1ad3c1c7bb9a1db0080492f356106cfc98d9
...
Major changes:
- New instructions and intrinsic functions for (atomic) tagged memory access. (RISCV backend)
- Code pointer tagging. (Middle-layer pass)
-- Compile-time extensions for tagging ordinary pointers, void*.
-- Compile-time extensions for allowing aliasing between void* and sensitive/ordinary pointers.
- Prologue/epilogue spill protection. (RISCV backend)
- Builtin memmove()/memcpy() may or may not copy tags according to alignment. (SelectionDAG)
- Lots of tests (including for memcpy/memmove, which are both here and in newlib/glibc).
- Add #define's for tagged memory. (Clang)
- Add architecture options for LowRISC and tagged memory. (RISCV backend and Clang)

To run the tests:
$ cd test/LowRISCTaggedMemory
$ ./run-all-tests.sh

There are also Linux tests, but these may break at the moment (preserving tagged registers on interrupt etc isn't implemented yet):
$ ./run-all-linux-tests.sh
(This uses Spike and the run_test_linux_spike.sh script)

The Clang submodule has some other obsolete branches:
- Tagging virtual table pointers in Clang.
-- This only works for C++. 
-- Implementing it in the front-end allows detecting vtable pointers reliably, but is higher maintenance costs.
-- Currently malloc()/free() don't clear tags.
-- For C++ code that doesn't do any custom memory management, this might be interesting.
- An -fsanitize= pass.
-- At present this segfaults, it looks like this might be due to upstream (riscv-llvm) bugs.

Using LLVM with code pointer tagging
------------------------------------

At present you need to run Clang, opt, llc and gcc separately. The tests provide good examples of how to script this.
One advantage is you can easily specify exactly what passes you want, and inspect generated IR code.
In principle the front-end should run this all automatically, but this requires some configuration hacking.

$ cd test/LowRISCTaggedMemory/CTests
# Have a look at build.sh, or ...
$ INCLUDES="-I. -I $RISCV/riscv64-unknown-elf/include/"
$ main=main-union.c
$ clang -O0 -target riscv -mcpu=LowRISC -mriscv=LowRISC $INCLUDES -S $main -emit-llvm -o ${main}.ll
# Have a look at the LLVM IR
$ less ${main}.ll
# Run the sanitize pass. You can add -O2 before -tag-code-pointers if you want.
$ opt -load $TOP/riscv-llvm/build/Debug+Asserts/lib/LLVMTagCodePointers.so -tag-code-pointers < ${main}.ll > ${main}.opt.bc
# Disassemble the LLVM bitcode back into IR
$ llvm-dis ${main}.opt.bc
# Now have a look at the augmented IR
# Note that we add initialisation functions and change loads and stores of sensitive pointers.
$ diff -ur ${main}.ll ${main}.opt.ll
# Now compile it to assembler
$ llc -use-init-array -filetype=asm -march=riscv -mcpu=LowRISC ${main}.opt.bc -o ${main}.opt.s
# Also compile the un-augmented version.
$ llc -use-init-array -filetype=asm -march=riscv -mcpu=LowRISC ${main}.ll -o ${main}.s
# View the assembler diffs. Note new instructions LDCT, SDCT, WRT and RDT.
# Note that *both* the generated assembler versions include function prologue spilling protection (at the start of any function).
$ diff -u ${main}.s ${main}.opt.s
$ less ${main}.opt.s
# Now compile it to an executable
$ riscv64-unknown-elf-gcc -o test-${main}.riscv *.s
# And run it in Spike
$ spike pk test-${main}.riscv

Configuration options
---------------------

At present all the configuration is in the source.
See the top of lib/Transforms/TagCodePointers/TagCodePointers.cpp.
These should be compile time options, but that requires fixing the 
-fsanitize=lowrisc-tag-code-pointers pass first.

How to use the LLVM frontend (currently doesn't include code pointer tagging)
-----------------------------------------------------------------------------

$ export PATH=$TOP/riscv/bin/:$PATH
$ ln -s $TOP/riscv/bin/riscv64-unknown-linux-gnu-g++ $TOP/riscv/bin/g++
$ ln -s $TOP/riscv/bin/riscv64-unknown-linux-gnu-gcc $TOP/riscv/bin/gcc
# Now try to compile something with a standard ./configure setup...
$ CFLAGS="-target riscv -march=RV64I" CPPFLAGS="-fno-exceptions" CC=clang ./configure --host=riscv64-unknown-linux-gnu

Currently this will include stack protection but not code pointer protection. Debugging the -fsanitize=lowrisc-tag-code-pointers pass (clang branch tag-code-pointers-pass-fsanitize) would fix this.

Good luck!

