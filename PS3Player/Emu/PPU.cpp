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

namespace rot {
inline uint64_t
rotl32dup(uint64_t x, uint64_t n) {
	const uint64_t w{ x & 0xFFFFFFFFull };
	return std::rotl((w << 32) | w, static_cast<int>(n & 31));
}

inline uint64_t
mask64(uint64_t mb, uint64_t me) {
	const uint64_t begin{ ~0ull >> mb };
	const uint64_t end{ ~0ull << (63 - me) };
	return (mb <= me) ? (begin & end) : (begin | end);
}
}

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

inline void
updateRc(PPU& ppu, uint64_t r) {
	if (ppu.inst.rc) {
		cr::setCRFixedPoint(ppu, static_cast<int64_t>(r), (ppu.reg.xer & XER_SO) != 0);
	}
}
}

namespace branch {
void
b(PPU& ppu) {
	const uint32_t cia{ ppu.reg.pc };
	const uint32_t nia{ ppu.inst.BranchTargetLI(cia) };

	if (ppu.inst.lk) {
		ppu.reg.lr = cia + 4;
	}

	ppu.reg.pc = nia;
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
	DEFINE_OPS(rs, d, ra);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ ra == 0 ? 0 : gpr[ra] };
	const uint64_t ea{ b + static_cast<uint64_t>(static_cast<int64_t>(static_cast<int16_t>(d))) };

	ppu.guestMem->Write8(static_cast<uint32_t>(ea), static_cast<uint8_t>(gpr[rs]));
}

void
stbu(PPU& ppu) {
	DEFINE_OPS(rs, d, ra);
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
	DEFINE_OPS(rs, d, ra);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ ra == 0 ? 0 : gpr[ra] };
	const uint64_t ea{ b + static_cast<uint64_t>(static_cast<int64_t>(static_cast<int16_t>(d))) };

	ppu.guestMem->Write16(static_cast<uint32_t>(ea), static_cast<uint16_t>(gpr[rs]));
}

void
sthu(PPU& ppu) {
	DEFINE_OPS(rs, d, ra);
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
	DEFINE_OPS(rs, d, ra);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t b{ ra == 0 ? 0 : gpr[ra] };
	const uint64_t ea{ b + static_cast<uint64_t>(static_cast<int64_t>(static_cast<int16_t>(d))) };

	ppu.guestMem->Write32(static_cast<uint32_t>(ea), static_cast<uint32_t>(gpr[rs]));
}

void
stwu(PPU& ppu) {
	DEFINE_OPS(rs, d, ra);
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
#pragma region fixed point load/store byte reversed
void
lhbrx(PPU& ppu) {
	DEFINE_OPS(rd, ra, rb);
	auto& gpr{ ppu.reg.gpr };

	const uint64_t b{ ra == 0 ? 0 : gpr[ra] };
	const uint64_t ea{ b + gpr[rb] };

	gpr[rd] = _byteswap_ushort(ppu.guestMem->Read16(static_cast<uint32_t>(ea)));
}

void
lwbrx(PPU& ppu) {
	DEFINE_OPS(rd, ra, rb);
	auto& gpr{ ppu.reg.gpr };

	const uint64_t b{ ra == 0 ? 0 : gpr[ra] };
	const uint64_t ea{ b + gpr[rb] };

	gpr[rd] = _byteswap_ulong(ppu.guestMem->Read32(static_cast<uint32_t>(ea)));
}

void
sthbrx(PPU& ppu) {
	DEFINE_OPS(rd, ra, rb);
	auto& gpr{ ppu.reg.gpr };

	const uint64_t b{ ra == 0 ? 0 : gpr[ra] };
	const uint64_t ea{ b + gpr[rb] };

	ppu.guestMem->Write16(static_cast<uint32_t>(ea), _byteswap_ushort(static_cast<uint16_t>(gpr[rd])));
}

void
stwbrx(PPU& ppu) {
	DEFINE_OPS(rd, ra, rb);
	auto& gpr{ ppu.reg.gpr };

	const uint64_t b{ ra == 0 ? 0 : gpr[ra] };
	const uint64_t ea{ b + gpr[rb] };

	ppu.guestMem->Write32(static_cast<uint32_t>(ea), _byteswap_ulong(static_cast<uint32_t>(gpr[rd])));
}

#pragma endregion
#pragma region fixed point arithmetic - check all insts!!!!
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

#pragma endregion
#pragma region fixed point logical
void
andi(PPU& ppu) {
	DEFINE_OPS(ra, rs, uimm);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t s{ gpr[rs] };
	const uint64_t ui{ static_cast<uint64_t>(static_cast<uint16_t>(uimm)) };
	const uint64_t r{ s & ui };

	gpr[ra] = r;

	cr::setCRFixedPoint(ppu, static_cast<int64_t>(r), (ppu.reg.xer & XER_SO) != 0);
}

void
andis(PPU& ppu) {
	DEFINE_OPS(ra, rs, uimm);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t s{ gpr[rs] };
	const uint64_t ui{ static_cast<uint64_t>(static_cast<uint16_t>(uimm)) << 16 };
	const uint64_t r{ s & ui };

	gpr[ra] = r;

	cr::setCRFixedPoint(ppu, static_cast<int64_t>(r), (ppu.reg.xer & XER_SO) != 0);
}

void
ori(PPU& ppu) {
	DEFINE_OPS(ra, rs, uimm);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t s{ gpr[rs] };
	const uint64_t ui{ static_cast<uint64_t>(static_cast<uint16_t>(uimm)) };
	const uint64_t r{ s | ui };

	gpr[ra] = r;
}

void
oris(PPU& ppu) {
	DEFINE_OPS(ra, rs, uimm);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t s{ gpr[rs] };
	const uint64_t ui{ static_cast<uint64_t>(static_cast<uint16_t>(uimm)) << 16 };
	const uint64_t r{ s | ui };

	gpr[ra] = r;
}

void
xori(PPU& ppu) {
	DEFINE_OPS(ra, rs, uimm);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t s{ gpr[rs] };
	const uint64_t ui{ static_cast<uint64_t>(static_cast<uint16_t>(uimm)) };
	const uint64_t r{ s ^ ui };

	gpr[ra] = r;
}

void
xoris(PPU& ppu) {
	DEFINE_OPS(ra, rs, uimm);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t s{ gpr[rs] };
	const uint64_t ui{ static_cast<uint64_t>(static_cast<uint16_t>(uimm)) << 16 };
	const uint64_t r{ s ^ ui };

	gpr[ra] = r;
}

void
andx(PPU& ppu) {
	DEFINE_OPS(ra, rs, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t r{ gpr[rs] & gpr[rb] };

	gpr[ra] = r;

	cr::updateRc(ppu, r);
}

void
orx(PPU& ppu) {
	DEFINE_OPS(ra, rs, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t r{ gpr[rs] | gpr[rb] };

	gpr[ra] = r;

	cr::updateRc(ppu, r);
}

void
xorx(PPU& ppu) {
	DEFINE_OPS(ra, rs, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t r{ gpr[rs] ^ gpr[rb] };

	gpr[ra] = r;

	cr::updateRc(ppu, r);
}

void
nand(PPU& ppu) {
	DEFINE_OPS(ra, rs, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t r{ ~(gpr[rs] & gpr[rb]) };

	gpr[ra] = r;

	cr::updateRc(ppu, r);
}

void
nor(PPU& ppu) {
	DEFINE_OPS(ra, rs, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t r{ ~(gpr[rs] | gpr[rb]) };

	gpr[ra] = r;

	cr::updateRc(ppu, r);
}

void
andc(PPU& ppu) {
	DEFINE_OPS(ra, rs, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t r{ gpr[rs] & ~gpr[rb] };

	gpr[ra] = r;

	cr::updateRc(ppu, r);
}

void
eqv(PPU& ppu) {
	DEFINE_OPS(ra, rs, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t r{ ~(gpr[rs] ^ gpr[rb]) };

	gpr[ra] = r;

	cr::updateRc(ppu, r);
}

void
orc(PPU& ppu) {
	DEFINE_OPS(ra, rs, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t r{ gpr[rs] | ~gpr[rb] };

	gpr[ra] = r;

	cr::updateRc(ppu, r);
}

void
extsb(PPU& ppu) {
	DEFINE_OPS(ra, rs);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t r{ static_cast<uint64_t>(static_cast<int64_t>(static_cast<int8_t>(gpr[rs]))) };

	gpr[ra] = r;

	cr::updateRc(ppu, r);
}

void
extsh(PPU& ppu) {
	DEFINE_OPS(ra, rs);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t r{ static_cast<uint64_t>(static_cast<int64_t>(static_cast<int16_t>(gpr[rs]))) };

	gpr[ra] = r;

	cr::updateRc(ppu, r);
}

void
extsw(PPU& ppu) {
	DEFINE_OPS(ra, rs);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t r{ static_cast<uint64_t>(static_cast<int64_t>(static_cast<int32_t>(gpr[rs]))) };

	gpr[ra] = r;

	cr::updateRc(ppu, r);
}

void
cntlzd(PPU& ppu) {
	DEFINE_OPS(ra, rs);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t r{ __lzcnt64(gpr[rs]) };

	gpr[ra] = r;

	cr::updateRc(ppu, r);
}

void
cntlzw(PPU& ppu) {
	DEFINE_OPS(ra, rs);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t r{ __lzcnt(static_cast<uint32_t>(gpr[rs])) };

	gpr[ra] = r;

	cr::updateRc(ppu, r);
}

void
popcntb(PPU& ppu) {
	DEFINE_OPS(ra, rs);
	auto& gpr{ ppu.reg.gpr };

	uint64_t x{ gpr[rs] };
	x = x - ((x >> 1) & 0x5555555555555555ull);
	x = (x & 0x3333333333333333ull) + ((x >> 2) & 0x3333333333333333ull);
	x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0Full;

	gpr[ra] = x;
}

#pragma endregion
#pragma region fixed point rotate and shift
void
rlwinm(PPU& ppu) {
	DEFINE_OPS(ra, rs, sh, mb, me);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t r{ rot::rotl32dup(gpr[rs], sh) & rot::mask64(mb + 32, me + 32) };

	gpr[ra] = r;
	cr::updateRc(ppu, r);
}

void
rlwnm(PPU& ppu) {
	DEFINE_OPS(ra, rs, rb, mb, me);
	auto& gpr{ ppu.reg.gpr };
	const uint32_t n{ static_cast<uint32_t>(gpr[rb] & 0x1F) };
	const uint64_t r{ rot::rotl32dup(gpr[rs], n) & rot::mask64(mb + 32, me + 32) };

	gpr[ra] = r;
	cr::updateRc(ppu, r);
}

void
rlwimi(PPU& ppu) {
	DEFINE_OPS(ra, rs, sh, mb, me);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t m{ rot::mask64(mb + 32, me + 32) };
	const uint64_t r{ (rot::rotl32dup(gpr[rs], sh) & m) | (gpr[ra] & ~m) };

	gpr[ra] = r;
	cr::updateRc(ppu, r);
}

void
rldicl(PPU& ppu) {
	DEFINE_OPS(ra, rs);
	auto& gpr{ ppu.reg.gpr };
	const uint32_t sh{ ppu.inst.Sh64() };
	const uint32_t mb{ ppu.inst.Mb64() };
	const uint64_t r{ std::rotl(gpr[rs], static_cast<int32_t>(sh)) & rot::mask64(mb, 63) };

	gpr[ra] = r;
	cr::updateRc(ppu, r);
}

void
rldicr(PPU& ppu) {
	DEFINE_OPS(ra, rs);
	auto& gpr{ ppu.reg.gpr };
	const uint32_t sh{ ppu.inst.Sh64() };
	const uint32_t me{ ppu.inst.Me64() };
	const uint64_t r{ std::rotl(gpr[rs], static_cast<int32_t>(sh)) & rot::mask64(0, me) };

	gpr[ra] = r;
	cr::updateRc(ppu, r);
}

void
rldic(PPU& ppu) {
	DEFINE_OPS(ra, rs);
	auto& gpr{ ppu.reg.gpr };
	const uint32_t sh{ ppu.inst.Sh64() };
	const uint32_t mb{ ppu.inst.Mb64() };
	const uint64_t r{ std::rotl(gpr[rs], static_cast<int32_t>(sh)) & rot::mask64(mb, 63 - sh) };

	gpr[ra] = r;
	cr::updateRc(ppu, r);
}

void
rldimi(PPU& ppu) {
	DEFINE_OPS(ra, rs);
	auto& gpr{ ppu.reg.gpr };
	const uint32_t sh{ ppu.inst.Sh64() };
	const uint32_t mb{ ppu.inst.Mb64() };
	const uint64_t m{ rot::mask64(mb, 63 - sh) };
	const uint64_t r{ (std::rotl(gpr[rs], static_cast<int32_t>(sh)) & m) | (gpr[ra] & ~m) };

	gpr[ra] = r;
	cr::updateRc(ppu, r);
}

void
rldcl(PPU& ppu) {
	DEFINE_OPS(ra, rs, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint32_t n{ static_cast<uint32_t>(gpr[rb] & 0x3F) };
	const uint32_t mb{ ppu.inst.Mb64() };
	const uint64_t r{ std::rotl(gpr[rs], static_cast<int32_t>(n)) & rot::mask64(mb, 63) };

	gpr[ra] = r;
	cr::updateRc(ppu, r);
}

void
rldcr(PPU& ppu) {
	DEFINE_OPS(ra, rs, rb);
	auto& gpr{ ppu.reg.gpr };
	const uint32_t n{ static_cast<uint32_t>(gpr[rb] & 0x3F) };
	const uint32_t me{ ppu.inst.Me64() };
	const uint64_t r{ std::rotl(gpr[rs], static_cast<int32_t>(n)) & rot::mask64(0, me) };

	gpr[ra] = r;
	cr::updateRc(ppu, r);
}

#pragma endregion
#pragma region move to/from system registers
void
mtspr(PPU& ppu) {
	DEFINE_OPS(spr, rs);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t n{ ppu.inst.Spr() };
	constexpr uint64_t XER_MASK{ 0xE000007Full };

	switch (n) {
	case 1:   ppu.reg.xer = gpr[rs] & XER_MASK; break;
	case 8:   ppu.reg.lr = gpr[rs]; break;
	case 9:   ppu.reg.ctr = gpr[rs]; break;
	case 256: ppu.reg.vrsave = static_cast<uint32_t>(gpr[rs]); break;
	default: break;
	}
}

void
mfspr(PPU& ppu) {
	DEFINE_OPS(spr, rs);
	auto& gpr{ ppu.reg.gpr };
	const uint64_t n{ ppu.inst.Spr() };
	constexpr uint64_t XER_MASK{ 0xE000007Full };

	switch (n) {
	case 1:   gpr[rs] = ppu.reg.xer & XER_MASK; break;
	case 8:   gpr[rs] = ppu.reg.lr; break;
	case 9:   gpr[rs] = ppu.reg.ctr; break;
	case 256: gpr[rs] = ppu.reg.vrsave; break;
	default: break;
	}
}

void
mtcrf(PPU& ppu) {
	DEFINE_OPS(crm, rs);
	auto& gpr{ ppu.reg.gpr };

	uint32_t mask{ 0 };
	for (uint32_t i{ 0 }; i < 8; ++i) {
		if (crm & (0x80u >> i)) {
			mask |= 0xFu << (28 - 4 * i);
		}
	}

	const uint32_t value{ static_cast<uint32_t>(gpr[rs]) };
	ppu.reg.cr = (ppu.reg.cr & ~mask) | (value & mask);
}

void
mfcr(PPU& ppu) {
	DEFINE_OPS(rd);
	ppu.reg.gpr[rd] = static_cast<uint64_t>(ppu.reg.cr);
}

#pragma endregion
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
	branch::b,		//13
	fixedp::rlwimi,	//14
	fixedp::rlwinm,	//15
	fixedp::rlwnm,	//16
	fixedp::ori,	//17
	fixedp::oris,	//18
	fixedp::xori,	//19
	fixedp::xoris,	//20
	fixedp::andi,	//21
	fixedp::andis,	//22
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
	fixedp::rldicl,	//63
	fixedp::rldicr,	//64
	fixedp::rldic,	//65
	fixedp::rldimi,	//66
	fixedp::rldcl,	//67
	fixedp::rldcr,	//68
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
	fixedp::lhbrx,	//93
	fixedp::lwbrx,	//94
	fixedp::sthbrx,	//95
	fixedp::stwbrx,	//96
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
	fixedp::andx,	//122
	fixedp::andc,	//123
	fixedp::orx,	//124
	fixedp::orc,	//125
	fixedp::xorx,	//126
	fixedp::nand,	//127
	fixedp::nor,	//128
	fixedp::eqv,	//129
	fixedp::extsb,	//130
	fixedp::extsh,	//131
	fixedp::extsw,	//132
	fixedp::cntlzw,	//133
	fixedp::cntlzd,	//134
	nullptr,	//135
	nullptr,	//136
	nullptr,	//137
	nullptr,	//138
	nullptr,	//139
	nullptr,	//140
	nullptr,	//141
	nullptr,	//142
	fixedp::mtspr,	//143
	fixedp::mfspr,	//144
	fixedp::mfcr,	//145
	fixedp::mtcrf,	//146
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

bool
PPUCycle(
	PPU& ppu) {
	ppu.inst = ppu.guestMem->Read32(ppu.reg.pc);

#if 1
	PrintInstruction(ppu.inst.raw, ppu.reg.pc);
#endif

	ppu.reg.pc += 4;

	if (!DecodeInstruction(ppu.inst, ppu.instData)) {
		printf("Failed to decode instruction!\n");
		return false;
	}

	PPUFetchOperands(ppu);

	const instructionOp op{ insts[static_cast<uint32_t>(ppu.instData.mnemonic)] };
	if (!op) {
		printf("Above instruction is not implemented!");
		return false;
	}

	op(ppu);

	return true;
}
