#include <libPs3Core/Include/PPUInstructions.h>

#include <string>
#include <sstream>
#include <format>

typedef const InstData& (*selectFn)(uint32_t raw);

struct primaryEntry {
	InstData	e{};
	selectFn	fn{ nullptr };
};

constexpr static primaryEntry gPrimary[64]{
	{ { InstMnemonic::None } },																//0x00  0
	{ { InstMnemonic::None } },																//0x01  1
	{ { InstMnemonic::Tdi } },																//0x02  2
	{ { InstMnemonic::Twi } },																//0x03	3
	{ { InstMnemonic::None } },																//0x04	4
	{ { InstMnemonic::None } },																//0x05	5
	{ { InstMnemonic::None } },																//0x06	6
	{ { InstMnemonic::None } },																//0x07	7
	{ { InstMnemonic::None } },																//0x08	8
	{ { InstMnemonic::None } },																//0x09	9
	{ { InstMnemonic::None } },																//0x0A 10
	{ { InstMnemonic::None } },																//0x0B 11
	{ { InstMnemonic::None } },																//0x0C 12
	{ { InstMnemonic::None } },																//0x0D 13
	{ { InstMnemonic::Addi, { InstOperand::RD, InstOperand::RA, InstOperand::SIMM } } },	//0x0E 14
	{ { InstMnemonic::None } },																//0x0F 15
	{ { InstMnemonic::None } },																//0x10 16
	{ { InstMnemonic::None } },																//0x11 17
	{ { InstMnemonic::None } },																//0x12 18
	{ { InstMnemonic::None } },																//0x13 19
	{ { InstMnemonic::None } },																//0x14 20
	{ { InstMnemonic::None } },																//0x15 21
	{ { InstMnemonic::None } },																//0x16 22
	{ { InstMnemonic::None } },																//0x17 23
	{ { InstMnemonic::Ori, { InstOperand::RS, InstOperand::RA, InstOperand::UIMM } } },		//0x18 24
	{ { InstMnemonic::Oris, { InstOperand::RS, InstOperand::RA, InstOperand::UIMM } } },	//0x19 25
	{ { InstMnemonic::None } },																//0x1A 26
	{ { InstMnemonic::None } },																//0x1B 27
	{ { InstMnemonic::None } },																//0x1C 28
	{ { InstMnemonic::None } },																//0x1D 29
	{ { InstMnemonic::None } },																//0x1E 30
	{ { InstMnemonic::None } },																//0x1F 31
	{ { InstMnemonic::Lwz, { InstOperand::RD, InstOperand::SIMM, InstOperand::RA0 } } },	//0x20 32
};

constexpr const char* gMnemonicStrs[]{
	"UNKNOWN",
	"TDI",
	"TWI",
	"MULLI",
	"SUBFIC",
	"DOZI",
	"CMPLI",
	"CMPI",
	"ADDIC",
	"ADDICD",
	"ADDI",
	"ADDIS",
	"BC",
	"SC",
	"B",
	"ORI",
	"ORIS",
	"XORI",
	"XORIS",
	"ANDI",
	"ANDIS",
	"LWZ",
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
	case InstOperand::LI:    return num(f.DispLI());
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

bool
LcDecodeInstruction(
	const InstField& instField,
	InstData& inst) {
	const primaryEntry& primary{ gPrimary[instField.opcd] };

	if (primary.fn) {
		//Get secondary form
	}
	else {
		inst = primary.e;
	}

	return true;
}

bool
LcGetInstructionString(
	const InstField& inst,
	char str[LC_INSTRUCTION_LENGTH]) {
	InstData data{};
	std::string out{};

	if (!LcDecodeInstruction(inst, data) || data.mnemonic == InstMnemonic::None) {
		out = std::format("UNKNOWN OPCD: 0x{:02X}", inst.opcd.Get());
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
