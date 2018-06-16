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

* VCPU is unclocked, but the `Update()` functions are called in such a way to propagate the "leading edge" of the data around the chip.
* Update() is always called unconditionally by every `Component`. Conditionally updating subcomponents based on the state of a `Wire` is cheating. Those decisions must be implemented in "hardware".

Feature Set
------------

* 32-bit words, 32-bit fixed instruction size
* 32 general purpose registers.
* 5-stage RISC pipeline (IF, ID, EX, MEM, WB). Pipeline stages are stepped concurrently by `CPU::Update()`.
* 512 bytes of instruction memory, 64bytes of instruction cache
* 8K of main memory, 512bytes of L1 cache.
* Memory is all byte-addressable.
* Runs at ~6Khz on my desktop.
* Planned full implementation of the MIPS I instruction set (division not yet supported).
* Single branch delay slot.
* Circuitry handles load stalls and cache miss stalls, no need for the assembler to handle these.
* Single-cycle step debugger with memory, register, and pipeline state output.
