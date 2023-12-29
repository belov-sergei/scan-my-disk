// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include <imgui.h>
#include <Localization.h>
#include <Filesystem.h>
#include <pugixml.hpp>

template <typename = void>
struct Settings {};

struct User {};

template <>
struct Settings<User> {
	inline static auto Language = Localization::Id("English");

	static void Load(pugi::xml_node root) {
		const auto user = root.child("User");
		const auto language = user.child("Language");

		Language = language.attribute("Value").as_uint(Language);
	}

	static void Save(pugi::xml_node root) {
		auto user = root.append_child("User");
		auto language = user.append_child("Language");

		auto value = language.append_attribute("Value");
		value.set_value(Language);
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
		const auto settings = Filesystem::GetLocalSettingsPath();
		if (!std::filesystem::exists(settings)) {
			std::filesystem::path path = settings;
			path.remove_filename();

			std::filesystem::create_directories(path);
			Save();
		}

		pugi::xml_document xml;
		xml.load_file(settings.c_str());

		Settings<User>::Load(xml.document_element());
	}

	static void Save() {
		const auto settings = Filesystem::GetLocalSettingsPath();

		pugi::xml_document xml;
		const auto xmlRoot = xml.append_child("Settings");

		Settings<User>::Save(xmlRoot);
		std::ignore = xml.save_file(settings.c_str());
	}
};
