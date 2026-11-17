#pragma once
#include <libPs3Core/Include/BitTypes.h>

//The source:
//https://math-atlas.sourceforge.net/devel/assembly/ppc_isa.pdf
//Was used for the following things:
//InstFieldBits (including bit descriptions)

template<uint32_t First, uint32_t Last, typename T, typename Storage = uint32_t>
using PpcBits = BitField<(sizeof(Storage) * 8 - 1) - Last, Last - First + 1, T, Storage>;

union InstField {
	uint32_t raw;

	// ---- Primary opcode -----------------------------------------------
	PpcBits<0, 5, uint32_t>		opcd;

	// ---- Register fields ------------------------------------------------
	PpcBits<6, 10, uint32_t>	rd;      // also: rs
	PpcBits<6, 10, uint32_t>	rs;
	PpcBits<6, 10, uint32_t>	frd;
	PpcBits<6, 10, uint32_t>	frs;
	PpcBits<6, 10, uint32_t>	vd;
	PpcBits<6, 10, uint32_t>	vs;

	PpcBits<11, 15, uint32_t>	ra;
	PpcBits<11, 15, uint32_t>	fra;
	PpcBits<11, 15, uint32_t>	va;

	PpcBits<16, 20, uint32_t>	rb;
	PpcBits<16, 20, uint32_t>	frb;
	PpcBits<16, 20, uint32_t>	vb;

	PpcBits<21, 25, uint32_t>	frc;     // A-form third source
	PpcBits<21, 25, uint32_t>	vc;      // VA-form third source

	// ---- Immediates -------------------------------------------------------
	PpcBits<16, 31, uint32_t>	uimm;
	PpcBits<16, 31, int32_t>	simm;
	PpcBits<16, 29, int32_t>	ds;      // DS-form displacement (word offset, see DispDS)
	PpcBits<30, 31, uint32_t>	dsXo;    // DS-form extended opcode (ld/ldu/lwa, std/stdu)

	// ---- Branch fields ----------------------------------------------------
	PpcBits<6, 29, int32_t>		li;      // I-form  (b)
	PpcBits<16, 29, int32_t>	bd;      // B-form  (bc)
	PpcBits<30, 30, bool>		aa;
	PpcBits<31, 31, bool>		lk;
	PpcBits<6, 10, uint32_t>	bo;
	PpcBits<11, 15, uint32_t>	bi;
	PpcBits<19, 20, uint32_t>	bh;      // branch hint (bclr/bcctr)

	// ---- Condition register -------------------------------------------------
	PpcBits<6, 8, uint32_t>		crfd;    // BF
	PpcBits<11, 13, uint32_t>	crfs;    // BFA
	PpcBits<6, 10, uint32_t>	crbd;    // BT
	PpcBits<11, 15, uint32_t>	crba;    // BA
	PpcBits<16, 20, uint32_t>	crbb;    // BB
	PpcBits<12, 19, uint32_t>	crm;     // mtcrf field mask
	PpcBits<7, 14, uint32_t>	fm;      // mtfsf field mask
	PpcBits<20, 23, uint32_t>	imm4;    // mtfsfi immediate (U)

	// ---- L / compare / misc flag bits ---------------------------------------
	PpcBits<10, 10, bool>		l;       // cmp, cmpi, cmpl, cmpli: 0 = 32-bit, 1 = 64-bit
	PpcBits<9, 10, uint32_t>	lSync;   // sync (ISA 2.0 two-bit L)
	PpcBits<15, 15, bool>		lMsr;    // mtmsr / mtmsrd
	PpcBits<6, 10, uint32_t>	to;      // tw / td / twi / tdi trap conditions
	PpcBits<12, 15, uint32_t>	sr;      // mfsr / mtsr
	PpcBits<16, 20, uint32_t>	nb;      // lswi / stswi byte count
	PpcBits<20, 26, uint32_t>	lev;     // sc

	// ---- Rc / OE ------------------------------------------------------------
	PpcBits<31, 31, bool>		rc;
	PpcBits<21, 21, bool>		oe;

	// ---- Shift / rotate (M-form: rlwinm, rlwimi, rlwnm) ------------------------
	PpcBits<16, 20, uint32_t>	sh;
	PpcBits<21, 25, uint32_t>	mb;
	PpcBits<26, 30, uint32_t>	me;

	// ---- MD / MDS / XS-form (rld*, sradi) ----------------------------------------
	PpcBits<16, 20, uint32_t>	shLo;
	PpcBits<30, 30, uint32_t>	shHi;
	PpcBits<21, 26, uint32_t>	mbRaw;   // stored rotated, see Mb64()
	PpcBits<21, 26, uint32_t>	meRaw;   // same bits, see Me64()

	// ---- SPR / TBR (halves swapped) ----------------------------------------------
	PpcBits<11, 20, uint32_t>	sprRaw;

	// ---- Extended opcodes -----------------------------------------------------------
	PpcBits<21, 30, uint32_t>	xo10;    // X-form, XL-form
	PpcBits<22, 30, uint32_t>	xo9;     // XO-form
	PpcBits<26, 30, uint32_t>	xo5;     // A-form
	PpcBits<27, 29, uint32_t>	xoMd;    // MD-form
	PpcBits<27, 30, uint32_t>	xoMds;   // MDS-form
	PpcBits<21, 29, uint32_t>	xoXs;    // XS-form
	PpcBits<30, 31, uint32_t>	xoDs;    // DS-form

	// ---- VMX (AltiVec) --------------------------------------------------------------
	PpcBits<11, 15, uint32_t>	vuimm;
	PpcBits<11, 15, int32_t>	vsimm;
	PpcBits<22, 25, uint32_t>	vsh;     // vsldoi
	PpcBits<21, 31, uint32_t>	vxo;     // VX-form
	PpcBits<26, 31, uint32_t>	vaXo;    // VA-form
	PpcBits<22, 31, uint32_t>	vcXo;    // VC-form (compares)
	PpcBits<21, 21, bool>		vrc;     // VC-form record bit

	InstField() = default;
	constexpr InstField(uint32_t v) noexcept : raw{ v } {}

	int32_t DispBD() const noexcept { return bd.Get() * 4; }
	int32_t DispLI() const noexcept { return li.Get() * 4; }
	int32_t DispDS() const noexcept { return ds.Get() * 4; }

	uint32_t BranchTargetBD(uint32_t cia) const noexcept {
		return aa ? static_cast<uint32_t>(DispBD()) : cia + static_cast<uint32_t>(DispBD());
	}
	uint32_t BranchTargetLI(uint32_t cia) const noexcept {
		return aa ? static_cast<uint32_t>(DispLI()) : cia + static_cast<uint32_t>(DispLI());
	}

	uint32_t Spr() const noexcept {
		const uint32_t f{ sprRaw.Get() };
		return ((f & 0x1F) << 5) | (f >> 5);
	}

	uint32_t Sh64() const noexcept { return (shHi.Get() << 5) | shLo.Get(); }

	uint32_t Mb64() const noexcept {
		const uint32_t f{ mbRaw.Get() };
		return ((f & 1) << 5) | (f >> 1);
	}

	uint32_t Me64() const noexcept { return Mb64(); }   // same encoding, different instruction
};

static_assert(sizeof(InstField) == 4);
static_assert(std::is_trivially_copyable_v<InstField>);

enum class InstMnemonic {
	None = 0,
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
	Bc,
	Sc,
	B,
	Rlwimi,
	Rlwinm,
	Rlmi,
	Rlwnm,
	Ori,
	Oris,
	Xori,
	Xoris,
	Andi,
	Andis,
	Lwz,
	Lwzu,
	Lbz,
	Lbzu,
	Stw,
	Stwu,
	Stb,
	Stbu,
	Lhz,
	Lhzu,
	Lha,
	Lhau,
	Sth,
	Sthu,
	Lmw,
	Stmw,
	Lfs,
	Lfsu,
	Lfd,
	Lfdu,
	Stfs,
	Stfsu,
	Stfd,
	Stfdu,
	Std,
	Stdu,
	Ba,
	Bl,
	Bla,
};

enum class InstOperand : uint8_t {
	None = 0,
	// GPR
	RD, RS, RA, RA0, RB,
	// FPR / VR
	FRD, FRS, FRA, FRB, FRC,
	VD, VS, VA, VB, VC,
	// Immediates
	SIMM, UIMM, DS, VSIMM, VUIMM, VSH, NB, SH, SH64, MB, ME, MB64, ME64,
	// Branch / CR
	BO, BI, BH, BD, LI, CRFD, CRFS, CRBD, CRBA, CRBB, CRM, FM,
	// Misc
	L, TO, SPR, SR, LEV,
};

struct InstData {
	InstMnemonic	mnemonic{};
	InstOperand		operands[5]{};
};

CORE_API bool LcDecodeInstruction(
	const InstField& instField,	//In
	InstData& inst);			//Out
