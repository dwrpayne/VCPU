
* Master is always in a releasable state. Dev is likely working but possibly unstable, while feature branches (if they exist) are in-progress work.*

Virtual CPU
---------

VCPU is a homebrew CPU built entirely in software that simulates a hardware CPU. It consists of software logic gates, wired together in building blocks of increasing complexity. The two major types of building blocks are `Component` and `Wire`. 

Wire
------

A `Wire` is a wrapper around a boolean, and is used by `Component` to propagate state changes throughout the system. It has `Set()` and `On()` functions that do exactly what you expect.

You can think of these sort of like the pin-ins and pin-outs on a physical chip.

Component
----------

A `Component` is the base class for all elements of the VCPU, from logic gates to registers to the ALU to the opcode decoder. It generally owns all its subcomponents, and implements two primary methods:
* `Connect()`, which wires together the subcomponents by connecting their inputs and outputs appropriately.
* `Update()`, which calls the `Update()` function of its subcomponents in the correct order to propagate signal flow along the internal `Wire`-ing.

* AndGate and Inverter are special cases - they actually own their output `Wire`s as member variables, and modify `Wire` state in their `Update()`. All other components only pass references around.

Philosophy
----------

* `Update()` is called in such a way to propagate the "leading edge" of the data around the chip. In a sense, all components are synchronous as they store state.
* `Update()` is always called unconditionally by every `Component`. Conditionally updating subcomponents based on the state of a `Wire` is cheating. No `Update()` can ever have an if statement in it. That decision must be implemented in "hardware".

Current Feature Set
------------

* 32-bit words, 32-bit fixed instruction size
* 32 general purpose registers.
* 5-stage RISC pipeline (IF, ID, EX, MEM, WB). Pipeline stages are stepped concurrently by `CPU::Update()`.
* 2K bytes of instruction memory, 128 bytes of L1 instruction cache
* 16K of main memory, 128 bytes of L1 data cache.
* Memory is all byte-addressable.
* Runs at ~1.5Khz on my desktop.
* Mostly full implementation of the MIPS I instruction set, except division.
* Single branch delay slot.
* Circuitry handles load stalls and cache miss stalls, no need for the assembler to handle these.
* Single-cycle step debugger with memory, register, and pipeline state output.
* Asynchronous system bus to communicate with memory and mem-mapped IO keyboard and terminal.
* Simple library of assembly functions.
* Full test suite

Future Work
---------------
* Exception Handling
* Interrupts
* Virtual Memory with TLB
* L2 cache
* Kernel mode / User mode
