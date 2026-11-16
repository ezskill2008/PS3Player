#pragma once
#include <libPs3Core/Include/CoreTypes.h>

template<typename T>
concept is_bitfield_compatible = std::is_integral_v<T> || std::is_enum_v<T>;

namespace detail {
template<typename T>
struct bitfield_underlying { using type = T; };

template<typename T> requires std::is_enum_v<T>
struct bitfield_underlying<T> { using type = std::underlying_type_t<T>; };
}

template<uint32_t Pos, uint32_t Size, typename T, typename Storage = uint32_t>
class BitField {
	static_assert(is_bitfield_compatible<T>, "BitField::T must be an integral or enum type!");
	static_assert(std::is_unsigned_v<Storage>, "BitField::Storage must be unsigned!");

	using U = typename detail::bitfield_underlying<T>::type;

	constexpr static uint32_t storageBits{ sizeof(Storage) * 8 };

	static_assert(Size > 0, "BitField::Size must be > 0!");
	static_assert(Pos + Size <= storageBits, "BitField does not fit in Storage!");
	static_assert(Size <= sizeof(U) * 8, "BitField::Size is larger than T!");

	constexpr static Storage valueMask{ Size == storageBits ? static_cast<Storage>(~Storage{ 0 })
															: static_cast<Storage>((Storage{ 1 } << Size) - 1) };
	constexpr static Storage fieldMask{ static_cast<Storage>(valueMask << Pos) };

public:
	BitField() = default;

	constexpr T Get() const noexcept {
		Storage v{ static_cast<Storage>((storage >> Pos) & valueMask) };

		if constexpr (std::is_signed_v<U>) {
			constexpr Storage signBit{ static_cast<Storage>(Storage{ 1 } << (Size - 1)) };
			v = static_cast<Storage>((v ^ signBit) - signBit);
		}

		return static_cast<T>(static_cast<U>(v));
	}

	constexpr void Set(T value) noexcept {
		const Storage v{ static_cast<Storage>((static_cast<Storage>(value) & valueMask) << Pos) };
		storage = static_cast<Storage>((storage & ~fieldMask) | v);
	}

	constexpr bool Test(uint32_t bit) const noexcept {
		return ((storage >> (Pos + bit)) & 1) != 0;
	}

	constexpr operator T() const noexcept { return Get(); }

	constexpr BitField& operator=(T value) noexcept {
		Set(value);
		return *this;
	}

	template<uint32_t P2, uint32_t S2, typename T2, typename St2>
	constexpr BitField& operator=(const BitField<P2, S2, T2, St2>& o) noexcept {
		Set(static_cast<T>(o.Get()));
		return *this;
	}

	constexpr BitField& operator|=(T v) noexcept { Set(static_cast<T>(static_cast<U>(Get()) | static_cast<U>(v))); return *this; }
	constexpr BitField& operator&=(T v) noexcept { Set(static_cast<T>(static_cast<U>(Get()) & static_cast<U>(v))); return *this; }
	constexpr BitField& operator^=(T v) noexcept { Set(static_cast<T>(static_cast<U>(Get()) ^ static_cast<U>(v))); return *this; }

	constexpr static uint32_t Position() noexcept { return Pos; }
	constexpr static uint32_t Width() noexcept { return Size; }

private:
	Storage	storage;
};
