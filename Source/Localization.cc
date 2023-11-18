#include <Localization.h>

namespace Localization {
	namespace Details {
		// clang-format off
		std::unordered_map<Language, std::unordered_map<uint32_t, std::string>> Translations =
		{
			{
				Language::English,
				{
					{Text("ABORT_LOADING_BUTTON"), "Abort"},
					{Text("EXPLORE_BUTTON"), "Explore"},
					{Text("ENGLISH_LANGUAGE"), "English"},
					{Text("FRENCH_LANGUAGE"), "Français"},
					{Text("SPANISH_LANGUAGE"), "Español"},
					{Text("CHINESE_LANGUAGE"), "简体中文"},
					{Text("RUSSIAN_LANGUAGE"), "Русский"},
				}
			},
			{
				Language::French,
				{
					{Text("ABORT_LOADING_BUTTON"), "Abandonner"},
					{Text("EXPLORE_BUTTON"), "Explorer"},
					{Text("ENGLISH_LANGUAGE"), "English"},
					{Text("FRENCH_LANGUAGE"), "Français"},
					{Text("SPANISH_LANGUAGE"), "Español"},
					{Text("CHINESE_LANGUAGE"), "简体中文"},
					{Text("RUSSIAN_LANGUAGE"), "Русский"},
				}
			},
			{
				Language::Spanish,
				{
					{Text("ABORT_LOADING_BUTTON"), "Cancelar"},
					{Text("EXPLORE_BUTTON"), "Explorar"},
					{Text("ENGLISH_LANGUAGE"), "English"},
					{Text("FRENCH_LANGUAGE"), "Français"},
					{Text("SPANISH_LANGUAGE"), "Español"},
					{Text("CHINESE_LANGUAGE"), "简体中文"},
					{Text("RUSSIAN_LANGUAGE"), "Русский"},
				}
			},
			{
				Language::Chinese,
				{
					{Text("ABORT_LOADING_BUTTON"), "中止"},
					{Text("EXPLORE_BUTTON"), "探索"},
					{Text("ENGLISH_LANGUAGE"), "English"},
					{Text("FRENCH_LANGUAGE"), "Français"},
					{Text("SPANISH_LANGUAGE"), "Español"},
					{Text("CHINESE_LANGUAGE"), "简体中文"},
					{Text("RUSSIAN_LANGUAGE"), "Русский"},
				}
			},
			{
				Language::Russian,
				{
					{Text("ABORT_LOADING_BUTTON"), "Прервать"},
					{Text("EXPLORE_BUTTON"), "Обозреватель"},
					{Text("ENGLISH_LANGUAGE"), "English"},
					{Text("FRENCH_LANGUAGE"), "Français"},
					{Text("SPANISH_LANGUAGE"), "Español"},
					{Text("CHINESE_LANGUAGE"), "简体中文"},
					{Text("RUSSIAN_LANGUAGE"), "Русский"},
				}
			},
		};
		// clang-format on
	} // namespace Details

	Text::operator std::string() const {
		return Details::Translations[_language][_value];
	}

	Text::operator std::string_view() const {
		return Details::Translations[_language][_value];
	}

	Text::operator const char *() const {
		return Details::Translations[_language][_value].c_str();
	}

	Text::operator uint32_t() const {
		return _value;
	}
} // namespace Localization
