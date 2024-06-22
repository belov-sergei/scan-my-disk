// Copyright ❤️ 2023-2024, Sergei Belov

#include "Application.h"
#include "Image.h"
#include "IMGUIFontComponent.Generated.h"

IMGUIFontComponent::IMGUIFontComponent() {
	Event<Application::Launch>::Receive(this, [this](const Application::Launch&) {
		ImFontAtlas* Fonts = ImGui::GetIO().Fonts;
		int ignore = 0;

		Fonts->AddFontDefault();

		const auto addCustomGlyph = [&, defaultFont = ImGui::GetDefaultFont()](Glyph& glyph) {
			glyph.index = Fonts->AddCustomRectFontGlyph(defaultFont, glyph.codepoint, glyph.width, glyph.height, glyph.advanceX, glyph.offset);
		};

		std::for_each(std::begin(Glyphs), std::end(Glyphs), addCustomGlyph);
		Fonts->Build();

		ImageData baseImage = {};
		baseImage.data = Image::Load(TextureData, sizeof(TextureData), baseImage.stride, ignore, 1);

		ImageData targetImage = {};
		Fonts->GetTexDataAsRGBA32(&targetImage.data, &targetImage.stride, &ignore);

		const auto copyGlyphsToTexture = [&](const Glyph& glyph) {
			const auto* rectangle = Fonts->GetCustomRectByIndex(glyph.index);

			const BaseImageIterator baseImageIterator = { glyph.x, glyph.y, glyph.width, glyph.height, baseImage.stride, baseImage.data };
			const TargetImageIterator targetImageIterator = { rectangle->X, rectangle->Y, rectangle->Width, rectangle->Height, targetImage.stride, (ImU32*)targetImage.data };

			std::transform<BaseImageIterator, TargetImageIterator>(baseImageIterator, {}, targetImageIterator, [](auto alpha) {
				return IM_COL32(255, 255, 255, alpha);
			});
		};

		std::for_each(std::begin(Glyphs), std::end(Glyphs), copyGlyphsToTexture);
		Image::Free(baseImage.data);
	});
}
