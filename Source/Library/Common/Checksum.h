// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once

namespace Common {
	class Crc32Kernel final {
		template <auto ... Index>
		constexpr Crc32Kernel(std::index_sequence<Index...>) noexcept {
			for (auto& value : _table) {
				value = static_cast<uint32_t>(&value - _table.data());
				((std::ignore = Index, value = value & 1 ? value >> 1 ^ 0xedb88320 : value >> 1), ...);
			}
		}

	public:
		constexpr Crc32Kernel() noexcept
			: Crc32Kernel(std::make_index_sequence<8>()) {}

		constexpr uint32_t operator()(std::string_view string) const {
			uint32_t result = ~0;
			for (const auto value : string) {
				result = _table[(result ^ value) & 255] ^ result >> 8;
			}
			return ~result;
		}

	private:
		std::array<uint32_t, 256> _table = {};
	};

	constexpr Crc32Kernel Checksum;
}
