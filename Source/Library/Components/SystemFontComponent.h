// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "Application.h"
#include "FontCollection.h"
#include "Identifier.h"

struct SystemFontComponent final {
	inline static const Identifier FontId = Identifier::Next();

	SystemFontComponent() {
		Event<Application::Loading>::Receive(this, [](const Application::Loading&) {
			Storage<FontCollection>::Write([](FontCollection& collection) {
				collection.CreateFont(FontId, GetBinaryFontData());
			});
		});

		Event<Application::Terminate>::Receive(this, [](const Application::Terminate&) {
			Storage<FontCollection>::Write([](FontCollection& collection) {
				collection.DeleteFont(FontId);
			});
		});
	}

private:
	static FontCollection::BinaryFontDataType GetBinaryFontData();
};
