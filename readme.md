Virtual CPU
---------

VCPU is a homebrew CPU implementing the MIPS I ISA, built entirely in software. It consists of software logic gates, wired together in building blocks of ever-increasing complexity. The two major types of building blocks are `Component` and `Wire`. 

Wire
------

A `Wire` is a wrapper around a boolean, and is used by `Component` to propagate state changes throughout the system. It has `Set()` and `On()` functions that do exactly what you expect.

You can think of these sort of like the pin-ins and pin-outs on a physical chip.

Component
----------

A `Component` is any building block, from a logic gate to a register to an ALU to the opcode decoder. It owns all its subcomponents, and implements two primary methods:
* `Connect()`, which wires together the subcomponents, connecting their inputs and outputs.
* `Update()`, which merely calls the `Update()` function of its subcomponents in the correct order to propagate signal flow along the internal `Wire`-ing.

It also has a getter method which gets some number of output `Wire`s or collections thereof, which vary by `Component`.

* AndGate and Inverter are special cases - they actually own their output `Wire`s as member variables, and modify state in their `Update()`. Every other component only ever deals with `Wire&`s.

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
* Full implementation of the MIPS I instruction set (multiplication and division not yet supported).
* Single branch delay slot.
* Circuitry handles load stalls and cache miss stalls, no need for the assembler to handle these.
