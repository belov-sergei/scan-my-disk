// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include <pugixml.hpp>

#include "Application.h"
#include "Localization.h"
#include "Settings.h"

struct LocalizationComponent final {
	LocalizationComponent() {
		Event<Application::Initialize>::Receive(this, [](const auto&) {
			pugi::xml_document xml;
			xml.load_file("Translations.xml");

			const auto root = xml.document_element();

			for (const auto language : root) {
				Localization::Language(language.name());

				for (const auto text : language) {
					const auto textId = text.attribute("Id").as_string();
					const auto value = text.attribute("Value").as_string();

					Localization::Text::Add(textId, value);
				}
			}

			Localization::Language(Settings<User>::Language);
		});
	}
};
