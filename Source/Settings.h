#pragma once
#include <imgui.h>
#include <Localization.h>
#include <pugixml.hpp>

template <typename = void>
struct Settings {};

struct User {};

template <>
struct Settings<User> {
	inline static auto Language = Localization::Language::English;

	static void Load(pugi::xml_node root) {
		const auto user = root.child("User");
		const auto language = user.child("Language");

		const auto value = language.attribute("Value").as_ullong(static_cast<unsigned long long>(Language));
		Language = static_cast<Localization::Language>(value);
	}

	static void Save(pugi::xml_node root) {
		auto user = root.append_child("User");
		auto language = user.append_child("Language");

		auto value = language.append_attribute("Value");
		value.set_value(static_cast<unsigned long long>(Language));
	}
};

struct Color {};

template <>
struct Settings<Color> {
	inline static auto Text = IM_COL32(190, 190, 190, 255);
};

template <>
struct Settings<void> {
	static void Load() {
		if (!std::filesystem::exists("Settings.xml")) {
			Save();
		}

		pugi::xml_document xml;
		xml.load_file("Settings.xml");

		Settings<User>::Load(xml.document_element());
	}

	static void Save() {
		pugi::xml_document xml;
		const auto xmlRoot = xml.append_child("Settings");

		Settings<User>::Save(xmlRoot);

		std::ignore = xml.save_file("Settings.xml");
	}
};
