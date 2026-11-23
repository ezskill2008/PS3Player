#include <PS3Player/Emu/PPU.h>
#include <PS3Player/Emu/PPUFetch.h>
#include <PS3Player/DissAsm/DissAsm.h>

#include <tuple>
#include <utility>
#include <cstddef>
#include <intrin.h>

namespace detail {
template <typename Arr, std::size_t... I>
constexpr auto ops_val_impl(const Arr& a, std::index_sequence<I...>) {
	return std::tuple{ a[I]... };
}
template <std::size_t N, typename Arr>
constexpr auto ops_val(const Arr& a) {
	return ops_val_impl(a, std::make_index_sequence<N>{});
}
}

#define OPS_EXPAND(x) x
#define OPS_NARGS_(_1,_2,_3,_4,_5,_6,_7,_8,N,...) N
#define OPS_NARGS(...) OPS_EXPAND(OPS_NARGS_(__VA_ARGS__,8,7,6,5,4,3,2,1,0))

#define DEFINE_OPS(...) \
    [[maybe_unused]] const auto [__VA_ARGS__] = \
        detail::ops_val<OPS_NARGS(__VA_ARGS__)>(ppu.operands)

#define XER_SO static_cast<uint64_t>(1 << 31)
#define XER_OV static_cast<uint64_t>(1 << 30)
#define XER_CA static_cast<uint64_t>(1 << 29)
#define CR_LT static_cast<uint8_t>(0b1000)
#define CR_GT static_cast<uint8_t>(0b0100)
#define CR_EQ static_cast<uint8_t>(0b0010)
#define CR_SO static_cast<uint8_t>(0b0001)

typedef void(*instructionOp)(PPU& ppu);

namespace cr {
inline void
setCRField(PPU& ppu, uint32_t field, uint8_t bits) {
	const uint32_t shift{ (7 - field) * 4 };
	ppu.reg.cr = (ppu.reg.cr & ~(0xFu << shift))
		| (static_cast<uint32_t>(bits & 0xF) << shift);
}

inline void
setCRFixedPoint(PPU& ppu, int64_t value, bool SO) {
	uint8_t bits{ value < 0 ? CR_LT : value > 0 ? CR_GT : CR_EQ };
	if (SO) bits |= CR_SO;
	setCRField(ppu, 0, bits);
}
}

namespace fixedp {
#pragma region fixed point load
void
lbz(PPU& ppu) {
	DEFINE_OPS(rd, ra, d);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ ra == 0 ? 0 : gpr[ra] };
	const uint64_t ea{ b + static_cast<int64_t>(static_cast<int16_t>(d)) };

	gpr[rd] = static_cast<uint64_t>(ppu.guestMem->Read8(static_cast<uint32_t>(ea)));
}

void
lbzu(PPU& ppu) {
	DEFINE_OPS(rd, ra, d);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ gpr[ra] };
	const uint64_t ea{ b + static_cast<int64_t>(static_cast<int16_t>(d)) };

	gpr[rd] = static_cast<uint64_t>(ppu.guestMem->Read8(static_cast<uint32_t>(ea)));
	gpr[ra] = ea;
}

void
lbzx(PPU& ppu) {
	DEFINE_OPS(rd, ra, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ ra == 0 ? 0 : gpr[ra] };
	const uint64_t ea{ b + gpr[rb] };

	gpr[rd] = static_cast<uint64_t>(ppu.guestMem->Read8(static_cast<uint32_t>(ea)));
}

void
lbzux(PPU& ppu) {
	DEFINE_OPS(rd, ra, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ gpr[ra] };
	const uint64_t ea{ b + gpr[rb] };

	gpr[rd] = static_cast<uint64_t>(ppu.guestMem->Read8(static_cast<uint32_t>(ea)));
	gpr[ra] = ea;
}

void
lhz(PPU& ppu) {
	DEFINE_OPS(rd, ra, d);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ ra == 0 ? 0 : gpr[ra] };
	const uint64_t ea{ b + static_cast<int64_t>(static_cast<int16_t>(d)) };

	gpr[rd] = static_cast<uint64_t>(ppu.guestMem->Read16(static_cast<uint32_t>(ea)));
}

void
lhzu(PPU& ppu) {
	DEFINE_OPS(rd, ra, d);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ gpr[ra] };
	const uint64_t ea{ b + static_cast<int64_t>(static_cast<int16_t>(d)) };

	gpr[rd] = static_cast<uint64_t>(ppu.guestMem->Read16(static_cast<uint32_t>(ea)));
	gpr[ra] = ea;
}

void
lhzx(PPU& ppu) {
	DEFINE_OPS(rd, ra, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ ra == 0 ? 0 : gpr[ra] };
	const uint64_t ea{ b + gpr[rb] };

	gpr[rd] = static_cast<uint64_t>(ppu.guestMem->Read16(static_cast<uint32_t>(ea)));
}

void
lhzux(PPU& ppu) {
	DEFINE_OPS(rd, ra, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ gpr[ra] };
	const uint64_t ea{ b + gpr[rb] };

	gpr[rd] = static_cast<uint64_t>(ppu.guestMem->Read16(static_cast<uint32_t>(ea)));
	gpr[ra] = ea;
}

void
lha(PPU& ppu) {
	DEFINE_OPS(rd, d, ra);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ ra == 0 ? 0 : gpr[ra] };
	const uint64_t ea{ b + static_cast<int64_t>(static_cast<int16_t>(d)) };

	gpr[rd] = static_cast<uint64_t>(static_cast<int64_t>(static_cast<int16_t>(
		ppu.guestMem->Read16(static_cast<uint32_t>(ea))
		)));
}

void
lhau(PPU& ppu) {
	DEFINE_OPS(rd, d, ra);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ gpr[ra] };
	const uint64_t ea{ b + static_cast<int64_t>(static_cast<int16_t>(d)) };

	gpr[rd] = static_cast<uint64_t>(static_cast<int64_t>(static_cast<int16_t>(
		ppu.guestMem->Read16(static_cast<uint32_t>(ea))
		)));
	gpr[ra] = ea;
}

void
lhax(PPU& ppu) {
	DEFINE_OPS(rd, ra, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ ra == 0 ? 0 : gpr[ra] };
	const uint64_t ea{ b + gpr[rb] };

	gpr[rd] = static_cast<uint64_t>(static_cast<int64_t>(static_cast<int16_t>(
		ppu.guestMem->Read16(static_cast<uint32_t>(ea))
		)));
}

void
lhaux(PPU& ppu) {
	DEFINE_OPS(rd, ra, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ gpr[ra] };
	const uint64_t ea{ b + gpr[rb] };

	gpr[rd] = static_cast<uint64_t>(static_cast<int64_t>(static_cast<int16_t>(
		ppu.guestMem->Read16(static_cast<uint32_t>(ea))
		)));
	gpr[ra] = ea;
}

void
lwz(PPU& ppu) {
	DEFINE_OPS(rd, d, ra);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ ra == 0 ? 0 : gpr[ra] };
	const uint64_t ea{ b + static_cast<int64_t>(static_cast<int16_t>(d)) };
	
	gpr[rd] = static_cast<uint64_t>(ppu.guestMem->Read32(static_cast<uint32_t>(ea)));
}

void
lwzu(PPU& ppu) {
	DEFINE_OPS(rd, d, ra);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ gpr[ra] };
	const uint64_t ea{ b + static_cast<int64_t>(static_cast<int16_t>(d)) };

	gpr[rd] = static_cast<uint64_t>(ppu.guestMem->Read32(static_cast<uint32_t>(ea)));
	gpr[ra] = ea;
}

void
lwzx(PPU& ppu) {
	DEFINE_OPS(rd, ra, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ ra == 0 ? 0 : gpr[ra] };
	const uint64_t ea{ b + gpr[rb] };

	gpr[rd] = static_cast<uint64_t>(ppu.guestMem->Read32(static_cast<uint32_t>(ea)));
}

void
lwzux(PPU& ppu) {
	DEFINE_OPS(rd, ra, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ gpr[ra] };
	const uint64_t ea{ b + gpr[rb] };

	gpr[rd] = static_cast<uint64_t>(ppu.guestMem->Read32(static_cast<uint32_t>(ea)));
	gpr[ra] = ea;
}

void
lwa(PPU& ppu) {
	DEFINE_OPS(rd, ra, ds);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ ra == 0 ? 0 : gpr[ra] };
	const uint64_t ea{ b + static_cast<uint64_t>(static_cast<int64_t>(ppu.inst.DispDS())) };

	gpr[rd] = static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(
		ppu.guestMem->Read32(static_cast<uint32_t>(ea))
		)));
}

void
lwax(PPU& ppu) {
	DEFINE_OPS(rd, ra, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ ra == 0 ? 0 : gpr[ra] };
	const uint64_t ea{ b + gpr[rb] };

	gpr[rd] = static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(
		ppu.guestMem->Read32(static_cast<uint32_t>(ea))
		)));
}

void
lwaux(PPU& ppu) {
	DEFINE_OPS(rd, ra, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ gpr[ra] };
	const uint64_t ea{ b + gpr[rb] };

	gpr[rd] = static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(
		ppu.guestMem->Read32(static_cast<uint32_t>(ea))
		)));
	gpr[ra] = ea;
}

void
ld(PPU& ppu) {
	DEFINE_OPS(rd, ra, ds);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ ra == 0 ? 0 : gpr[ra] };
	const uint64_t ea{ b + static_cast<uint64_t>(static_cast<int64_t>(ppu.inst.DispDS())) };

	gpr[rd] = ppu.guestMem->Read64(static_cast<uint32_t>(ea));
}

void
ldu(PPU& ppu) {
	DEFINE_OPS(rd, ra, ds);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ gpr[ra] };
	const uint64_t ea{ b + static_cast<uint64_t>(static_cast<int64_t>(ppu.inst.DispDS())) };

	gpr[rd] = ppu.guestMem->Read64(static_cast<uint32_t>(ea));
	gpr[ra] = ea;
}

void
ldx(PPU& ppu) {
	DEFINE_OPS(rd, ra, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ ra == 0 ? 0 : gpr[ra] };
	const uint64_t ea{ b + gpr[rb] };

	gpr[rd] = ppu.guestMem->Read64(static_cast<uint32_t>(ea));
}

void
ldux(PPU& ppu) {
	DEFINE_OPS(rd, ra, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ gpr[ra] };
	const uint64_t ea{ b + gpr[rb] };

	gpr[rd] = ppu.guestMem->Read64(static_cast<uint32_t>(ea));
	gpr[ra] = ea;
}

#pragma endregion
#pragma region fixed point store
void
stb(PPU& ppu) {
	DEFINE_OPS(rs, ra, d);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ ra == 0 ? 0 : gpr[ra] };
	const uint64_t ea{ b + static_cast<uint64_t>(static_cast<int64_t>(static_cast<int16_t>(d))) };

	ppu.guestMem->Write8(static_cast<uint32_t>(ea), static_cast<uint8_t>(gpr[rs]));
}

void
stbu(PPU& ppu) {
	DEFINE_OPS(rs, ra, d);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ gpr[ra] };
	const uint64_t ea{ b + static_cast<uint64_t>(static_cast<int64_t>(static_cast<int16_t>(d))) };

	ppu.guestMem->Write8(static_cast<uint32_t>(ea), static_cast<uint8_t>(gpr[rs]));
	gpr[ra] = ea;
}

void
stbx(PPU& ppu) {
	DEFINE_OPS(rs, ra, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ ra == 0 ? 0 : gpr[ra] };
	const uint64_t ea{ b + gpr[rb] };

	ppu.guestMem->Write8(static_cast<uint32_t>(ea), static_cast<uint8_t>(gpr[rs]));
}

void
stbux(PPU& ppu) {
	DEFINE_OPS(rs, ra, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ gpr[ra] };
	const uint64_t ea{ b + gpr[rb] };

	ppu.guestMem->Write8(static_cast<uint32_t>(ea), static_cast<uint8_t>(gpr[rs]));
	gpr[ra] = ea;
}

void
sth(PPU& ppu) {
	DEFINE_OPS(rs, ra, d);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ ra == 0 ? 0 : gpr[ra] };
	const uint64_t ea{ b + static_cast<uint64_t>(static_cast<int64_t>(static_cast<int16_t>(d))) };

	ppu.guestMem->Write16(static_cast<uint32_t>(ea), static_cast<uint16_t>(gpr[rs]));
}

void
sthu(PPU& ppu) {
	DEFINE_OPS(rs, ra, d);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ gpr[ra] };
	const uint64_t ea{ b + static_cast<uint64_t>(static_cast<int64_t>(static_cast<int16_t>(d))) };

	ppu.guestMem->Write16(static_cast<uint32_t>(ea), static_cast<uint16_t>(gpr[rs]));
	gpr[ra] = ea;
}

void
sthx(PPU& ppu) {
	DEFINE_OPS(rs, ra, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ ra == 0 ? 0 : gpr[ra] };
	const uint64_t ea{ b + gpr[rb] };

	ppu.guestMem->Write16(static_cast<uint32_t>(ea), static_cast<uint16_t>(gpr[rs]));
}

void
sthux(PPU& ppu) {
	DEFINE_OPS(rs, ra, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ gpr[ra] };
	const uint64_t ea{ b + gpr[rb] };

	ppu.guestMem->Write16(static_cast<uint32_t>(ea), static_cast<uint16_t>(gpr[rs]));
	gpr[ra] = ea;
}

void
stw(PPU& ppu) {
	DEFINE_OPS(rs, ra, d);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ ra == 0 ? 0 : gpr[ra] };
	const uint64_t ea{ b + static_cast<uint64_t>(static_cast<int64_t>(static_cast<int16_t>(d))) };

	ppu.guestMem->Write32(static_cast<uint32_t>(ea), static_cast<uint32_t>(gpr[rs]));
}

void
stwu(PPU& ppu) {
	DEFINE_OPS(rs, ra, d);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ gpr[ra] };
	const uint64_t ea{ b + static_cast<uint64_t>(static_cast<int64_t>(static_cast<int16_t>(d))) };

	ppu.guestMem->Write32(static_cast<uint32_t>(ea), static_cast<uint32_t>(gpr[rs]));
	gpr[ra] = ea;
}

void
stwx(PPU& ppu) {
	DEFINE_OPS(rs, ra, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ ra == 0 ? 0 : gpr[ra] };
	const uint64_t ea{ b + gpr[rb] };

	ppu.guestMem->Write32(static_cast<uint32_t>(ea), static_cast<uint32_t>(gpr[rs]));
}

void
stwux(PPU& ppu) {
	DEFINE_OPS(rs, ra, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ gpr[ra] };
	const uint64_t ea{ b + gpr[rb] };

	ppu.guestMem->Write32(static_cast<uint32_t>(ea), static_cast<uint32_t>(gpr[rs]));
	gpr[ra] = ea;
}

void
std(PPU& ppu) {
	DEFINE_OPS(rs, ra, ds);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ ra == 0 ? 0 : gpr[ra] };
	const uint64_t ea{ b + static_cast<uint64_t>(static_cast<int64_t>(ppu.inst.DispDS())) };

	ppu.guestMem->Write64(static_cast<uint32_t>(ea), gpr[rs]);
}

void
stdu(PPU& ppu) {
	DEFINE_OPS(rs, ra, ds);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ gpr[ra] };
	const uint64_t ea{ b + static_cast<uint64_t>(static_cast<int64_t>(ppu.inst.DispDS())) };

	ppu.guestMem->Write64(static_cast<uint32_t>(ea), gpr[rs]);
	gpr[ra] = ea;
}


void
stdx(PPU& ppu) {
	DEFINE_OPS(rs, ra, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ ra == 0 ? 0 : gpr[ra] };
	const uint64_t ea{ b + gpr[rb] };

	ppu.guestMem->Write64(static_cast<uint32_t>(ea), gpr[rs]);
}

void
stdux(PPU& ppu) {
	DEFINE_OPS(rs, ra, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ gpr[ra] };
	const uint64_t ea{ b + gpr[rb] };

	ppu.guestMem->Write64(static_cast<uint32_t>(ea), gpr[rs]);
	gpr[ra] = ea;
}
#pragma endregion
//fixed point arithmetic
void
addi(PPU& ppu) {
	DEFINE_OPS(rd, ra, simm);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t imm{ static_cast<uint64_t>(static_cast<int64_t>(static_cast<int16_t>(simm))) };

	gpr[rd] = (ra == 0) ? imm : gpr[ra] + imm;
}

void
addis(PPU& ppu) {
	DEFINE_OPS(rd, ra, simm);
	auto& gpr{ ppu.reg.gpr };

	const uint64_t imm{ static_cast<uint64_t>(static_cast<int64_t>(static_cast<int16_t>(simm))) << 16 };
	gpr[rd] = (ra == 0) ? imm : gpr[ra] + imm;
}

void
addic(PPU& ppu) {
	DEFINE_OPS(rd, ra, simm);
	auto& gpr{ ppu.reg.gpr };

	const uint64_t a{ gpr[ra] };
	const uint64_t b{ static_cast<uint64_t>(static_cast<int64_t>(static_cast<int16_t>(simm))) };

	uint64_t r{ 0 };
	const uint8_t cout{ _addcarry_u64(0, a, b, &r) };

	gpr[rd] = r;

	if (cout) {
		ppu.reg.xer |= XER_CA;
	}
	else {
		ppu.reg.xer &= ~static_cast<uint64_t>(XER_CA);
	}
}

void
addicd(PPU& ppu) {
	DEFINE_OPS(rd, ra, simm);
	auto& gpr{ ppu.reg.gpr };

	const uint64_t a{ gpr[ra] };
	const uint64_t b{ static_cast<uint64_t>(static_cast<int64_t>(static_cast<int16_t>(simm))) };

	uint64_t r{ 0 };
	const uint8_t cout{ _addcarry_u64(0, a, b, &r) };

	gpr[rd] = r;

	cr::setCRFixedPoint(ppu,
		static_cast<int64_t>(r),
		(ppu.reg.xer & XER_SO) != 0);

	if (cout) {
		ppu.reg.xer |= XER_CA;
	}
	else {
		ppu.reg.xer &= ~static_cast<uint64_t>(XER_CA);
	}
}

void
add(PPU& ppu) {
	DEFINE_OPS(rd, ra, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t a{ gpr[ra] };
	const uint64_t b{ gpr[rb] };
	const uint64_t r{ a + b };

	gpr[rd] = r;

	if (ppu.inst.oe) {
		const bool ov{ ((a ^ r) & (b ^ r)) >> 63 != 0 };
		if (ov) {
			ppu.reg.xer |= (XER_OV | XER_SO);   // set OV, and SO is sticky
		}
		else {
			ppu.reg.xer &= ~XER_OV;             // clear OV only; SO is never cleared here
		}
	}
	if (ppu.inst.rc) {
		cr::setCRFixedPoint(ppu,
			static_cast<int64_t>(r),
			(ppu.reg.xer & XER_SO) != 0);
	}
}

void
subf(PPU& ppu) {
	DEFINE_OPS(rd, ra, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t a{ gpr[ra] };
	const uint64_t b{ gpr[rb] };
	const uint64_t r{ ~a + b + 1 };

	gpr[rd] = r;

	if (ppu.inst.oe) {
		const bool ov{ (((a ^ b) & (b ^ r)) >> 63) != 0 };
		if (ov) {
			ppu.reg.xer |= (XER_OV | XER_SO);   // set OV, and SO is sticky
		}
		else {
			ppu.reg.xer &= ~XER_OV;             // clear OV only; SO is never cleared here
		}
	}
	if (ppu.inst.rc) {
		cr::setCRFixedPoint(ppu,
			static_cast<int64_t>(r),
			(ppu.reg.xer & XER_SO) != 0);
	}
}

void
ori(PPU& ppu) {
	DEFINE_OPS(rs, ra, uimm);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t s{ gpr[rs] };
	const uint64_t ui{ static_cast<uint64_t>(static_cast<uint16_t>(uimm)) };
	const uint64_t r{ s | ui };

	gpr[ra] = r;
}

void
oris(PPU& ppu) {
	DEFINE_OPS(rs, ra, uimm);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t s{ gpr[rs] };
	const uint64_t ui{ static_cast<uint64_t>(static_cast<uint16_t>(uimm)) << 16 };
	const uint64_t r{ s | ui };

	gpr[ra] = r;
}
}

constexpr static instructionOp insts[197]{
	nullptr,	//0
	nullptr,	//1
	nullptr,	//2
	nullptr,	//3
	nullptr,	//4
	nullptr,	//5
	nullptr,	//6
	fixedp::addic,	//7
	fixedp::addicd,	//8
	fixedp::addi,	//9
	fixedp::addis,	//10
	nullptr,	//11
	nullptr,	//12
	nullptr,	//13
	nullptr,	//14
	nullptr,	//15
	nullptr,	//16
	fixedp::ori,	//17
	fixedp::oris,	//18
	nullptr,	//19
	nullptr,	//20
	nullptr,	//21
	nullptr,	//22
	fixedp::lwz,	//23
	fixedp::lwzu,	//24
	fixedp::lbz,	//25
	fixedp::lbzu,	//26
	fixedp::stw,	//27
	fixedp::stwu,	//28
	fixedp::stb,	//29
	fixedp::stbu,	//30
	fixedp::lhz,	//31
	fixedp::lhzu,	//32
	fixedp::lha,	//33
	fixedp::lhau,	//34
	fixedp::sth,	//35
	fixedp::sthu,	//36
	nullptr,	//37
	nullptr,	//38
	nullptr,	//39
	nullptr,	//40
	nullptr,	//41
	nullptr,	//42
	nullptr,	//43
	nullptr,	//44
	nullptr,	//45
	nullptr,	//46
	fixedp::ld,		//47
	fixedp::ldu,	//48
	fixedp::lwa,	//49
	fixedp::std,	//50
	fixedp::stdu,	//51
	nullptr,	//52
	nullptr,	//53
	nullptr,	//54
	nullptr,	//55
	nullptr,	//56
	nullptr,	//57
	nullptr,	//58
	nullptr,	//59
	nullptr,	//60
	nullptr,	//61
	nullptr,	//62
	nullptr,	//63
	nullptr,	//64
	nullptr,	//65
	nullptr,	//66
	nullptr,	//67
	nullptr,	//68
	nullptr,	//69
	nullptr,	//70
	nullptr,	//71
	nullptr,	//72
	fixedp::lbzx,	//73
	fixedp::lbzux,	//74
	fixedp::lhzx,	//75
	fixedp::lhzux,	//76
	fixedp::lhax,	//77
	fixedp::lhaux,	//78
	fixedp::lwzx,	//79
	fixedp::lwzux,	//80
	fixedp::lwax,	//81
	fixedp::lwaux,	//82
	fixedp::ldx,	//83
	fixedp::ldux,	//84
	fixedp::stbx,	//85
	fixedp::stbux,	//86
	fixedp::sthx,	//87
	fixedp::sthux,	//88
	fixedp::stwx,	//89
	fixedp::stwux,	//90
	fixedp::stdx,	//91
	fixedp::stdux,	//92
	nullptr,	//93
	nullptr,	//94
	nullptr,	//95
	nullptr,	//96
	nullptr,	//97
	nullptr,	//98
	nullptr,	//99
	nullptr,	//100
	fixedp::add,	//101
	nullptr,	//102
	nullptr,	//103
	nullptr,	//104
	nullptr,	//105
	fixedp::subf,	//106
	nullptr,	//107
	nullptr,	//108
	nullptr,	//109
	nullptr,	//110
	nullptr,	//111
	nullptr,	//112
	nullptr,	//113
	nullptr,	//114
	nullptr,	//115
	nullptr,	//116
	nullptr,	//117
	nullptr,	//118
	nullptr,	//119
	nullptr,	//120
	nullptr,	//121
	nullptr,	//122
	nullptr,	//123
	nullptr,	//124
	nullptr,	//125
	nullptr,	//126
	nullptr,	//127
	nullptr,	//128
	nullptr,	//129
	nullptr,	//130
	nullptr,	//131
	nullptr,	//132
	nullptr,	//133
	nullptr,	//134
	nullptr,	//135
	nullptr,	//136
	nullptr,	//137
	nullptr,	//138
	nullptr,	//139
	nullptr,	//140
	nullptr,	//141
	nullptr,	//142
	nullptr,	//143
	nullptr,	//144
	nullptr,	//145
	nullptr,	//146
	nullptr,	//147
	nullptr,	//148
	nullptr,	//149
	nullptr,	//150
	nullptr,	//151
	nullptr,	//152
	nullptr,	//153
	nullptr,	//154
	nullptr,	//155
	nullptr,	//156
	nullptr,	//157
	nullptr,	//158
	nullptr,	//159
	nullptr,	//160
	nullptr,	//161
	nullptr,	//162
	nullptr,	//163
	nullptr,	//164
	nullptr,	//165
	nullptr,	//166
	nullptr,	//167
	nullptr,	//168
	nullptr,	//169
	nullptr,	//170
	nullptr,	//171
	nullptr,	//172
	nullptr,	//173
	nullptr,	//174
	nullptr,	//175
	nullptr,	//176
	nullptr,	//177
	nullptr,	//178
	nullptr,	//179
	nullptr,	//180
	nullptr,	//181
	nullptr,	//182
	nullptr,	//183
	nullptr,	//184
	nullptr,	//185
	nullptr,	//186
	nullptr,	//187
	nullptr,	//188
	nullptr,	//189
	nullptr,	//190
	nullptr,	//191
	nullptr,	//192
	nullptr,	//193
	nullptr,	//194
	nullptr,	//195
	nullptr,	//196
};

static_assert(static_cast<size_t>(InstMnemonic::Mtfsb1) == _countof(insts));

void
PPUCycle(
	PPU& ppu) {
	ppu.inst = ppu.guestMem->Read32(ppu.reg.pc);

#if 1
	PrintInstruction(ppu.inst.raw, ppu.reg.pc);
#endif

	ppu.reg.pc += 4;

	if (!DecodeInstruction(ppu.inst, ppu.instData)) {
		printf("Failed to decode instruction!\n");
		return;
	}

	PPUFetchOperands(ppu);

	const instructionOp op{ insts[static_cast<uint32_t>(ppu.instData.mnemonic)] };
	if (!op) return;

	op(ppu);
}
