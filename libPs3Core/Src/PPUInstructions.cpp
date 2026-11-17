#include <libPs3Core/Include/PPUInstructions.h>

typedef const InstData& (*selectFn)(const InstField&);

struct primaryEntry {
	InstData	e{};
	selectFn	fn{ nullptr };
};

constexpr static InstData gOpCode12[] {
	{ InstMnemonic::B, { InstOperand::LI } },	//b target_addr (AA=0 LK=0)
	{ InstMnemonic::Ba, { InstOperand::LI } },	//ba target_addr (AA=1 LK=0)
	{ InstMnemonic::Bl, { InstOperand::LI } },	//bl target_addr (AA=0 LK=1)
	{ InstMnemonic::Bla, { InstOperand::LI } },	//bla target_addr (AA=1 LK=1)
};

const InstData& selectOpCode12(const InstField& field) {
	uint32_t index{ static_cast<uint32_t>(field.aa) & (static_cast<uint32_t>(field.lk) << 1) };
	return gOpCode12[index];
};

constexpr static InstData gOpCode3E []{
	{ InstMnemonic::Std, { InstOperand::RS, InstOperand::RA0, InstOperand::DS } },	//std RS,DS(RA
	{ InstMnemonic::Stdu, { InstOperand::RS, InstOperand::RA0, InstOperand::DS } },	//stdu RS,DS(RA)
};

const InstData& selectOpCode3E(const InstField& field) {
	return gOpCode3E[field.xoDs];
}

constexpr static primaryEntry gPrimary[64]{
	{ { InstMnemonic::None } },																//0x00  0
	{ { InstMnemonic::None } },																//0x01  1
	{ { InstMnemonic::Tdi } },																//0x02  2
	{ { InstMnemonic::Twi } },																//0x03	3
	{ { InstMnemonic::None } },																//0x04	4
	{ { InstMnemonic::None } },																//0x05	5
	{ { InstMnemonic::None } },																//0x06	6
	{ { InstMnemonic::Mulli } },															//0x07	7
	{ { InstMnemonic::Subfic } },															//0x08	8
	{ { InstMnemonic::Dozi } },																//0x09	9
	{ { InstMnemonic::Cmpli } },															//0x0A 10
	{ { InstMnemonic::Cmpi } },																//0x0B 11
	{ { InstMnemonic::Addic } },															//0x0C 12
	{ { InstMnemonic::Addicd } },															//0x0D 13
	{ { InstMnemonic::Addi, { InstOperand::RD, InstOperand::RA, InstOperand::SIMM } } },	//0x0E 14	addi RT,RA,SI
	{ { InstMnemonic::Addis } },															//0x0F 15	addis RT,RA,SI
	{ { InstMnemonic::Bc } },																//0x10 16
	{ { InstMnemonic::Sc } },																//0x11 17
	{ { InstMnemonic::B }, selectOpCode12 },												//0x12 18
	{ { InstMnemonic::None } },																//0x13 19
	{ { InstMnemonic::Rlwimi } },															//0x14 20
	{ { InstMnemonic::Rlwinm } },															//0x15 21
	{ { InstMnemonic::Rlmi } },																//0x16 22
	{ { InstMnemonic::Rlwnm } },															//0x17 23
	{ { InstMnemonic::Ori, { InstOperand::RS, InstOperand::RA, InstOperand::UIMM } } },		//0x18 24
	{ { InstMnemonic::Oris, { InstOperand::RS, InstOperand::RA, InstOperand::UIMM } } },	//0x19 25
	{ { InstMnemonic::Xori } },																//0x1A 26
	{ { InstMnemonic::Xoris } },															//0x1B 27
	{ { InstMnemonic::Andi } },																//0x1C 28
	{ { InstMnemonic::Andis } },															//0x1D 29
	{ { InstMnemonic::None } },																//0x1E 30
	{ { InstMnemonic::None } },																//0x1F 31
	{ { InstMnemonic::Lwz, { InstOperand::RD, InstOperand::SIMM, InstOperand::RA0 } } },	//0x20 32
	{ { InstMnemonic::Lwzu } },																//0x21 33
	{ { InstMnemonic::Lbz } },																//0x22 34
	{ { InstMnemonic::Lbzu } },																//0x23 35
	{ { InstMnemonic::Stw } },																//0x24 36
	{ { InstMnemonic::Stwu } },																//0x25 37
	{ { InstMnemonic::Stb } },																//0x26 38
	{ { InstMnemonic::Stbu } },																//0x27 39
	{ { InstMnemonic::Lhz } },																//0x28 40
	{ { InstMnemonic::Lhzu } },																//0x29 41
	{ { InstMnemonic::Lha } },																//0x2A 42
	{ { InstMnemonic::Lhau } },																//0x2B 43
	{ { InstMnemonic::Sth } },																//0x2C 44
	{ { InstMnemonic::Sthu } },																//0x2D 45
	{ { InstMnemonic::Lmw } },																//0x2E 46
	{ { InstMnemonic::Stmw } },																//0x2F 47
	{ { InstMnemonic::Lfs } },																//0x30 48
	{ { InstMnemonic::Lfsu } },																//0x31 49
	{ { InstMnemonic::Lfd } },																//0x32 50
	{ { InstMnemonic::Lfdu } },																//0x33 51
	{ { InstMnemonic::Stfs } },																//0x34 52
	{ { InstMnemonic::Stfsu } },															//0x35 53
	{ { InstMnemonic::Stfd } },																//0x36 54
	{ { InstMnemonic::Stfdu } },															//0x37 55
	{ { InstMnemonic::None } },																//0x38 56
	{ { InstMnemonic::None } },																//0x39 57
	{ { InstMnemonic::None } },																//0x3A 58
	{ { InstMnemonic::None } },																//0x3B 59
	{ { InstMnemonic::None } },																//0x3C 60
	{ { InstMnemonic::None } },																//0x3D 61
	{ { InstMnemonic::None }, selectOpCode3E },												//0x3E 62
	{ { InstMnemonic::None } },																//0x3F 63
};

bool
LcDecodeInstruction(
	const InstField& instField,
	InstData& inst) {
	const primaryEntry& primary{ gPrimary[instField.opcd] };

	if (primary.fn) {
		inst = primary.fn(instField);
	}
	else {
		inst = primary.e;
	}

	return true;
}
