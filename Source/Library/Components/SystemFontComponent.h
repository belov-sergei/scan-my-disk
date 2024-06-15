// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "Application.h"
#include "FontCollection.h"
#include "Identifier.h"

struct SystemFontComponent final {
	inline static const Identifier FontId = Identifier::Next();

	SystemFontComponent() {
		Event<Application::Initialize>::Receive(this, [this](const auto&) {
			Storage<FontCollection>::Write([this](FontCollection& collection) {
				collection.CreateFont(FontId, GetBinaryFontData());
			});
		});

		Event<Application::Terminate>::Receive(this, [](const auto&) {
			Storage<FontCollection>::Write([](FontCollection& collection) {
				collection.DeleteFont(FontId);
			});
		});
	}

private:
	static FontCollection::BinaryFontDataType GetBinaryFontData();
};
