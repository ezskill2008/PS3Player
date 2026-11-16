#include <libPs3Core/Include/Instructions.h>

#include <string>
#include <sstream>

struct entry {
	InstOp		op{};
	InstForm	form{};
	OpFmt		fmt{};
};

typedef const entry& (*selectFn)(uint32_t raw);
typedef void(*fetchFn)(uint32_t raw, Instruction& inst);

struct primaryEntry {
	entry		e{};
	selectFn	fn{ nullptr };
};

//TODO Add inst op fmt
constexpr static primaryEntry gPrimary[64]{
	{ { InstOp::OpNone, InstForm::Undefined } },		//0x00  0
	{ { InstOp::OpNone, InstForm::Undefined } },		//0x01  1
	{ { InstOp::Tdi, InstForm::D } },					//0x02  2
	{ { InstOp::Twi, InstForm::D } },					//0x03	3
	{ { InstOp::OpNone, InstForm::Undefined } },		//0x04	4
	{ { InstOp::OpNone, InstForm::Undefined } },		//0x05	5
	{ { InstOp::OpNone, InstForm::Undefined } },		//0x06	6
	{ { InstOp::OpNone, InstForm::Undefined } },		//0x07	7
	{ { InstOp::OpNone, InstForm::Undefined } },		//0x08	8
	{ { InstOp::OpNone, InstForm::Undefined } },		//0x09	9
	{ { InstOp::OpNone, InstForm::Undefined } },		//0x0A 10
	{ { InstOp::OpNone, InstForm::Undefined } },		//0x0B 11
	{ { InstOp::OpNone, InstForm::Undefined } },		//0x0C 12
	{ { InstOp::OpNone, InstForm::Undefined } },		//0x0D 13
	{ { InstOp::Addi, InstForm::D, OpFmt::RtRaSi } },	//0x0E 14
	{ { InstOp::OpNone, InstForm::Undefined } },		//0x0F 15
};

//MUST match OpFmt
constexpr static InstBits gFmtBits[]{
	InstBits::None,
	InstBits::RT | InstBits::RA | InstBits::SI,
	InstBits::RT | InstBits::RA | InstBits::UI,
	InstBits::RT | InstBits::RS | InstBits::UI,
	InstBits::RT | InstBits::D | InstBits::RA,
	InstBits::RS | InstBits::D | InstBits::RA,
	InstBits::RT,
};

constexpr const char* gOpInstStrs[]{
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
};

void
fetchIForm(uint32_t raw, Instruction& inst) {

}

void
fetchDForm(uint32_t raw, Instruction& inst) {
	inst.rt = InstFieldBits::RT(raw);
	inst.ra = InstFieldBits::RA(raw);
	inst.ui = InstFieldBits::UI(raw);
	inst.si = InstFieldBits::SI(raw);
}

constexpr static fetchFn gFetchForm[14]{
	nullptr,
	fetchIForm,
	nullptr,
	nullptr,
	fetchDForm,
};

static_assert(_countof(gFetchForm) + 1 == static_cast<uint32_t>(InstForm::MDS));

bool
LcDecodeInstruction(
	uint32_t raw,
	Instruction& inst) {
	inst = {};
	inst.opcode = InstFieldBits::OPCD(raw);

	const primaryEntry& primary{ gPrimary[inst.opcode] };
	const uint8_t form_idx{ static_cast<uint8_t>(primary.e.form) };

	if (primary.fn) {
		//Get secondary form
	}
	else {
		if (primary.e.form == InstForm::Undefined) return false;

		inst.instOp = primary.e.op;
		inst.fmt = primary.e.fmt;
	}


	if (!gFetchForm[form_idx]) return false;

	gFetchForm[form_idx](raw, inst);

	return true;
}

bool
LcGetInstructionString(
	const Instruction& inst,
	char str[LC_INSTRUCTION_LENGTH]) {
	const InstBits usedBits{ gFmtBits[static_cast<uint32_t>(inst.fmt)] };

	std::string out{ gOpInstStrs[static_cast<uint32_t>(inst.instOp)] };

	if (inst.instOp == InstOp::OpNone) {
		out += " ";
		out += std::to_string(inst.opcode);

		strncpy_s(str, LC_INSTRUCTION_LENGTH, out.c_str(), _TRUNCATE);

		return true;
	}

	bool first{ true };

	auto Sep = [&]() {
		out += first ? " " : ", ";
		first = false;
		};

	if (Has(usedBits, InstBits::RT)) { Sep(); out += "R" + std::to_string(inst.rt); }
	if (Has(usedBits, InstBits::RA)) { Sep(); out += "R" + std::to_string(inst.ra); }
	if (Has(usedBits, InstBits::RB)) { Sep(); out += "R" + std::to_string(inst.rb); }
	if (Has(usedBits, InstBits::UI)) { Sep(); out += std::to_string(inst.ui); }
	if (Has(usedBits, InstBits::SI)) { Sep(); out += std::to_string(inst.si); }

	strncpy_s(str, LC_INSTRUCTION_LENGTH, out.c_str(), _TRUNCATE);

	return true;
}
