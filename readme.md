PS3Player - An open source playstation 3 emulator.

DISCLAIMER! THIS PROJECT IS NOT COMPLETE OR GUARANTEED TO WORK.
THE AUTHOR IS NOT RESPONSIBLE FOR ANY ISSUES RELATING TO USE

Currently working:
ISO Disk Decoding and Streaming.
SELF -> ELF Decoding using provided ps3 keys.
Instruction decoding and some implementations.
Full dissassembly for ppu elf.

How it works:
Provide a .iso game with a seperate .key file containing the the binary or hex key.
For system keys, a .key file is needed using the following layout:

type (app, seven, npdrm), revisiion, erk, riv.

Add any permutations your games might need, errors looding encrypted files are most likely to come from here.
System keys are not provided with this project, though they can easily be found on the ps3 dev wiki.
