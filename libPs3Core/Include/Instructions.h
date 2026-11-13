#pragma once
#include <libPs3Core/Include/LibCore.h>

//The source:
//https://math-atlas.sourceforge.net/devel/assembly/ppc_isa.pdf
//Was used for the following things:
//InstForm, InstFieldBits (including bit descriptions)

enum class InstForm {
	Undefined = 0,
	I,
	B,
	SC,
	D,
	DS,
	X,
	XL,
	XFX,
	XFL,
	XS,
	XO,
	A,
	M,
	MD,
	MDS,
};

enum class InstOp {
	OpNone = 0,
	Tdi,
	Twi,
	Mulli,
	Subfic,
	Dozi,
	Cmpli,
	Cmpi,
	Addic,
	Addicd,
	Addi,
	Addis,
};

struct Instruction {
	uint32_t	opcode{};

	uint16_t ui{};
	int16_t si{};
	uint8_t rt{ 0 }, ra{ 0 }, rb{ 0 }, rs{ 0 };
	bool aa{ false }, lk{ false };
};

CORE_API bool LcDecodeInstruction(
	uint32_t raw, 
	Instruction& inst);

namespace InstFieldBits {
constexpr inline uint32_t GetBits(uint32_t data, int first, int last) {
	int width{ last - first + 1 };
	uint32_t mask{ (width == 32) ? 0xFFFFFFFFu : ((1u << width) - 1) };
	return (data >> (31 - last)) & mask;
}

//AA (30)
//	Absolute Address bit.
//
//	0 The immediate field represents an address
//		relative to the current instruction address.
//		For I-form branches the effective address of
//		the branch target is the sum of the LI field
//		sign-extended to 64 bits and the address of
//		the branch instruction. For B-form branches
//		the effective address of the branch target is
//		the sum of the BD field sign-extended to 64
//		bits and the address of the branch instruction.
//	1 The immediate field represents an absolute
//		address. For I-form branches the effective
//		address of the branch target is the LI field
//		sign-extended to 64 bits. For B-form
//		branches the effective address of the branch
//		target is the BD field sign-extended to 64
//		bits.
constexpr inline bool AA(uint32_t inst) {
	return (inst >> 1) & 0x1;
}

//BA(11:15)
//	Field used to specify a bit in the CR to be used as
//	a source.
constexpr inline uint32_t BA(uint32_t inst) {
	return (inst >> 16) & 0x1F;
}

//BB(16:20)
//	Field used to specify a bit in the CR to be used as
//	a source.
constexpr inline uint32_t BB(uint32_t inst) {
	return (inst >> 11) & 0x1F;
}

//BD(16:29)
//	Immediate field used to specify a 14 - bit signed
//	two's complement branch displacement which is
//	concatenated on the right with 0b00 and signextended to 64 bits.
constexpr inline int32_t BD(uint32_t inst) {
	return static_cast<int32_t>(static_cast<int16_t>(inst & 0xFFFC));
}

//BF(6:8)
//	Field used to specify one of the CR fields or one
//	of the FPSCR fields to be used as a target.
constexpr inline uint32_t BF(uint32_t inst) {
	return (inst >> 23) & 0x7;
}

//BFA(11:13)
//	Field used to specify one of the CR fields or one
//	of the FPSCR fields to be used as a source.
constexpr inline uint32_t BFA(uint32_t inst) {
	return (inst >> 18) & 0x7;
}

//BH(19:20)
//	Field used to specify a hint in the Branch Conditional 
//	to Link Register and Branch Conditional to
//	Count Register instructions.The encoding is
//	described in Section 2.4.1, “Branch Instructions”
//	on page 20.
constexpr inline uint32_t BH(uint32_t inst) {
	return (inst >> 11) & 0x3;
}

//BI(11:15)
//	Field used to specify a bit in the CR to be tested
//	by a Branch Conditional instruction.
constexpr inline uint32_t BI(uint32_t inst) {
	return (inst >> 16) & 0x1F;
}

//BO(6:10)
//	Field used to specify options for the Branch Conditional instructions.
//	The encoding is described in
//	Section 2.4.1, “Branch Instructions” on page 20.
constexpr inline uint32_t BO(uint32_t inst) {
	return (inst >> 21) & 0x1F;
}

//BT(6:10)
//	Field used to specify a bit in the CR or in the
//	FPSCR to be used as a target.
constexpr inline uint32_t BT(uint32_t inst) {
	return (inst >> 21) & 0x1F;
}

//D(16:31)
//	Immediate field used to specify a 16 - bit signed
//	two's complement integer which is sign-extended
//	to 64 bits.
constexpr inline int16_t D(uint32_t inst) {
	return static_cast<int16_t>(inst & 0xFFFF);
}

//DS(16:29)
//	Immediate field used to specify a 14 - bit signed
//	two's complement integer which is concatenated
//	on the right with 0b00 and sign - extended to 64
//	bits.
constexpr inline int32_t DS(uint32_t inst) {
	return static_cast<int32_t>(static_cast<int16_t>(inst & 0xFFFC));
}

//FLM(7:14)
//	Field mask used to identify the FPSCR fields that
//	are to be updated by the mtfsf instruction.
constexpr inline uint32_t FLM(uint32_t inst) {
	return (inst >> 17) & 0xFF;
}

//FRA(11:15)
//	Field used to specify an FPR to be used as a
//	source.
constexpr inline uint32_t FRA(uint32_t inst) {
	return (inst >> 16) & 0x1F;
}

//FRB(16:20)
//	Field used to specify an FPR to be used as a
//	source.
constexpr inline uint32_t FRB(uint32_t inst) {
	return (inst >> 11) & 0x1F;
}

//FRC(21:25)
//	Field used to specify an FPR to be used as a
//	source.
constexpr inline uint32_t FRC(uint32_t inst) {
	return (inst >> 6) & 0x1F;
}

//FRS(6:10)
//	Field used to specify an FPR to be used as a
//	source.
constexpr inline uint32_t FRS(uint32_t inst) {
	return (inst >> 21) & 0x1F;
}

//FRT(6:10)
//	Field used to specify an FPR to be used as a
//	target.
constexpr inline uint32_t FRT(uint32_t inst) {
	return (inst >> 21) & 0x1F;
}

//FXM(12:19)
//	Field mask used to identify the CR fields that are
//	to be written by the mtcrf and mtocrf instructions,
//	or read by the mfocrf instruction.
constexpr inline uint32_t FXM(uint32_t inst) {
	return (inst >> 12) & 0xFF;
}

//L(10)
//	Field used to specify whether a fixed - point
//	Compare instruction is to compare 64 - bit
//	numbers or 32 - bit numbers.
constexpr inline uint32_t L10(uint32_t inst) {
	return (inst >> 21) & 0x1;
}

//L(15)
//	Field used by the Move To Machine State Register and TLB Invalidate Entry instructions(see
//		Book III, PowerPC Operating Environment Architecture).
constexpr inline uint32_t L15(uint32_t inst) {
	return (inst >> 16) & 0x1;
}

//L(9:10)
//	Field used by the Synchronize instruction(see
//		Book II, PowerPC Virtual Environment Architecture).
constexpr inline uint32_t LSYNC(uint32_t inst) {
	return (inst >> 21) & 0x3;
}

//LEV(20:26)
//	Field used by the System Call instruction.
constexpr inline uint32_t LEV(uint32_t inst) {
	return (inst >> 5) & 0x7F;
}

//LI(6:29)
//	Immediate field used to specify a 24 - bit signed
//	two's complement integer which is concatenated
//	on the right with 0b00 and sign - extended to 64
//	bits.
constexpr inline int32_t LI(uint32_t inst) {
	return static_cast<int32_t>(inst & 0x03FFFFFC) << 6 >> 6;
}

//LK(31)
//	LINK bit.
//	0 Do not set the Link Register.
//	1 Set the Link Register. The address of the
//		instruction following the Branch instruction is
//		placed into the Link Register.
constexpr inline uint32_t LK(uint32_t inst) {
	return inst & 0x1;
}

//MB (21:25) and ME (26:30)
//	Fields used in M - form instructions to specify a
//	64 - bit mask consisting of 1 - bits from bit MB + 32
//	through bit ME + 32 inclusive and 0 - bits elsewhere, as described in Section 3.3.12, “FixedPoint Rotate and Shift Instructions” on page 68.
constexpr inline uint32_t MBM(uint32_t inst) {
	return (inst >> 6) & 0x1F;
}

constexpr inline uint32_t MEM(uint32_t inst) {
	return (inst >> 1) & 0x1F;
}

//MB(21:26)
//	Field used in MD - form and MDS - form instructions
//	to specify the first 1 - bit of a 64 - bit mask, as
//	described in Section 3.3.12, “Fixed - Point Rotate
//	and Shift Instructions” on page 68.
constexpr inline uint32_t MB(uint32_t inst) {
	uint32_t f{ (inst >> 5) & 0x3F };
	return ((f & 1) << 5) | (f >> 1);
}

//ME(21:26)
//	Field used in MD - form and MDS - form instructions
//	to specify the last 1 - bit of a 64 - bit mask, as
//	described in Section 3.3.12, “Fixed - Point Rotate
//	and Shift Instructions” on page 68.
constexpr inline uint32_t ME(uint32_t inst) {
	uint32_t f{ (inst >> 5) & 0x3F };
	return ((f & 1) << 5) | (f >> 1);
}

//NB(16:20)
//	Field used to specify the number of bytes to
//	move in an immediate Move Assist instruction.
constexpr inline uint32_t NB(uint32_t inst) {
	return (inst >> 11) & 0x1F;
}

//OPCD(0:5)
//	Primary opcode field.
constexpr inline uint32_t OPCD(uint32_t inst) {
	return inst >> 26;
}

//OE(21)
//	Field used by XO - form instructions to enable
//	setting OV and SO in the XER.
constexpr inline uint32_t OE(uint32_t inst) {
	return (inst >> 10) & 0x1;
}

//RA(11:15)
//	Field used to specify a GPR to be used as a
//	source or as a target.
constexpr inline uint32_t RA(uint32_t inst) {
	return (inst >> 16) & 0x1F;
}

//RB(16:20)
//	Field used to specify a GPR to be used as a
//	source.
constexpr inline uint32_t RB(uint32_t inst) {
	return (inst >> 11) & 0x1F;
}

//Rc(31)
//	RECORD bit.
//	0 Do not alter the Condition Register.
//	1 Set Condition Register Field 0 or Field 1 as
//	described in Section 2.3.1, “Condition
//	Register” on page 18.
constexpr inline uint32_t Rc(uint32_t inst) {
	return inst & 0x1;
}

//RS(6:10)
//	Field used to specify a GPR to be used as a
//	source.
constexpr inline uint32_t RS(uint32_t inst) {
	return (inst >> 21) & 0x1F;
}

//RT(6:10)
//	Field used to specify a GPR to be used as a
//	target.
constexpr inline uint32_t RT(uint32_t inst) {
	return (inst >> 21) & 0x1F;
}

//SH(16:20, or 16 : 20 and 30)
//	Field used to specify a shift amount.
constexpr inline uint32_t SH32(uint32_t inst) {
	return (inst >> 11) & 0x1F;
}

constexpr inline uint32_t SH64(uint32_t inst) {
	return (((inst >> 1) & 0x1) << 5) | ((inst >> 11) & 0x1F);
}

//SI(16:31)
//	Immediate field used to specify a 16 - bit signed
//	integer.
constexpr inline int16_t SI(uint32_t inst) {
	return static_cast<int16_t>(inst & 0xFFFF);
}

//SPR(11:20)
//	Field used to specify a Special Purpose Register
//	for the mtspr and mfspr instructions.
constexpr inline uint32_t SPR(uint32_t inst) {
	uint32_t f{ (inst >> 11) & 0x3FF };
	return ((f & 0x1F) << 5) | (f >> 5);
}

//SR(12:15)
//	Field used by the Segment Register Manipulation
//	instructions(see Book III, PowerPC Operating
//	Environment Architecture).
constexpr inline uint32_t SR(uint32_t inst) {
	return (inst >> 16) & 0xF;
}

//TBR(11:20)
//	Field used by the Move From Time Base instruction(
//	see Book II, PowerPC Virtual Environment Architecture).
constexpr inline uint32_t TBR(uint32_t inst) {
	uint32_t f{ (inst >> 11) & 0x3FF };
	return ((f & 0x1F) << 5) | (f >> 5);
}

//TH(9:10)
//	Field used by the optional data stream variant of
//	the dcbt instruction(see Book II, PowerPC Virtual
//		Environment Architecture).
constexpr inline uint32_t TH(uint32_t inst) {
	return (inst >> 21) & 0x3;
}

//TO(6:10)
//	Field used to specify the conditions on which to
//	trap.The encoding is described in Section 3.3.10,
//	“Fixed - Point Trap Instructions” on page 60.
constexpr inline uint32_t TO(uint32_t inst) {
	return (inst >> 21) & 0x1F;
}

//U(16:19)
//	Immediate field used as the data to be placed
//	into a field in the FPSCR.
constexpr inline uint32_t U(uint32_t inst) {
	return (inst >> 12) & 0xF;
}

//UI(16:31)
//	Immediate field used to specify a 16 - bit unsigned
//	integer.
constexpr inline uint16_t UI(uint32_t inst) {
	return inst & 0xFFFF;
}
};
