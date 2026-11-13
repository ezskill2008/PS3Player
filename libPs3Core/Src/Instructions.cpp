#include <libPs3Core/Include/Instructions.h>

struct entry {
	InstOp		op{};
	InstForm	form{};
};

typedef const entry& (*selectFn)(uint32_t raw);
typedef void(*fetchFn)(uint32_t raw, Instruction& inst);

struct primaryEntry {
	entry		e{};
	selectFn	fn{ nullptr };
};

constexpr static primaryEntry gPrimary[64]{
	{ { InstOp::OpNone, InstForm::Undefined } },	//0x00  0
	{ { InstOp::OpNone, InstForm::Undefined } },	//0x01  1
	{ { InstOp::Tdi, InstForm::D } },				//0x02  2
	{ { InstOp::Twi, InstForm::D } },				//0x03	3
	{ { InstOp::OpNone, InstForm::Undefined } },	//0x04	4
	{ { InstOp::OpNone, InstForm::Undefined } },	//0x05	5
	{ { InstOp::OpNone, InstForm::Undefined } },	//0x06	6
	{ { InstOp::OpNone, InstForm::Undefined } },	//0x07	7
	{ { InstOp::OpNone, InstForm::Undefined } },	//0x08	8
	{ { InstOp::OpNone, InstForm::Undefined } },	//0x09	9
	{ { InstOp::OpNone, InstForm::Undefined } },	//0x0A 10
	{ { InstOp::OpNone, InstForm::Undefined } },	//0x0B 11
	{ { InstOp::OpNone, InstForm::Undefined } },	//0x0C 12
	{ { InstOp::OpNone, InstForm::Undefined } },	//0x0D 13
	{ { InstOp::Addi, InstForm::D } },				//0x0E 14
	{ { InstOp::OpNone, InstForm::Undefined } },	//0x0F 15
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

};

static_assert(_countof(gFetchForm) + 1 == static_cast<uint32_t>(InstForm::MDS));

bool
LcDecodeInstruction(
	uint32_t raw,
	Instruction& inst) {
	inst = {};
	inst.opcode = InstFieldBits::OPCD(raw);
	const primaryEntry& primary{ gPrimary[inst.opcode] };

	return true;
}
