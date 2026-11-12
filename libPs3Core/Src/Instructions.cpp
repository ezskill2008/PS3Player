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

constexpr static primaryEntry gPrimary[64] {
	{ { InstOp::OpNone, InstForm::Undefined } },
	{ { InstOp::OpNone, InstForm::Undefined } },
};

void
fetchIForm(uint32_t raw, Instruction& inst) {

}

bool
LpsDecodeInstruction(
	uint32_t raw,
	Instruction& inst) {


	return true;
}
