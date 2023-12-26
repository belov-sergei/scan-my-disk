#include <Localization.h>

namespace Localization {
	namespace Details {
		// The default language for searching translations missing in other languages.
		uint32_t Default = 0;

		// The currently selected language.
		uint32_t Language = 0;

		std::string Empty;

		// Translation table for all added languages.
		std::unordered_map<uint32_t, std::unordered_map<uint32_t, std::string>> Languages;
	} // namespace Details

	void Language(uint32_t language) {
		Details::Language = language;
		Details::Languages[Details::Language];

		// The first added language will be used as default.
		if (std::size(Details::Languages) == 1) {
			Details::Default = Details::Language;
		}
	}

	void Language(std::string_view language) {
		Language(Id(language));
	}

	uint32_t Language() {
		return Details::Language;
	}

	std::vector<uint32_t> Languages() {
		std::vector<uint32_t> result;
		result.reserve(std::size(Details::Languages));

		for (const auto& [key, value] : Details::Languages) {
			result.emplace_back(key);
		}

		return result;
	}

	void Reset() {
		Details::Default = 0;
		Details::Language = 0;
		Details::Languages = {};
	}

	void Text::Add(std::string_view textId, std::string_view value) {
		Details::Languages[Details::Language][Id(textId)] = value;
	}

	Text::operator const std::string &() const {
		if (std::empty(Details::Languages)) {
			return Details::Empty;
		}

		const auto& language = Details::Languages[Details::Language];
		if (language.contains(_id)) {
			return language.at(_id);
		}

		// The current ID was not found. Use default translation.
		return Details::Languages[Details::Default][_id];
	}

	Text::operator std::string_view() const {
		return static_cast<const std::string&>(*this);
	}

	Text::operator const char *() const {
		return static_cast<const std::string&>(*this).c_str();
	}

	Text::operator uint32_t() const {
		return _id;
	}
} // namespace Localization
