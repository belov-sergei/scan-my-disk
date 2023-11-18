#pragma once
#include <Utilities.h>

namespace Localization {
	enum class Language : uint32_t {
		English = Utilities::CRC32("ENGLISH_LANGUAGE"),
		French = Utilities::CRC32("FRENCH_LANGUAGE"),
		Spanish = Utilities::CRC32("SPANISH_LANGUAGE"),
		Chinese = Utilities::CRC32("CHINESE_LANGUAGE"),
		Russian = Utilities::CRC32("RUSSIAN_LANGUAGE"),

	};

	class Text {
	public:
		constexpr Text(std::string_view textId) {
			_value = Utilities::CRC32(textId);
		}

		constexpr Text(uint32_t value) {
			_value = value;
		}

		constexpr Text(Language language) {
			_value = static_cast<uint32_t>(language);
		}

		operator std::string() const;
		operator std::string_view() const;
		operator const char *() const;
		operator uint32_t() const;

		static void SetLanguage(Language value) {
			_language = value;
		}

		static Language GetLanguage() {
			return _language;
		}

	private:
		uint32_t _value;
		inline static Language _language = Language::English;
	};
} // namespace Localization
