#pragma once
#include <Utilities.h>

namespace Localization {
	// Calculates the CRC32 of a string, which is considered the numeric ID of the text.
	constexpr uint32_t Id(std::string_view textId) {
		return Utilities::CRC32(textId);
	}

	// Sets the current language using a numerical ID.
	void Language(uint32_t language);

	// Sets the current language using a language name.
	void Language(std::string_view language);

	// Returns the current language.
	uint32_t Language();

	// Returns a list of IDs for all languages that have been added.
	std::vector<uint32_t> Languages();

	// Resets the localization state.
	void Reset();

	// Helper class for creating and retrieving the translation of text by ID..
	struct Text {
		// Adds new text to the current language, which is selected using the Language function.
		static void Add(std::string_view textId, std::string_view value);

		constexpr Text(std::string_view textId) {
			_id = Id(textId);
		}

		constexpr Text(uint32_t textId) {
			_id = textId;
		}

		operator const std::string &() const;
		operator std::string_view() const;
		operator const char *() const;
		operator uint32_t() const;

	private:
		uint32_t _id;
	};
} // namespace Localization
