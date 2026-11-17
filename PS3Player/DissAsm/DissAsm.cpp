#include <PS3Player/DissAsm/DissAsm.h>

#include <string>
#include <format>
#include <sstream>
#include <iomanip>

constexpr const char* gMnemonicStrs[]{
	"none",
	"tdi",
	"twi",
	"mulli",
	"subfic",
	"dozi",
	"cmpli",
	"cmpi",
	"addic",
	"addic.",
	"addi",
	"addis",
	"bc",
	"sc",
	"b",
	"rlwimi",
	"rlwinm",
	"rlmi",
	"rlwnm",
	"ori",
	"oris",
	"xori",
	"xoris",
	"andi.",
	"andis.",
	"lwz",
	"lwzu",
	"lbz",
	"lbzu",
	"stw",
	"stwu",
	"stb",
	"stbu",
	"lhz",
	"lhzu",
	"lha",
	"lhau",
	"sth",
	"sthu",
	"lmw",
	"stmw",
	"lfs",
	"lfsu",
	"lfd",
	"lfdu",
	"stfs",
	"stfsu",
	"stfd",
	"stfdu",
	"std",
	"stdu",
	"ba",
	"bl",
	"bla",
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
	case InstOperand::CRM:   return num(f.crm.Get());
	case InstOperand::FM:    return num(f.fm.Get());
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

	case InstMnemonic::Std:
		dsForm = true;
		break;

	case InstMnemonic::Stdu:
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

static std::string
formatBc(const InstField& f, uint32_t pc) {
	static constexpr const char* condTrue[4]{ "lt", "gt", "eq", "so" };
	static constexpr const char* condFalse[4]{ "ge", "le", "ne", "ns" };

	const uint32_t bo{ f.bo.Get() };
	const uint32_t bi{ f.bi.Get() };
	const uint32_t crf{ bi >> 2 };
	const uint32_t bit{ bi & 3 };

	const std::string suffix{ std::string{ f.lk.Get() ? "l" : "" } + (f.aa.Get() ? "a" : "") };
	const std::string target{ branchTarget(f.aa.Get(), f.DispBD(), pc) };
	const std::string crPart{ crf != 0 ? "CR" + num(crf) + ", " : std::string{} };

	// 001at: branch if condition false, 011at: branch if condition true (hint bits ignored)
	if ((bo & 0x1C) == 0x0C)
		return "b" + std::string{ condTrue[bit] } + suffix + " " + crPart + target;
	if ((bo & 0x1C) == 0x04)
		return "b" + std::string{ condFalse[bit] } + suffix + " " + crPart + target;

	// 1a00t: decrement CTR, branch if CTR != 0.  1a01t: ... if CTR == 0
	if ((bo & 0x16) == 0x10)
		return "bdnz" + suffix + " " + target;
	if ((bo & 0x16) == 0x12)
		return "bdz" + suffix + " " + target;

	return "bc" + suffix + " " + num(bo) + ", " + num(bi) + ", " + target;
}

static std::string
formatRotateRaw(InstMnemonic m, const InstField& f, bool byReg) {
	std::string out{ mnemonicName(m) };
	if (f.rc.Get()) out += ".";

	out += " " + reg('R', f.ra.Get()) + ", " + reg('R', f.rs.Get()) + ", ";
	out += byReg ? reg('R', f.rb.Get()) : num(f.sh.Get());
	out += ", " + num(f.mb.Get()) + ", " + num(f.me.Get());
	return out;
}

bool
formatSimplified(const InstData& d, const InstField& f, uint32_t pc, std::string& out) {
	if (f.raw == 0x60000000) { out = "nop"; return true; }
	if (f.raw == 0x4E800020) { out = "blr"; return true; }

	if (formatMemory(d, f, out)) return true;

	switch (d.mnemonic) {
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

	case InstMnemonic::Cmpi:
	case InstMnemonic::Cmpli: {
		const bool logical{ d.mnemonic == InstMnemonic::Cmpli };
		const std::string name{ std::string{ logical ? "cmpl" : "cmp" } + (f.l.Get() ? "di" : "wi") };
		const std::string crPart{ f.crfd.Get() != 0 ? "CR" + num(f.crfd.Get()) + ", " : std::string{} };
		const std::string imm{ logical ? num(f.uimm.Get()) : num(f.simm.Get()) };

		out = name + " " + crPart + reg('R', f.ra.Get()) + ", " + imm;
		return true;
	}

	case InstMnemonic::Bc:
		out = formatBc(f, pc);
		return true;

	case InstMnemonic::B:
	case InstMnemonic::Ba:
	case InstMnemonic::Bl:
	case InstMnemonic::Bla:
		out = mnemonicName(d.mnemonic) + " " + branchTarget(f.aa.Get(), f.DispLI(), pc);
		return true;

	case InstMnemonic::Sc:
		out = f.lev.Get() == 0 ? std::string{ "sc" } : "sc " + num(f.lev.Get());
		return true;

	case InstMnemonic::Rlwinm: {
		const uint32_t sh{ f.sh.Get() };
		const uint32_t mb{ f.mb.Get() };
		const uint32_t me{ f.me.Get() };
		const std::string dot{ f.rc.Get() ? "." : "" };
		const std::string args{ " " + reg('R', f.ra.Get()) + ", " + reg('R', f.rs.Get()) + ", " };

		if (sh == 0 && me == 31)            out = "clrlwi" + dot + args + num(mb);
		else if (sh == 0 && mb == 0)        out = "clrrwi" + dot + args + num(31 - me);
		else if (mb == 0 && me == 31)       out = "rotlwi" + dot + args + num(sh);
		else if (mb == 0 && me == 31 - sh)  out = "slwi" + dot + args + num(sh);
		else if (me == 31 && sh == 32 - mb) out = "srwi" + dot + args + num(mb);
		else                                out = formatRotateRaw(d.mnemonic, f, false);
		return true;
	}

	case InstMnemonic::Rlwnm:
		if (f.mb.Get() == 0 && f.me.Get() == 31) {  // rotlw rA, rS, rB
			out = std::string{ "rotlw" } + (f.rc.Get() ? "." : "") + " " +
				reg('R', f.ra.Get()) + ", " + reg('R', f.rs.Get()) + ", " + reg('R', f.rb.Get());
		}
		else {
			out = formatRotateRaw(d.mnemonic, f, true);
		}
		return true;

	case InstMnemonic::Rlwimi:
		out = formatRotateRaw(d.mnemonic, f, false);
		return true;

	case InstMnemonic::Rlmi:
		out = formatRotateRaw(d.mnemonic, f, true);
		return true;

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

	if (!LcDecodeInstruction(inst, data) || data.mnemonic == InstMnemonic::None) {
		out = std::format("UNKNOWN OPCD: 0x{:02X}", inst.opcd.Get());
		strncpy_s(str, LC_INSTRUCTION_LENGTH, out.c_str(), _TRUNCATE);
		return true;
	}

	if (formatSimplified(data, inst, pc, out)) {
		strncpy_s(str, LC_INSTRUCTION_LENGTH, out.c_str(), _TRUNCATE);
		return true;
	}

	out = gMnemonicStrs[static_cast<uint32_t>(data.mnemonic)];

	constexpr size_t maxOps{ std::size(data.operands) };
	bool first{ true };

	for (size_t i{ 0 }; i < maxOps && data.operands[i] != InstOperand::None; ++i) {
		const InstOperand op{ data.operands[i] };

		out += first ? " " : ", ";
		first = false;

		const bool isDisp{ op == InstOperand::SIMM || op == InstOperand::DS };
		if (isDisp && i + 1 < maxOps && data.operands[i + 1] == InstOperand::RA0) {
			out += formatOperand(op, inst);
			out += "(";
			out += formatOperand(InstOperand::RA0, inst);
			out += ")";
			++i;
			continue;
		}

		out += formatOperand(op, inst);
	}

	strncpy_s(str, LC_INSTRUCTION_LENGTH, out.c_str(), _TRUNCATE);

	return true;
}

bool
OutputDissAssemblyToStdOut(
	const Elf64_Program& prg,
	const GuestMem& mem) {
	printf("\nDissAsm:\n");

	uint32_t pc{ prg.opd.entryPc };

	for (uint32_t i{ 0 }; i < 100;++i) {
		const uint32_t raw{ mem.Read32(pc) };

		InstField inst{ raw };
		InstData data{};

		LcDecodeInstruction(inst, data);

		char str[LC_INSTRUCTION_LENGTH]{};
		getInstructionString(inst, str, pc);
		printf("%#010x %s\n", pc, str);

		pc += sizeof(uint32_t);
	}


	return true;
}
