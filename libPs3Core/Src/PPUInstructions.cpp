#include <libPs3Core/Include/PPUInstructions.h>
#include <array>

typedef const InstData& (*selectFn)(const InstField&);

struct primaryEntry {
	InstData	e{};
	selectFn	fn{ nullptr };
};

struct xoDefinition {
	uint16_t xo{};
	InstData entry{};
};

template<size_t N, size_t M>
constexpr std::array<InstData, N> buildOpTable(const xoDefinition(&defs)[M]) {
	std::array<InstData, N> table{};
	for (const xoDefinition& d : defs)
		table[d.xo] = d.entry;
	return table;
}

constexpr static xoDefinition gOpCode13Defs[]{
	{ 0,   { InstMnemonic::Mcrf,   { InstOperand::CRFD, InstOperand::CRFS } } },					//mcrf BF,BFA
	{ 16,  { InstMnemonic::Bclr,   { InstOperand::BO, InstOperand::BI, InstOperand::BH } } },		//bclr BO,BI,BH (LK=0/1)
	{ 33,  { InstMnemonic::Crnor,  { InstOperand::CRBD, InstOperand::CRBA, InstOperand::CRBB } } },	//crnor BT,BA,BB
	{ 129, { InstMnemonic::Crandc, { InstOperand::CRBD, InstOperand::CRBA, InstOperand::CRBB } } },	//crandc BT,BA,BB
	{ 193, { InstMnemonic::Crxor,  { InstOperand::CRBD, InstOperand::CRBA, InstOperand::CRBB } } },	//crxor BT,BA,BB
	{ 225, { InstMnemonic::Crnand, { InstOperand::CRBD, InstOperand::CRBA, InstOperand::CRBB } } },	//crnand BT,BA,BB
	{ 257, { InstMnemonic::Crand,  { InstOperand::CRBD, InstOperand::CRBA, InstOperand::CRBB } } },	//crand BT,BA,BB
	{ 289, { InstMnemonic::Creqv,  { InstOperand::CRBD, InstOperand::CRBA, InstOperand::CRBB } } },	//creqv BT,BA,BB
	{ 417, { InstMnemonic::Crorc,  { InstOperand::CRBD, InstOperand::CRBA, InstOperand::CRBB } } },	//crorc BT,BA,BB
	{ 449, { InstMnemonic::Cror,   { InstOperand::CRBD, InstOperand::CRBA, InstOperand::CRBB } } },	//cror BT,BA,BB
	{ 528, { InstMnemonic::Bcctr,  { InstOperand::BO, InstOperand::BI, InstOperand::BH } } },		//bcctr BO,BI,BH (LK=0/1)
};

constexpr auto gOpCode13{ buildOpTable<1024>(gOpCode13Defs) };

const InstData& selectOpCode13(const InstField& field) {
	return gOpCode13[field.xo10];
}

constexpr static InstData gOpCode1E[] {
	{ InstMnemonic::Rldicl,	 { InstOperand::RA, InstOperand::RS, InstOperand::SH, InstOperand::MB } },	//rldicl RA,RS,SH,MB	(Rc=0)
	{ InstMnemonic::Rldicl,	 { InstOperand::RA, InstOperand::RS, InstOperand::SH, InstOperand::MB } },	//rldicl RA,RS,SH,MB	(Rc=1)
	{ InstMnemonic::Rldicr,  { InstOperand::RA, InstOperand::RS, InstOperand::SH, InstOperand::ME } },	//rldicr RA,RS,SH,ME	(Rc=0)
	{ InstMnemonic::Rldicr,  { InstOperand::RA, InstOperand::RS, InstOperand::SH, InstOperand::ME } },	//rldicr RA,RS,SH,ME	(Rc=1)
	{ InstMnemonic::Rldic,   { InstOperand::RA, InstOperand::RS, InstOperand::SH, InstOperand::MB } },	//rldic  RA,RS,SH,MB	(Rc=0)
	{ InstMnemonic::Rldic,   { InstOperand::RA, InstOperand::RS, InstOperand::SH, InstOperand::MB } },	//rldic  RA,RS,SH,MB	(Rc=1)
	{ InstMnemonic::Rldimi,  { InstOperand::RA, InstOperand::RS, InstOperand::SH, InstOperand::MB } },	//rldimi RA,RS,SH,MB	(Rc=0)
	{ InstMnemonic::Rldimi,  { InstOperand::RA, InstOperand::RS, InstOperand::SH, InstOperand::MB } },	//rldimi RA,RS,SH,MB	(Rc=1)
	{ InstMnemonic::Rldcr,   { InstOperand::RA, InstOperand::RS, InstOperand::RB, InstOperand::ME } },	//rldcr RA,RS,RB,ME		(Rc=0)
	{ InstMnemonic::Rldcr,   { InstOperand::RA, InstOperand::RS, InstOperand::RB, InstOperand::ME } },	//rldcr RA,RS,RB,ME		(Rc=1)
};

const InstData& selectOpCode1E(const InstField& field) {
	return gOpCode1E[field.xoMds];
}

constexpr static xoDefinition gOpCode1FDefs[]{
	{ 0,   { InstMnemonic::Cmp,    { InstOperand::CRFD, InstOperand::L, InstOperand::RA, InstOperand::RB } } },	//cmp BF,L,RA,RB
	{ 32,  { InstMnemonic::Cmpl,   { InstOperand::CRFD, InstOperand::L, InstOperand::RA, InstOperand::RB } } },	//cmpl BF,L,RA,RB
	{ 4,   { InstMnemonic::Tw,     { InstOperand::TO, InstOperand::RA, InstOperand::RB } } },					//tw TO,RA,RB
	{ 68,  { InstMnemonic::Td,     { InstOperand::TO, InstOperand::RA, InstOperand::RB } } },					//td TO,RA,RB
	{ 266, { InstMnemonic::Add,    { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//add
	{ 778, { InstMnemonic::Add,    { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//addo
	{ 10,  { InstMnemonic::Addc,   { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//addc
	{ 522, { InstMnemonic::Addc,   { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//addco
	{ 138, { InstMnemonic::Adde,   { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//adde
	{ 650, { InstMnemonic::Adde,   { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//addeo
	{ 234, { InstMnemonic::Addme,  { InstOperand::RD, InstOperand::RA } } },									//addme
	{ 746, { InstMnemonic::Addme,  { InstOperand::RD, InstOperand::RA } } },									//addmeo
	{ 202, { InstMnemonic::Addze,  { InstOperand::RD, InstOperand::RA } } },									//addze
	{ 714, { InstMnemonic::Addze,  { InstOperand::RD, InstOperand::RA } } },									//addzeo
	{ 40,  { InstMnemonic::Subf,   { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//subf
	{ 552, { InstMnemonic::Subf,   { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//subfo
	{ 8,   { InstMnemonic::Subfc,  { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//subfc
	{ 520, { InstMnemonic::Subfc,  { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//subfco
	{ 136, { InstMnemonic::Subfe,  { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//subfe
	{ 648, { InstMnemonic::Subfe,  { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//subfeo
	{ 232, { InstMnemonic::Subfme, { InstOperand::RD, InstOperand::RA } } },									//subfme
	{ 744, { InstMnemonic::Subfme, { InstOperand::RD, InstOperand::RA } } },									//subfmeo
	{ 200, { InstMnemonic::Subfze, { InstOperand::RD, InstOperand::RA } } },									//subfze
	{ 712, { InstMnemonic::Subfze, { InstOperand::RD, InstOperand::RA } } },									//subfzeo
	{ 104, { InstMnemonic::Neg,    { InstOperand::RD, InstOperand::RA } } },									//neg
	{ 616, { InstMnemonic::Neg,    { InstOperand::RD, InstOperand::RA } } },									//nego
	{ 235, { InstMnemonic::Mullw,  { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//mullw
	{ 747, { InstMnemonic::Mullw,  { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//mullwo
	{ 233, { InstMnemonic::Mulld,  { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//mulld
	{ 745, { InstMnemonic::Mulld,  { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//mulldo
	{ 491, { InstMnemonic::Divw,   { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//divw
	{ 1003,{ InstMnemonic::Divw,   { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//divwo
	{ 459, { InstMnemonic::Divwu,  { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//divwu
	{ 971, { InstMnemonic::Divwu,  { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//divwuo
	{ 489, { InstMnemonic::Divd,   { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//divd
	{ 1001,{ InstMnemonic::Divd,   { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//divdo
	{ 457, { InstMnemonic::Divdu,  { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//divdu
	{ 969, { InstMnemonic::Divdu,  { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//divduo
	{ 75,  { InstMnemonic::Mulhw,  { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//mulhw
	{ 11,  { InstMnemonic::Mulhwu, { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//mulhwu
	{ 73,  { InstMnemonic::Mulhd,  { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//mulhd
	{ 9,   { InstMnemonic::Mulhdu, { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//mulhdu
	{ 28,  { InstMnemonic::And,    { InstOperand::RA, InstOperand::RS, InstOperand::RB } } },					//and
	{ 60,  { InstMnemonic::Andc,   { InstOperand::RA, InstOperand::RS, InstOperand::RB } } },					//andc
	{ 444, { InstMnemonic::Or,     { InstOperand::RA, InstOperand::RS, InstOperand::RB } } },					//or
	{ 412, { InstMnemonic::Orc,    { InstOperand::RA, InstOperand::RS, InstOperand::RB } } },					//orc
	{ 316, { InstMnemonic::Xor,    { InstOperand::RA, InstOperand::RS, InstOperand::RB } } },					//xor
	{ 476, { InstMnemonic::Nand,   { InstOperand::RA, InstOperand::RS, InstOperand::RB } } },					//nand
	{ 124, { InstMnemonic::Nor,    { InstOperand::RA, InstOperand::RS, InstOperand::RB } } },					//nor
	{ 284, { InstMnemonic::Eqv,    { InstOperand::RA, InstOperand::RS, InstOperand::RB } } },					//eqv
	{ 954, { InstMnemonic::Extsb,  { InstOperand::RA, InstOperand::RS } } },									//extsb
	{ 922, { InstMnemonic::Extsh,  { InstOperand::RA, InstOperand::RS } } },									//extsh
	{ 986, { InstMnemonic::Extsw,  { InstOperand::RA, InstOperand::RS } } },									//extsw
	{ 26,  { InstMnemonic::Cntlzw, { InstOperand::RA, InstOperand::RS } } },									//cntlzw
	{ 58,  { InstMnemonic::Cntlzd, { InstOperand::RA, InstOperand::RS } } },									//cntlzd
	{ 24,  { InstMnemonic::Slw,    { InstOperand::RA, InstOperand::RS, InstOperand::RB } } },					//slw
	{ 536, { InstMnemonic::Srw,    { InstOperand::RA, InstOperand::RS, InstOperand::RB } } },					//srw
	{ 792, { InstMnemonic::Sraw,   { InstOperand::RA, InstOperand::RS, InstOperand::RB } } },					//sraw
	{ 824, { InstMnemonic::Srawi,  { InstOperand::RA, InstOperand::RS, InstOperand::SH } } },					//srawi
	{ 27,  { InstMnemonic::Sld,    { InstOperand::RA, InstOperand::RS, InstOperand::RB } } },					//sld
	{ 539, { InstMnemonic::Srd,    { InstOperand::RA, InstOperand::RS, InstOperand::RB } } },					//srd
	{ 794, { InstMnemonic::Srad,   { InstOperand::RA, InstOperand::RS, InstOperand::RB } } },					//srad
	{ 826, { InstMnemonic::Sradi,  { InstOperand::RA, InstOperand::RS, InstOperand::SH64 } } },					//sradi (sh5 = 0)
	{ 827, { InstMnemonic::Sradi,  { InstOperand::RA, InstOperand::RS, InstOperand::SH64 } } },					//sradi (sh5 = 1)
	{ 87,  { InstMnemonic::Lbzx,   { InstOperand::RD, InstOperand::RA0, InstOperand::RB } } },					//lbzx
	{ 119, { InstMnemonic::Lbzux,  { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//lbzux
	{ 279, { InstMnemonic::Lhzx,   { InstOperand::RD, InstOperand::RA0, InstOperand::RB } } },					//lhzx
	{ 311, { InstMnemonic::Lhzux,  { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//lhzux
	{ 343, { InstMnemonic::Lhax,   { InstOperand::RD, InstOperand::RA0, InstOperand::RB } } },					//lhax
	{ 375, { InstMnemonic::Lhaux,  { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//lhaux
	{ 23,  { InstMnemonic::Lwzx,   { InstOperand::RD, InstOperand::RA0, InstOperand::RB } } },					//lwzx
	{ 55,  { InstMnemonic::Lwzux,  { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//lwzux
	{ 341, { InstMnemonic::Lwax,   { InstOperand::RD, InstOperand::RA0, InstOperand::RB } } },					//lwax
	{ 373, { InstMnemonic::Lwaux,  { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//lwaux
	{ 21,  { InstMnemonic::Ldx,    { InstOperand::RD, InstOperand::RA0, InstOperand::RB } } },					//ldx
	{ 53,  { InstMnemonic::Ldux,   { InstOperand::RD, InstOperand::RA, InstOperand::RB } } },					//ldux
	{ 215, { InstMnemonic::Stbx,   { InstOperand::RS, InstOperand::RA0, InstOperand::RB } } },					//stbx
	{ 247, { InstMnemonic::Stbux,  { InstOperand::RS, InstOperand::RA, InstOperand::RB } } },					//stbux
	{ 407, { InstMnemonic::Sthx,   { InstOperand::RS, InstOperand::RA0, InstOperand::RB } } },					//sthx
	{ 439, { InstMnemonic::Sthux,  { InstOperand::RS, InstOperand::RA, InstOperand::RB } } },					//sthux
	{ 151, { InstMnemonic::Stwx,   { InstOperand::RS, InstOperand::RA0, InstOperand::RB } } },					//stwx
	{ 183, { InstMnemonic::Stwux,  { InstOperand::RS, InstOperand::RA, InstOperand::RB } } },					//stwux
	{ 149, { InstMnemonic::Stdx,   { InstOperand::RS, InstOperand::RA0, InstOperand::RB } } },					//stdx
	{ 181, { InstMnemonic::Stdux,  { InstOperand::RS, InstOperand::RA, InstOperand::RB } } },					//stdux
	{ 790, { InstMnemonic::Lhbrx,  { InstOperand::RD, InstOperand::RA0, InstOperand::RB } } },					//lhbrx
	{ 534, { InstMnemonic::Lwbrx,  { InstOperand::RD, InstOperand::RA0, InstOperand::RB } } },					//lwbrx
	{ 918, { InstMnemonic::Sthbrx, { InstOperand::RS, InstOperand::RA0, InstOperand::RB } } },					//sthbrx
	{ 662, { InstMnemonic::Stwbrx, { InstOperand::RS, InstOperand::RA0, InstOperand::RB } } },					//stwbrx
	{ 597, { InstMnemonic::Lswi,   { InstOperand::RD, InstOperand::RA0, InstOperand::NB } } },					//lswi
	{ 533, { InstMnemonic::Lswx,   { InstOperand::RD, InstOperand::RA0, InstOperand::RB } } },					//lswx
	{ 725, { InstMnemonic::Stswi,  { InstOperand::RS, InstOperand::RA0, InstOperand::NB } } },					//stswi
	{ 661, { InstMnemonic::Stswx,  { InstOperand::RS, InstOperand::RA0, InstOperand::RB } } },					//stswx
	{ 339, { InstMnemonic::Mfspr,  { InstOperand::RD, InstOperand::SPR } } },									//mfspr RT,SPR
	{ 467, { InstMnemonic::Mtspr,  { InstOperand::SPR, InstOperand::RS } } },									//mtspr SPR,RS
	{ 19,  { InstMnemonic::Mfcr,   { InstOperand::RD } } },														//mfcr RT (mfocrf if bit 11 set)
	{ 144, { InstMnemonic::Mtcrf,  { InstOperand::CRM, InstOperand::RS } } },									//mtcrf FXM,RS (mtocrf if bit 11 set)
	{ 512, { InstMnemonic::Mcrxr,  { InstOperand::CRFD } } },													//mcrxr BF
	{ 535, { InstMnemonic::Lfsx,   { InstOperand::FRD, InstOperand::RA0, InstOperand::RB } } },					//lfsx
	{ 567, { InstMnemonic::Lfsux,  { InstOperand::FRD, InstOperand::RA, InstOperand::RB } } },					//lfsux
	{ 599, { InstMnemonic::Lfdx,   { InstOperand::FRD, InstOperand::RA0, InstOperand::RB } } },					//lfdx
	{ 631, { InstMnemonic::Lfdux,  { InstOperand::FRD, InstOperand::RA, InstOperand::RB } } },					//lfdux
	{ 663, { InstMnemonic::Stfsx,  { InstOperand::FRS, InstOperand::RA0, InstOperand::RB } } },					//stfsx
	{ 695, { InstMnemonic::Stfsux, { InstOperand::FRS, InstOperand::RA, InstOperand::RB } } },					//stfsux
	{ 727, { InstMnemonic::Stfdx,  { InstOperand::FRS, InstOperand::RA0, InstOperand::RB } } },					//stfdx
	{ 759, { InstMnemonic::Stfdux, { InstOperand::FRS, InstOperand::RA, InstOperand::RB } } },					//stfdux
	{ 983, { InstMnemonic::Stfiwx, { InstOperand::FRS, InstOperand::RA0, InstOperand::RB } } },					//stfiwx
};

constexpr auto gOpCode1F{ buildOpTable<1024>(gOpCode1FDefs) };

const InstData& selectOpCode1F(const InstField& field) {
	return gOpCode1F[field.xo10];
}

constexpr static InstData gOpCode3A[] {
	{ InstMnemonic::Ld,		{ InstOperand::RD, InstOperand::RA0, InstOperand::DS } },	//ld RT,DS(RA)
	{ InstMnemonic::Ldu,	{ InstOperand::RD, InstOperand::RA0, InstOperand::DS } },	//ldu RT,DS(RA)
	{ InstMnemonic::Lwa,	{ InstOperand::RD, InstOperand::RA0, InstOperand::DS } },	//lwa RT,DS(RA)
};

const InstData& selectOpCode3A(const InstField& field) {
	return gOpCode3A[field.xoDs];
}

constexpr static xoDefinition gOpCode3BDefs[]{
	{ 18, { InstMnemonic::Fdivs,   { InstOperand::FRD, InstOperand::FRA, InstOperand::FRB } } },						//fdivs FRT,FRA,FRB
	{ 20, { InstMnemonic::Fsubs,   { InstOperand::FRD, InstOperand::FRA, InstOperand::FRB } } },						//fsubs FRT,FRA,FRB
	{ 21, { InstMnemonic::Fadds,   { InstOperand::FRD, InstOperand::FRA, InstOperand::FRB } } },						//fadds FRT,FRA,FRB
	{ 22, { InstMnemonic::Fsqrts,  { InstOperand::FRD, InstOperand::FRB } } },											//fsqrts FRT,FRB (optional)
	{ 24, { InstMnemonic::Fres,    { InstOperand::FRD, InstOperand::FRB } } },											//fres FRT,FRB (optional)
	{ 25, { InstMnemonic::Fmuls,   { InstOperand::FRD, InstOperand::FRA, InstOperand::FRC } } },						//fmuls FRT,FRA,FRC
	{ 28, { InstMnemonic::Fmsubs,  { InstOperand::FRD, InstOperand::FRA, InstOperand::FRC, InstOperand::FRB } } },		//fmsubs FRT,FRA,FRC,FRB
	{ 29, { InstMnemonic::Fmadds,  { InstOperand::FRD, InstOperand::FRA, InstOperand::FRC, InstOperand::FRB } } },		//fmadds FRT,FRA,FRC,FRB
	{ 30, { InstMnemonic::Fnmsubs, { InstOperand::FRD, InstOperand::FRA, InstOperand::FRC, InstOperand::FRB } } },		//fnmsubs FRT,FRA,FRC,FRB
	{ 31, { InstMnemonic::Fnmadds, { InstOperand::FRD, InstOperand::FRA, InstOperand::FRC, InstOperand::FRB } } },		//fnmadds FRT,FRA,FRC,FRB
};

constexpr auto gOpCode3B{ buildOpTable<32>(gOpCode3BDefs) };

const InstData& selectOpCode3B(const InstField& field) {
	return gOpCode3B[field.xo5];
}

constexpr static InstData gOpCode3E []{
	{ InstMnemonic::Std, { InstOperand::RS, InstOperand::RA0, InstOperand::DS } },	//std RS,DS(RA)
	{ InstMnemonic::Stdu, { InstOperand::RS, InstOperand::RA0, InstOperand::DS } },	//stdu RS,DS(RA)
};

const InstData& selectOpCode3E(const InstField& field) {
	return gOpCode3E[field.xoDs];
}

constexpr static xoDefinition gOpCode3FADefs[]{
	{ 18, { InstMnemonic::Fdiv,    { InstOperand::FRD, InstOperand::FRA, InstOperand::FRB } } },						//fdiv FRT,FRA,FRB
	{ 20, { InstMnemonic::Fsub,    { InstOperand::FRD, InstOperand::FRA, InstOperand::FRB } } },						//fsub FRT,FRA,FRB
	{ 21, { InstMnemonic::Fadd,    { InstOperand::FRD, InstOperand::FRA, InstOperand::FRB } } },						//fadd FRT,FRA,FRB
	{ 22, { InstMnemonic::Fsqrt,   { InstOperand::FRD, InstOperand::FRB } } },											//fsqrt FRT,FRB (optional)
	{ 23, { InstMnemonic::Fsel,    { InstOperand::FRD, InstOperand::FRA, InstOperand::FRC, InstOperand::FRB } } },		//fsel FRT,FRA,FRC,FRB (optional)
	{ 25, { InstMnemonic::Fmul,    { InstOperand::FRD, InstOperand::FRA, InstOperand::FRC } } },						//fmul FRT,FRA,FRC
	{ 26, { InstMnemonic::Frsqrte, { InstOperand::FRD, InstOperand::FRB } } },											//frsqrte FRT,FRB (optional)
	{ 28, { InstMnemonic::Fmsub,   { InstOperand::FRD, InstOperand::FRA, InstOperand::FRC, InstOperand::FRB } } },		//fmsub FRT,FRA,FRC,FRB
	{ 29, { InstMnemonic::Fmadd,   { InstOperand::FRD, InstOperand::FRA, InstOperand::FRC, InstOperand::FRB } } },		//fmadd FRT,FRA,FRC,FRB
	{ 30, { InstMnemonic::Fnmsub,  { InstOperand::FRD, InstOperand::FRA, InstOperand::FRC, InstOperand::FRB } } },		//fnmsub FRT,FRA,FRC,FRB
	{ 31, { InstMnemonic::Fnmadd,  { InstOperand::FRD, InstOperand::FRA, InstOperand::FRC, InstOperand::FRB } } },		//fnmadd FRT,FRA,FRC,FRB
};

constexpr static xoDefinition gOpCode3FDefs[]{
	{ 0,   { InstMnemonic::Fcmpu,  { InstOperand::CRFD, InstOperand::FRA, InstOperand::FRB } } },						//fcmpu BF,FRA,FRB
	{ 32,  { InstMnemonic::Fcmpo,  { InstOperand::CRFD, InstOperand::FRA, InstOperand::FRB } } },						//fcmpo BF,FRA,FRB
	{ 12,  { InstMnemonic::Frsp,   { InstOperand::FRD, InstOperand::FRB } } },											//frsp FRT,FRB
	{ 14,  { InstMnemonic::Fctiw,  { InstOperand::FRD, InstOperand::FRB } } },											//fctiw FRT,FRB
	{ 15,  { InstMnemonic::Fctiwz, { InstOperand::FRD, InstOperand::FRB } } },											//fctiwz FRT,FRB
	{ 814, { InstMnemonic::Fctid,  { InstOperand::FRD, InstOperand::FRB } } },											//fctid FRT,FRB
	{ 815, { InstMnemonic::Fctidz, { InstOperand::FRD, InstOperand::FRB } } },											//fctidz FRT,FRB
	{ 846, { InstMnemonic::Fcfid,  { InstOperand::FRD, InstOperand::FRB } } },											//fcfid FRT,FRB
	{ 72,  { InstMnemonic::Fmr,    { InstOperand::FRD, InstOperand::FRB } } },											//fmr FRT,FRB
	{ 40,  { InstMnemonic::Fneg,   { InstOperand::FRD, InstOperand::FRB } } },											//fneg FRT,FRB
	{ 264, { InstMnemonic::Fabs,   { InstOperand::FRD, InstOperand::FRB } } },											//fabs FRT,FRB
	{ 136, { InstMnemonic::Fnabs,  { InstOperand::FRD, InstOperand::FRB } } },											//fnabs FRT,FRB
	{ 583, { InstMnemonic::Mffs,   { InstOperand::FRD } } },															//mffs FRT
	{ 64,  { InstMnemonic::Mcrfs,  { InstOperand::CRFD, InstOperand::CRFS } } },										//mcrfs BF,BFA
	{ 134, { InstMnemonic::Mtfsfi, { InstOperand::CRFD, InstOperand::UIMM } } },										//mtfsfi BF,U
	{ 711, { InstMnemonic::Mtfsf,   { InstOperand::FM, InstOperand::FRB } } },											//mtfsf FLM,FRB
	{ 70,  { InstMnemonic::Mtfsb0, { InstOperand::CRBD } } },															//mtfsb0 BT
	{ 38,  { InstMnemonic::Mtfsb1, { InstOperand::CRBD } } },															//mtfsb1 BT
};

constexpr auto gOpCode3FA{ buildOpTable<32>(gOpCode3FADefs) };
constexpr auto gOpCode3F{ buildOpTable<1024>(gOpCode3FDefs) };

const InstData& selectOpCode3F(const InstField& field) {
	const InstData& a{ gOpCode3FA[field.xo5] };

	if (a.mnemonic != InstMnemonic::None) {
		return a;
	}

	return gOpCode3F[field.xo10];
}
constexpr static primaryEntry gPrimary[64]{
	{ { InstMnemonic::None } },																								//0x00  0
	{ { InstMnemonic::None } },																								//0x01  1
	{ { InstMnemonic::Tdi,    { InstOperand::TO, InstOperand::RA, InstOperand::SIMM } } },									//0x02  2	tdi TO,RA,SI
	{ { InstMnemonic::Twi,    { InstOperand::TO, InstOperand::RA, InstOperand::SIMM } } },									//0x03  3	twi TO,RA,SI
	{ { InstMnemonic::None } },																								//0x04  4	(VMX, not in Book I)
	{ { InstMnemonic::None } },																								//0x05  5
	{ { InstMnemonic::None } },																								//0x06  6
	{ { InstMnemonic::Mulli,  { InstOperand::RD, InstOperand::RA, InstOperand::SIMM } } },									//0x07  7	mulli RT,RA,SI
	{ { InstMnemonic::Subfic, { InstOperand::RD, InstOperand::RA, InstOperand::SIMM } } },									//0x08  8	subfic RT,RA,SI
	{ { InstMnemonic::None } },																								//0x09  9
	{ { InstMnemonic::Cmpli,  { InstOperand::CRFD, InstOperand::L, InstOperand::RA, InstOperand::UIMM } } },				//0x0A 10	cmpli BF,L,RA,UI
	{ { InstMnemonic::Cmpi,   { InstOperand::CRFD, InstOperand::L, InstOperand::RA, InstOperand::SIMM } } },				//0x0B 11	cmpi BF,L,RA,SI
	{ { InstMnemonic::Addic,  { InstOperand::RD, InstOperand::RA, InstOperand::SIMM } } },									//0x0C 12	addic RT,RA,SI
	{ { InstMnemonic::Addicd, { InstOperand::RD, InstOperand::RA, InstOperand::SIMM } } },									//0x0D 13	addic. RT,RA,SI
	{ { InstMnemonic::Addi,   { InstOperand::RD, InstOperand::RA0, InstOperand::SIMM } } },									//0x0E 14	addi RT,RA|0,SI
	{ { InstMnemonic::Addis,  { InstOperand::RD, InstOperand::RA0, InstOperand::SIMM } } },									//0x0F 15	addis RT,RA|0,SI
	{ { InstMnemonic::Bc,     { InstOperand::BO, InstOperand::BI, InstOperand::BD } } },									//0x10 16	bc BO,BI,BD (AA,LK)
	{ { InstMnemonic::Sc,     { InstOperand::LEV } } },																		//0x11 17	sc LEV
	{ { InstMnemonic::B,      { InstOperand::LI } } },																		//0x12 18	b LI (AA,LK)
	{ { InstMnemonic::None }, selectOpCode13 },																				//0x13 19
	{ { InstMnemonic::Rlwimi, { InstOperand::RA, InstOperand::RS, InstOperand::SH, InstOperand::MB, InstOperand::ME } } },	//0x14 20	rlwimi RA,RS,SH,MB,ME
	{ { InstMnemonic::Rlwinm, { InstOperand::RA, InstOperand::RS, InstOperand::SH, InstOperand::MB, InstOperand::ME } }	},	//0x15 21	rlwinm RA,RS,SH,MB,ME
	{ { InstMnemonic::None } },																								//0x16 22	(rlmi, POWER only)
	{ { InstMnemonic::Rlwnm,  { InstOperand::RA, InstOperand::RS, InstOperand::RB, InstOperand::MB, InstOperand::ME } } },	//0x17 23	rlwnm RA,RS,RB,MB,ME
	{ { InstMnemonic::Ori,    { InstOperand::RA, InstOperand::RS, InstOperand::UIMM } } },									//0x18 24	ori RA,RS,UI
	{ { InstMnemonic::Oris,   { InstOperand::RA, InstOperand::RS, InstOperand::UIMM } } },									//0x19 25	oris RA,RS,UI
	{ { InstMnemonic::Xori,   { InstOperand::RA, InstOperand::RS, InstOperand::UIMM } } },									//0x1A 26	xori RA,RS,UI
	{ { InstMnemonic::Xoris,  { InstOperand::RA, InstOperand::RS, InstOperand::UIMM } } },									//0x1B 27	xoris RA,RS,UI
	{ { InstMnemonic::Andi,   { InstOperand::RA, InstOperand::RS, InstOperand::UIMM } } },									//0x1C 28	andi. RA,RS,UI
	{ { InstMnemonic::Andis,  { InstOperand::RA, InstOperand::RS, InstOperand::UIMM } } },									//0x1D 29	andis. RA,RS,UI
	{ { InstMnemonic::None }, selectOpCode1E },																				//0x1E 30
	{ { InstMnemonic::None }, selectOpCode1F },																				//0x1F 31
	{ { InstMnemonic::Lwz,    { InstOperand::RD, InstOperand::SIMM, InstOperand::RA0 } } },									//0x20 32	lwz RT,D(RA|0)
	{ { InstMnemonic::Lwzu,   { InstOperand::RD, InstOperand::SIMM, InstOperand::RA } } },									//0x21 33	lwzu RT,D(RA)
	{ { InstMnemonic::Lbz,    { InstOperand::RD, InstOperand::SIMM, InstOperand::RA0 } } },									//0x22 34
	{ { InstMnemonic::Lbzu,   { InstOperand::RD, InstOperand::SIMM, InstOperand::RA } } },									//0x23 35
	{ { InstMnemonic::Stw,    { InstOperand::RS, InstOperand::SIMM, InstOperand::RA0 } } },									//0x24 36
	{ { InstMnemonic::Stwu,   { InstOperand::RS, InstOperand::SIMM, InstOperand::RA } } },									//0x25 37
	{ { InstMnemonic::Stb,    { InstOperand::RS, InstOperand::SIMM, InstOperand::RA0 } } },									//0x26 38
	{ { InstMnemonic::Stbu,   { InstOperand::RS, InstOperand::SIMM, InstOperand::RA } } },									//0x27 39
	{ { InstMnemonic::Lhz,    { InstOperand::RD, InstOperand::SIMM, InstOperand::RA0 } } },									//0x28 40
	{ { InstMnemonic::Lhzu,   { InstOperand::RD, InstOperand::SIMM, InstOperand::RA } } },									//0x29 41
	{ { InstMnemonic::Lha,    { InstOperand::RD, InstOperand::SIMM, InstOperand::RA0 } } },									//0x2A 42
	{ { InstMnemonic::Lhau,   { InstOperand::RD, InstOperand::SIMM, InstOperand::RA } } },									//0x2B 43
	{ { InstMnemonic::Sth,    { InstOperand::RS, InstOperand::SIMM, InstOperand::RA0 } } },									//0x2C 44
	{ { InstMnemonic::Sthu,   { InstOperand::RS, InstOperand::SIMM, InstOperand::RA } } },									//0x2D 45
	{ { InstMnemonic::Lmw,    { InstOperand::RD, InstOperand::SIMM, InstOperand::RA0 } } },									//0x2E 46
	{ { InstMnemonic::Stmw,   { InstOperand::RS, InstOperand::SIMM, InstOperand::RA0 } } },									//0x2F 47
	{ { InstMnemonic::Lfs,    { InstOperand::FRD, InstOperand::SIMM, InstOperand::RA0 } } },								//0x30 48
	{ { InstMnemonic::Lfsu,   { InstOperand::FRD, InstOperand::SIMM, InstOperand::RA } } },									//0x31 49
	{ { InstMnemonic::Lfd,    { InstOperand::FRD, InstOperand::SIMM, InstOperand::RA0 } } },								//0x32 50
	{ { InstMnemonic::Lfdu,   { InstOperand::FRD, InstOperand::SIMM, InstOperand::RA } } },									//0x33 51
	{ { InstMnemonic::Stfs,   { InstOperand::FRS, InstOperand::SIMM, InstOperand::RA0 } } },								//0x34 52
	{ { InstMnemonic::Stfsu,  { InstOperand::FRS, InstOperand::SIMM, InstOperand::RA } } },									//0x35 53
	{ { InstMnemonic::Stfd,   { InstOperand::FRS, InstOperand::SIMM, InstOperand::RA0 } } },								//0x36 54
	{ { InstMnemonic::Stfdu,  { InstOperand::FRS, InstOperand::SIMM, InstOperand::RA } } },									//0x37 55
	{ { InstMnemonic::None } },																								//0x38 56
	{ { InstMnemonic::None } },																								//0x39 57
	{ { InstMnemonic::None }, selectOpCode3A },																				//0x3A 58
	{ { InstMnemonic::None }, selectOpCode3B },																				//0x3B 59
	{ { InstMnemonic::None } },																								//0x3C 60
	{ { InstMnemonic::None } },																								//0x3D 61
	{ { InstMnemonic::None }, selectOpCode3E },																				//0x3E 62
	{ { InstMnemonic::None }, selectOpCode3F },																				//0x3F 63
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
