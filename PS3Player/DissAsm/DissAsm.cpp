#include <PS3Player/DissAsm/DissAsm.h>

#include <string>
#include <format>
#include <sstream>
#include <iomanip>

constexpr static const char* gMnemonicStrs[]{
	"none",

	// ---- Primary opcodes (OPCD only, in opcode order) ----
	"tdi",		// 2
	"twi",		// 3
	"mulli",	// 7
	"subfic",	// 8
	"cmpli",	// 10
	"cmpi",		// 11
	"addic",	// 12
	"addicd",	// 13 (addic.)
	"addi",		// 14
	"addis",	// 15
	"bc",		// 16 (bc/bca/bcl/bcla)
	"sc",		// 17
	"b",		// 18 (b/ba/bl/bla)
	"rlwimi",	// 20
	"rlwinm",	// 21
	"rlwnm",	// 23
	"ori",		// 24
	"oris",		// 25
	"xori",		// 26
	"xoris",	// 27
	"andi",		// 28 (andi.)
	"andis",	// 29 (andis.)
	"lwz",		// 32
	"lwzu",		// 33
	"lbz",		// 34
	"lbzu",		// 35
	"stw",		// 36
	"stwu",		// 37
	"stb",		// 38
	"stbu",		// 39
	"lhz",		// 40
	"lhzu",		// 41
	"lha",		// 42
	"lhau",		// 43
	"sth",		// 44
	"sthu",		// 45
	"lmw",		// 46
	"stmw",		// 47
	"lfs",		// 48
	"lfsu",		// 49
	"lfd",		// 50
	"lfdu",		// 51
	"stfs",		// 52
	"stfsu",	// 53
	"stfd",		// 54
	"stfdu",	// 55
	"ld",		// 58, XO 0
	"ldu",		// 58, XO 1
	"lwa",		// 58, XO 2
	"std",		// 62, XO 0
	"stdu",		// 62, XO 1

	// ---- Opcode 19: CR ops / branch to register (XL-form) ----
	"mcrf",
	"bclr",		// bclr/bclrl
	"crnor",
	"crandc",
	"crxor",
	"crnand",
	"crand",
	"creqv",
	"crorc",
	"cror",
	"bcctr",	// bcctr/bcctrl

	// ---- Opcode 30: 64-bit rotates (MD/MDS-form) ----
	"rldicl",
	"rldicr",
	"rldic",
	"rldimi",
	"rldcl",
	"rldcr",

	// ---- Opcode 31: compare / trap ----
	"cmp",
	"cmpl",
	"tw",
	"td",

	// ---- Opcode 31: integer loads ----
	"lbzx",
	"lbzux",
	"lhzx",
	"lhzux",
	"lhax",
	"lhaux",
	"lwzx",
	"lwzux",
	"lwax",
	"lwaux",
	"ldx",
	"ldux",

	// ---- Opcode 31: integer stores ----
	"stbx",
	"stbux",
	"sthx",
	"sthux",
	"stwx",
	"stwux",
	"stdx",
	"stdux",

	// ---- Opcode 31: byte-reversed loads/stores ----
	"lhbrx",
	"lwbrx",
	"sthbrx",
	"stwbrx",

	// ---- Opcode 31: move assist (string) ----
	"lswi",
	"lswx",
	"stswi",
	"stswx",

	// ---- Opcode 31: arithmetic (XO-form, OE/Rc merged) ----
	"add",
	"addc",
	"adde",
	"addme",
	"addze",
	"subf",
	"subfc",
	"subfe",
	"subfme",
	"subfze",
	"neg",
	"mullw",
	"mulld",
	"mulhw",
	"mulhwu",
	"mulhd",
	"mulhdu",
	"divw",
	"divwu",
	"divd",
	"divdu",

	// ---- Opcode 31: logical ----
	"and",
	"andc",
	"or",
	"orc",
	"xor",
	"nand",
	"nor",
	"eqv",
	"extsb",
	"extsh",
	"extsw",
	"cntlzw",
	"cntlzd",

	// ---- Opcode 31: shifts ----
	"slw",
	"srw",
	"sraw",
	"srawi",
	"sld",
	"srd",
	"srad",
	"sradi",

	// ---- Opcode 31: move to/from system registers ----
	"mfspr",
	"mtspr",
	"mfcr",
	"mtcrf",
	"mfocrf",		// optional (ch. 5)
	"mtocrf",		// optional (ch. 5)
	"mcrxr",		// phased out (ch. 6)

	// ---- Opcode 31: floating-point loads/stores (indexed) ----
	"lfsx",
	"lfsux",
	"lfdx",
	"lfdux",
	"stfsx",
	"stfsux",
	"stfdx",
	"stfdux",
	"stfiwx",

	// ---- Opcode 59: single-precision FP ----
	"fdivs",
	"fsubs",
	"fadds",
	"fsqrts",		// optional
	"fres",			// optional
	"fmuls",
	"fmsubs",
	"fmadds",
	"fnmsubs",
	"fnmadds",

	// ---- Opcode 63: double-precision FP ----
	"fcmpu",
	"fcmpo",
	"frsp",
	"fctiw",
	"fctiwz",
	"fctid",
	"fctidz",
	"fcfid",
	"fdiv",
	"fsub",
	"fadd",
	"fsqrt",		// optional
	"fsel",			// optional
	"fmul",
	"frsqrte",		// optional
	"fmsub",
	"fmadd",
	"fnmsub",
	"fnmadd",
	"fmr",
	"fneg",
	"fabs",
	"fnabs",

	// ---- Opcode 63: FPSCR ----
	"mffs",
	"mcrfs",
	"mtfsfi",
	"mtfsf",
	"mtfsb0",
	"mtfsb1",
};

static std::string
reg(char prefix, uint32_t n) {
	return std::string{ prefix } + std::to_string(n);
}

static std::string
num(int64_t v) {
	return std::to_string(v);
}

static std::string
hex(int64_t v) {
	std::stringstream stream;
	stream << "$"
		<< std::setfill('0') << std::setw(sizeof(int64_t) * 2)
		<< std::hex << v;
	return stream.str();
}

static bool
isRegOperand(InstOperand op) {
	return op >= InstOperand::RD && op <= InstOperand::VC;
}

static std::string
formatOperand(InstOperand op, const InstField& f) {
	switch (op) {
	case InstOperand::RD:   return reg('R', f.rd.Get());
	case InstOperand::RS:   return reg('R', f.rs.Get());
	case InstOperand::RA:   return reg('R', f.ra.Get());
	case InstOperand::RA0:  return f.ra.Get() == 0 ? std::string{ "0" } : reg('R', f.ra.Get());
	case InstOperand::RB:   return reg('R', f.rb.Get());
	case InstOperand::FRD:  return reg('F', f.frd.Get());
	case InstOperand::FRS:  return reg('F', f.frs.Get());
	case InstOperand::FRA:  return reg('F', f.fra.Get());
	case InstOperand::FRB:  return reg('F', f.frb.Get());
	case InstOperand::FRC:  return reg('F', f.frc.Get());
	case InstOperand::VD:   return reg('V', f.vd.Get());
	case InstOperand::VS:   return reg('V', f.vs.Get());
	case InstOperand::VA:   return reg('V', f.va.Get());
	case InstOperand::VB:   return reg('V', f.vb.Get());
	case InstOperand::VC:   return reg('V', f.vc.Get());
	case InstOperand::SIMM:  return num(f.simm.Get());
	case InstOperand::UIMM:  return num(f.uimm.Get());
	case InstOperand::DS:    return num(f.DispDS());
	case InstOperand::VSIMM: return num(f.vsimm.Get());
	case InstOperand::VUIMM: return num(f.vuimm.Get());
	case InstOperand::VSH:   return num(f.vsh.Get());
	case InstOperand::NB:    return num(f.nb.Get());
	case InstOperand::SH:    return num(f.sh.Get());
	case InstOperand::SH64:  return num(f.Sh64());
	case InstOperand::MB:    return num(f.mb.Get());
	case InstOperand::ME:    return num(f.me.Get());
	case InstOperand::MB64:  return num(f.Mb64());
	case InstOperand::ME64:  return num(f.Me64());
	case InstOperand::BO:    return num(f.bo.Get());
	case InstOperand::BI:    return num(f.bi.Get());
	case InstOperand::BH:    return num(f.bh.Get());
	case InstOperand::BD:    return num(f.DispBD());
	case InstOperand::LI:    return hex(f.DispLI());
	case InstOperand::CRFD:  return "CR" + num(f.crfd.Get());
	case InstOperand::CRFS:  return "CR" + num(f.crfs.Get());
	case InstOperand::CRBD:  return num(f.crbd.Get());
	case InstOperand::CRBA:  return num(f.crba.Get());
	case InstOperand::CRBB:  return num(f.crbb.Get());
	case InstOperand::CRM:   return std::format("0x{:02X}", f.crm.Get());
	case InstOperand::FM:    return std::format("0x{:02X}", f.fm.Get());
	case InstOperand::L:     return num(f.l.Get());
	case InstOperand::TO:    return num(f.to.Get());
	case InstOperand::SPR:   return num(f.Spr());
	case InstOperand::SR:    return num(f.sr.Get());
	case InstOperand::LEV:   return num(f.lev.Get());
	default:                 return "?";
	}
}

static std::string
mnemonicName(InstMnemonic m) {
	return std::string{ gMnemonicStrs[static_cast<uint32_t>(m)] };
}

// ---- Rc / OE suffix handling ---------------------------------------------------------
// The mnemonic enum has one entry per instruction, so the Rc ('.') and OE ('o') bits have
// to be appended when printing.

// Instructions whose Rc is part of the opcode (always record): addic. andi. andis.
static bool
isAlwaysRecord(InstMnemonic m) {
	return m == InstMnemonic::Addicd || m == InstMnemonic::Andi || m == InstMnemonic::Andis;
}

// XO-form instructions that have an OE bit (bit 21)
static bool
hasOe(InstMnemonic m) {
	switch (m) {
	case InstMnemonic::Add:    case InstMnemonic::Addc:   case InstMnemonic::Adde:
	case InstMnemonic::Addme:  case InstMnemonic::Addze:
	case InstMnemonic::Subf:   case InstMnemonic::Subfc:  case InstMnemonic::Subfe:
	case InstMnemonic::Subfme: case InstMnemonic::Subfze: case InstMnemonic::Neg:
	case InstMnemonic::Mullw:  case InstMnemonic::Mulld:
	case InstMnemonic::Divw:   case InstMnemonic::Divwu:
	case InstMnemonic::Divd:   case InstMnemonic::Divdu:
		return true;
	default:
		return false;
	}
}

// Instructions that have an Rc bit (bit 31)
static bool
hasRc(InstMnemonic m) {
	if (hasOe(m)) return true;

	switch (m) {
		// rotates
	case InstMnemonic::Rlwimi: case InstMnemonic::Rlwinm: case InstMnemonic::Rlwnm:
	case InstMnemonic::Rldicl: case InstMnemonic::Rldicr: case InstMnemonic::Rldic:
	case InstMnemonic::Rldimi: case InstMnemonic::Rldcl:  case InstMnemonic::Rldcr:
		// high multiplies
	case InstMnemonic::Mulhw:  case InstMnemonic::Mulhwu:
	case InstMnemonic::Mulhd:  case InstMnemonic::Mulhdu:
		// logical
	case InstMnemonic::And:    case InstMnemonic::Andc:   case InstMnemonic::Or:
	case InstMnemonic::Orc:    case InstMnemonic::Xor:    case InstMnemonic::Nand:
	case InstMnemonic::Nor:    case InstMnemonic::Eqv:
	case InstMnemonic::Extsb:  case InstMnemonic::Extsh:  case InstMnemonic::Extsw:
	case InstMnemonic::Cntlzw: case InstMnemonic::Cntlzd:
		// shifts
	case InstMnemonic::Slw:    case InstMnemonic::Srw:    case InstMnemonic::Sraw:
	case InstMnemonic::Srawi:  case InstMnemonic::Sld:    case InstMnemonic::Srd:
	case InstMnemonic::Srad:   case InstMnemonic::Sradi:
		// FP single
	case InstMnemonic::Fdivs:  case InstMnemonic::Fsubs:  case InstMnemonic::Fadds:
	case InstMnemonic::Fsqrts: case InstMnemonic::Fres:   case InstMnemonic::Fmuls:
	case InstMnemonic::Fmsubs: case InstMnemonic::Fmadds: case InstMnemonic::Fnmsubs:
	case InstMnemonic::Fnmadds:
		// FP double
	case InstMnemonic::Frsp:   case InstMnemonic::Fctiw:  case InstMnemonic::Fctiwz:
	case InstMnemonic::Fctid:  case InstMnemonic::Fctidz: case InstMnemonic::Fcfid:
	case InstMnemonic::Fdiv:   case InstMnemonic::Fsub:   case InstMnemonic::Fadd:
	case InstMnemonic::Fsqrt:  case InstMnemonic::Fsel:   case InstMnemonic::Fmul:
	case InstMnemonic::Frsqrte:
	case InstMnemonic::Fmsub:  case InstMnemonic::Fmadd:  case InstMnemonic::Fnmsub:
	case InstMnemonic::Fnmadd:
	case InstMnemonic::Fmr:    case InstMnemonic::Fneg:   case InstMnemonic::Fabs:
	case InstMnemonic::Fnabs:
		// FPSCR
	case InstMnemonic::Mffs:   case InstMnemonic::Mtfsfi: case InstMnemonic::Mtfsf:
	case InstMnemonic::Mtfsb0: case InstMnemonic::Mtfsb1:
		return true;
	default:
		return false;
	}
}

// Mnemonic with the 'o' and '.' suffixes (e.g. "addco.")
static std::string
mnemonicFull(InstMnemonic m, const InstField& f) {
	std::string s{ mnemonicName(m) };

	if (isAlwaysRecord(m)) {
		if (s.empty() || s.back() != '.') s += '.';
		return s;
	}

	if (hasOe(m) && ((f.raw >> 10) & 1)) s += 'o';	// OE = instruction bit 21
	if (hasRc(m) && f.rc.Get())          s += '.';
	return s;
}

static std::string
branchTarget(bool aa, int32_t disp, uint32_t pc) {
	const uint32_t target{ aa ? static_cast<uint32_t>(disp) : pc + static_cast<uint32_t>(disp) };
	return std::format("0x{:08X}", target);
}

// ---- Loads / stores -----------------------------------------------------------------
// rD, d(rA). The update forms always print rA, the others print 0 when rA == 0.
static bool
formatMemory(const InstData& d, const InstField& f, std::string& out) {
	char prefix{ 'R' };
	bool update{ false };
	bool dsForm{ false };

	switch (d.mnemonic) {
	case InstMnemonic::Lwz:  case InstMnemonic::Lbz:  case InstMnemonic::Lhz:
	case InstMnemonic::Lha:  case InstMnemonic::Lmw:
	case InstMnemonic::Stw:  case InstMnemonic::Stb:  case InstMnemonic::Sth:
	case InstMnemonic::Stmw:
		break;

	case InstMnemonic::Lwzu: case InstMnemonic::Lbzu: case InstMnemonic::Lhzu:
	case InstMnemonic::Lhau:
	case InstMnemonic::Stwu: case InstMnemonic::Stbu: case InstMnemonic::Sthu:
		update = true;
		break;

	case InstMnemonic::Lfs:  case InstMnemonic::Lfd:
	case InstMnemonic::Stfs: case InstMnemonic::Stfd:
		prefix = 'F';
		break;

	case InstMnemonic::Lfsu: case InstMnemonic::Lfdu:
	case InstMnemonic::Stfsu: case InstMnemonic::Stfdu:
		prefix = 'F';
		update = true;
		break;

	case InstMnemonic::Ld:   case InstMnemonic::Lwa:  case InstMnemonic::Std:
		dsForm = true;
		break;

	case InstMnemonic::Ldu:  case InstMnemonic::Stdu:
		dsForm = true;
		update = true;
		break;

	default:
		return false;
	}

	const int32_t disp{ dsForm ? f.DispDS() : f.simm.Get() };
	const uint32_t ra{ f.ra.Get() };
	const std::string base{ (!update && ra == 0) ? std::string{ "0" } : reg('R', ra) };

	out = mnemonicName(d.mnemonic) + " " + reg(prefix, f.rd.Get()) + ", " + num(disp) + "(" + base + ")";
	return true;
}

// ---- Branches -----------------------------------------------------------------------
// Finds the extended-mnemonic root for a BO/BI pair ("blt", "bne", "bdnz", "bdnzt", "b" ...).
// Returns an empty string if there is no extended form.
//   usesCr : root is a condition test, so CRn (if n != 0) is printed as an operand
//   usesBi : root is a CTR+condition form, so the raw BI number is printed as an operand
static std::string
branchRoot(uint32_t bo, uint32_t bi, bool allowCtr, bool& usesCr, bool& usesBi) {
	static constexpr const char* condTrue[4]{ "lt", "gt", "eq", "so" };
	static constexpr const char* condFalse[4]{ "ge", "le", "ne", "ns" };

	usesCr = false;
	usesBi = false;

	if ((bo & 0x14) == 0x14)							// 1z1zz: branch always
		return "b";

	if ((bo & 0x1C) == 0x0C) { usesCr = true; return std::string{ "b" } + condTrue[bi & 3]; }	// 011at
	if ((bo & 0x1C) == 0x04) { usesCr = true; return std::string{ "b" } + condFalse[bi & 3]; }	// 001at

	if (!allowCtr) return {};

	if ((bo & 0x16) == 0x10) return "bdnz";				// 1a00t
	if ((bo & 0x16) == 0x12) return "bdz";				// 1a01t

	usesBi = true;
	switch (bo & 0x1E) {								// 0000z 0001z 0100z 0101z
	case 0x00: return "bdnzf";
	case 0x02: return "bdzf";
	case 0x08: return "bdnzt";
	case 0x0A: return "bdzt";
	default:   usesBi = false; return {};
	}
}

// bc / bca / bcl / bcla
static std::string
formatBc(const InstField& f, uint32_t pc) {
	const uint32_t bo{ f.bo.Get() };
	const uint32_t bi{ f.bi.Get() };
	const std::string suffix{ std::string{ f.lk.Get() ? "l" : "" } + (f.aa.Get() ? "a" : "") };
	const std::string target{ branchTarget(f.aa.Get(), f.DispBD(), pc) };

	bool usesCr{}, usesBi{};
	const std::string root{ branchRoot(bo, bi, true, usesCr, usesBi) };

	// An unconditional bc has no extended form (it is just a b with a short range)
	if (root.empty() || root == "b")
		return "bc" + suffix + " " + num(bo) + ", " + num(bi) + ", " + target;

	std::string out{ root + suffix + " " };
	if (usesBi) out += num(bi) + ", ";
	if (usesCr && (bi >> 2) != 0) out += "CR" + num(bi >> 2) + ", ";
	return out + target;
}

// bclr / bclrl / bcctr / bcctrl
static std::string
formatBranchReg(const InstField& f, bool isCtr) {
	const uint32_t bo{ f.bo.Get() };
	const uint32_t bi{ f.bi.Get() };
	const uint32_t bh{ f.bh.Get() };
	const std::string lk{ f.lk.Get() ? "l" : "" };

	bool usesCr{}, usesBi{};
	const std::string root{ bh == 0 ? branchRoot(bo, bi, !isCtr, usesCr, usesBi) : std::string{} };

	if (root.empty()) {
		std::string out{ std::string{ isCtr ? "bcctr" : "bclr" } + lk + " " + num(bo) + ", " + num(bi) };
		if (bh != 0) out += ", " + num(bh);
		return out;
	}

	std::string out{ root + (isCtr ? "ctr" : "lr") + lk };	// blr, bctr, bltlr, bnectrl, bdnzlr ...
	if (usesBi) out += " " + num(bi);
	if (usesCr && (bi >> 2) != 0) out += " CR" + num(bi >> 2);
	return out;
}

// ---- Traps --------------------------------------------------------------------------
static const char*
trapCond(uint32_t to) {
	switch (to) {
	case 1:  return "lgt";
	case 2:  return "llt";
	case 4:  return "eq";
	case 5:  return "lge";
	case 6:  return "lle";
	case 8:  return "gt";
	case 12: return "ge";
	case 16: return "lt";
	case 20: return "le";
	case 24: return "ne";
	default: return nullptr;
	}
}

static std::string
formatTrap(const InstData& d, const InstField& f) {
	const bool dword{ d.mnemonic == InstMnemonic::Td || d.mnemonic == InstMnemonic::Tdi };
	const bool imm{ d.mnemonic == InstMnemonic::Twi || d.mnemonic == InstMnemonic::Tdi };
	const uint32_t to{ f.to.Get() };
	const std::string base{ dword ? "td" : "tw" };
	const std::string last{ imm ? num(f.simm.Get()) : reg('R', f.rb.Get()) };

	if (!imm && !dword && to == 31 && f.ra.Get() == 0 && f.rb.Get() == 0)
		return "trap";

	if (const char* cond{ trapCond(to) })
		return base + cond + (imm ? "i " : " ") + reg('R', f.ra.Get()) + ", " + last;

	return base + (imm ? "i " : " ") + num(to) + ", " + reg('R', f.ra.Get()) + ", " + last;
}

// ---- Rotates ------------------------------------------------------------------------
static std::string
formatRotateRaw(InstMnemonic m, const InstField& f, bool byReg) {
	std::string out{ mnemonicFull(m, f) };

	out += " " + reg('R', f.ra.Get()) + ", " + reg('R', f.rs.Get()) + ", ";
	out += byReg ? reg('R', f.rb.Get()) : num(f.sh.Get());
	out += ", " + num(f.mb.Get()) + ", " + num(f.me.Get());
	return out;
}

static const char*
sprName(uint32_t spr) {
	switch (spr) {
	case 1:  return "xer";
	case 8:  return "lr";
	case 9:  return "ctr";
	default: return nullptr;
	}
}

bool
formatSimplified(const InstData& d, const InstField& f, uint32_t pc, std::string& out) {
	if (f.raw == 0x60000000) { out = "nop"; return true; }

	if (formatMemory(d, f, out)) return true;

	const std::string dot{ f.rc.Get() ? "." : "" };
	const std::string raRs{ " " + reg('R', f.ra.Get()) + ", " + reg('R', f.rs.Get()) };

	switch (d.mnemonic) {
		// ---- immediates ----
	case InstMnemonic::Addi:                      // addi rD, 0, simm -> li rD, simm
		if (f.ra.Get() == 0) {
			out = "li " + reg('R', f.rd.Get()) + ", " + num(f.simm.Get());
			return true;
		}
		return false;

	case InstMnemonic::Addis:                     // addis rD, 0, imm -> lis rD, imm
		if (f.ra.Get() == 0) {
			out = "lis " + reg('R', f.rd.Get()) + ", " + std::format("0x{:X}", f.uimm.Get());
			return true;
		}
		return false;

		// ---- compares ----
	case InstMnemonic::Cmpi:
	case InstMnemonic::Cmpli: {
		const bool logical{ d.mnemonic == InstMnemonic::Cmpli };
		const std::string name{ std::string{ logical ? "cmpl" : "cmp" } + (f.l.Get() ? "di" : "wi") };
		const std::string crPart{ f.crfd.Get() != 0 ? "CR" + num(f.crfd.Get()) + ", " : std::string{} };
		const std::string imm{ logical ? num(f.uimm.Get()) : num(f.simm.Get()) };

		out = name + " " + crPart + reg('R', f.ra.Get()) + ", " + imm;
		return true;
	}
	case InstMnemonic::Cmp:
	case InstMnemonic::Cmpl: {
		const bool logical{ d.mnemonic == InstMnemonic::Cmpl };
		const std::string name{ std::string{ logical ? "cmpl" : "cmp" } + (f.l.Get() ? "d" : "w") };
		const std::string crPart{ f.crfd.Get() != 0 ? "CR" + num(f.crfd.Get()) + ", " : std::string{} };

		out = name + " " + crPart + reg('R', f.ra.Get()) + ", " + reg('R', f.rb.Get());
		return true;
	}

						   // ---- traps ----
	case InstMnemonic::Tw:  case InstMnemonic::Td:
	case InstMnemonic::Twi: case InstMnemonic::Tdi:
		out = formatTrap(d, f);
		return true;

		// ---- branches ----
	case InstMnemonic::B:
		out = std::string{ "b" } + (f.lk.Get() ? "l" : "") + (f.aa.Get() ? "a" : "") + " " +
			branchTarget(f.aa.Get(), f.DispLI(), pc);
		return true;
	case InstMnemonic::Bc:
		out = formatBc(f, pc);
		return true;
	case InstMnemonic::Bclr:
		out = formatBranchReg(f, false);
		return true;
	case InstMnemonic::Bcctr:
		out = formatBranchReg(f, true);
		return true;
	case InstMnemonic::Sc:
		out = f.lev.Get() == 0 ? std::string{ "sc" } : "sc " + num(f.lev.Get());
		return true;

		// ---- CR logical ----
	case InstMnemonic::Creqv:
		if (f.crbd.Get() == f.crba.Get() && f.crba.Get() == f.crbb.Get()) {
			out = "crset " + num(f.crbd.Get());
			return true;
		}
		return false;
	case InstMnemonic::Crxor:
		if (f.crbd.Get() == f.crba.Get() && f.crba.Get() == f.crbb.Get()) {
			out = "crclr " + num(f.crbd.Get());
			return true;
		}
		return false;
	case InstMnemonic::Cror:
		if (f.crba.Get() == f.crbb.Get()) {
			out = "crmove " + num(f.crbd.Get()) + ", " + num(f.crba.Get());
			return true;
		}
		return false;
	case InstMnemonic::Crnor:
		if (f.crba.Get() == f.crbb.Get()) {
			out = "crnot " + num(f.crbd.Get()) + ", " + num(f.crba.Get());
			return true;
		}
		return false;

		// ---- logical ----
	case InstMnemonic::Or:                        // or rA, rS, rS -> mr rA, rS
		if (f.rs.Get() == f.rb.Get()) {
			out = "mr" + dot + raRs;
			return true;
		}
		return false;
	case InstMnemonic::Nor:                       // nor rA, rS, rS -> not rA, rS
		if (f.rs.Get() == f.rb.Get()) {
			out = "not" + dot + raRs;
			return true;
		}
		return false;

		// ---- SPR / CR moves ----
	case InstMnemonic::Mfspr: {
		const char* n{ sprName(f.Spr()) };
		if (!n) return false;
		out = std::string{ "mf" } + n + " " + reg('R', f.rd.Get());
		return true;
	}
	case InstMnemonic::Mtspr: {
		const char* n{ sprName(f.Spr()) };
		if (!n) return false;
		out = std::string{ "mt" } + n + " " + reg('R', f.rs.Get());
		return true;
	}
	case InstMnemonic::Mfcr:                      // bit 11 set -> mfocrf
		if ((f.raw >> 20) & 1)
			out = "mfocrf " + reg('R', f.rd.Get()) + ", " + std::format("0x{:02X}", f.crm.Get());
		else
			out = "mfcr " + reg('R', f.rd.Get());
		return true;
	case InstMnemonic::Mtcrf:                     // bit 11 set -> mtocrf
		out = std::string{ ((f.raw >> 20) & 1) ? "mtocrf " : "mtcrf " } +
			std::format("0x{:02X}", f.crm.Get()) + ", " + reg('R', f.rs.Get());
		return true;

		// ---- 32-bit rotates ----
	case InstMnemonic::Rlwinm: {
		const uint32_t sh{ f.sh.Get() };
		const uint32_t mb{ f.mb.Get() };
		const uint32_t me{ f.me.Get() };

		if (sh == 0 && me == 31)            out = "clrlwi" + dot + raRs + ", " + num(mb);
		else if (sh == 0 && mb == 0)        out = "clrrwi" + dot + raRs + ", " + num(31 - me);
		else if (mb == 0 && me == 31)       out = "rotlwi" + dot + raRs + ", " + num(sh);
		else if (mb == 0 && me == 31 - sh)  out = "slwi" + dot + raRs + ", " + num(sh);
		else if (me == 31 && sh == 32 - mb) out = "srwi" + dot + raRs + ", " + num(mb);
		else                                out = formatRotateRaw(d.mnemonic, f, false);
		return true;
	}
	case InstMnemonic::Rlwnm:
		if (f.mb.Get() == 0 && f.me.Get() == 31)    // rotlw rA, rS, rB
			out = "rotlw" + dot + raRs + ", " + reg('R', f.rb.Get());
		else
			out = formatRotateRaw(d.mnemonic, f, true);
		return true;
	case InstMnemonic::Rlwimi:
		out = formatRotateRaw(d.mnemonic, f, false);
		return true;

		// ---- 64-bit rotates ----
	case InstMnemonic::Rldicl: {
		const int64_t sh{ f.Sh64() };
		const int64_t mb{ f.Mb64() };

		if (sh == 0)                    out = "clrldi" + dot + raRs + ", " + num(mb);
		else if (mb == 0)               out = "rotldi" + dot + raRs + ", " + num(sh);
		else if (sh == 64 - mb)         out = "srdi" + dot + raRs + ", " + num(mb);
		else return false;
		return true;
	}
	case InstMnemonic::Rldicr: {
		const int64_t sh{ f.Sh64() };
		const int64_t me{ f.Me64() };

		if (sh == 0)                    out = "clrrdi" + dot + raRs + ", " + num(63 - me);
		else if (me == 63 - sh)         out = "sldi" + dot + raRs + ", " + num(sh);
		else return false;
		return true;
	}

	default:
		return false;
	}
}

bool
getInstructionString(
	const InstField& inst,
	char str[LC_INSTRUCTION_LENGTH],
	uint32_t pc) {
	InstData data{};
	std::string out{};

	if (!DecodeInstruction(inst, data) || data.mnemonic == InstMnemonic::None) {
		const uint32_t opcd{ inst.opcd.Get() };

		if (opcd == 0x13 || opcd == 0x1F || opcd == 0x3B || opcd == 0x3F)
			out = std::format("UNKNOWN OPCD: 0x{:02X} XO: {} (0x{:08X})", opcd, inst.xo10.Get(), inst.raw);
		else
			out = std::format("UNKNOWN OPCD: 0x{:02X} (0x{:08X})", opcd, inst.raw);

		strncpy_s(str, LC_INSTRUCTION_LENGTH, out.c_str(), _TRUNCATE);
		return true;
	}

	if (formatSimplified(data, inst, pc, out)) {
		strncpy_s(str, LC_INSTRUCTION_LENGTH, out.c_str(), _TRUNCATE);
		return true;
	}

	out = mnemonicFull(data.mnemonic, inst);

	constexpr size_t maxOps{ std::size(data.operands) };
	bool first{ true };
	std::string prev{};

	for (size_t i{ 0 }; i < maxOps && data.operands[i] != InstOperand::None; ++i) {
		const InstOperand op{ data.operands[i] };

		const bool isDisp{ op == InstOperand::SIMM || op == InstOperand::DS };
		if (isDisp && i + 1 < maxOps && data.operands[i + 1] == InstOperand::RA0) {
			out += first ? " " : ", ";
			first = false;
			out += formatOperand(op, inst) + "(" + formatOperand(InstOperand::RA0, inst) + ")";
			prev.clear();
			++i;
			continue;
		}

		const std::string text{ formatOperand(op, inst) };

		if (isRegOperand(op) && text == prev)
			continue;   // duplicate register, print it once

		out += first ? " " : ", ";
		first = false;
		out += text;
		prev = text;
	}

	strncpy_s(str, LC_INSTRUCTION_LENGTH, out.c_str(), _TRUNCATE);

	return true;
}

void
PrintInstruction(
	uint32_t raw,
	uint32_t pc) {

	InstField inst{ raw };

	char str[LC_INSTRUCTION_LENGTH]{};
	getInstructionString(inst, str, pc);

	printf("%#010X ", pc);
	printf("%s\n", str);
}

bool
OutputDissassemblyToStdOut(
	const Elf64_Program& prg,
	const GuestMem& mem,
	const DissassemblyOptions& options) {
	printf("\nDissAsm:\n");

	uint32_t pc{ prg.opd.entryPc };

	if (options.followThrough) {
		printf("Follow through is not yet supported!");
		return false;
	}

	for (uint32_t i{ 0 }; i < options.nonFollowOps; ++i) {
		const uint32_t raw{ mem.Read32(pc) };

		InstField inst{ raw };

		char str[LC_INSTRUCTION_LENGTH]{};
		getInstructionString(inst, str, pc);

		if (options.includePc) {
			printf("%#010x ", pc);
		}

		printf("%s\n", str);

		pc += sizeof(uint32_t);
	}

	return true;
}
