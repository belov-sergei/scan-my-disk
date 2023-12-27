#include <gtest/gtest.h>
#include <Localization.cpp>

decltype(auto) Text(std::string_view textId) {
	return static_cast<const std::string&>(Localization::Text(textId));
}

TEST(Localization, SwitchLanguage) {
	Localization::Reset();

	EXPECT_EQ(Localization::Language(), 0);

	Localization::Language("English");

	const auto english = Localization::Language();
	EXPECT_NE(english, 0);

	Localization::Language("Simplified Chinese");
	EXPECT_NE(Localization::Language(), 0);
	EXPECT_NE(Localization::Language(), english);
}

TEST(Localization, LanguagesEnumeration) {
	Localization::Reset();

	EXPECT_EQ(Localization::Language(), 0);
	EXPECT_EQ(std::size(Localization::Languages()), 0);

	std::vector<uint32_t> languages;

	Localization::Language("English");
	languages.emplace_back(Localization::Language());

	Localization::Language("Simplified Chinese");
	languages.emplace_back(Localization::Language());

	EXPECT_EQ(std::size(languages), std::size(Localization::Languages()));

	for (uint32_t languageId : Localization::Languages()) {
		std::erase(languages, languageId);
	}

	EXPECT_EQ(languages.empty(), true);
}

TEST(Localization, Reset) {
	Localization::Language("English");

	EXPECT_NE(Localization::Language(), 0);
	EXPECT_NE(std::size(Localization::Languages()), 0);

	Localization::Text::Add("File", "File");
	EXPECT_EQ(Text("File"), "File");

	Localization::Reset();

	EXPECT_EQ(Localization::Language(), 0);
	EXPECT_EQ(std::size(Localization::Languages()), 0);
	EXPECT_EQ(Text("File"), "");
}

TEST(Localization, TextAddition) {
	Localization::Reset();

	Localization::Language("English");
	Localization::Text::Add("File", "File");
	Localization::Text::Add("Exit", "Exit");

	Localization::Language("Simplified Chinese");
	Localization::Text::Add("File", "文件");
	Localization::Text::Add("Exit", "退出");

	Localization::Language("English");
	EXPECT_EQ(Text("File"), "File");
	EXPECT_EQ(Text("Exit"), "Exit");

	Localization::Language("Simplified Chinese");
	EXPECT_EQ(Text("File"), "文件");
	EXPECT_EQ(Text("Exit"), "退出");
}

TEST(Localization, UnknownLanguage) {
	Localization::Reset();

	Localization::Language("English");
	Localization::Text::Add("File", "File");
	Localization::Text::Add("Exit", "Exit");

	Localization::Language("Unknown Language");
	EXPECT_EQ(Text("File"), "File");
	EXPECT_EQ(Text("Exit"), "Exit");
}

TEST(Localization, EmptyLanguagesList) {
	Localization::Reset();

	EXPECT_EQ(std::size(Localization::Languages()), 0);

	EXPECT_EQ(Text("File"), "");
	EXPECT_EQ(Text("Exit"), "");

	Localization::Language("English");
	EXPECT_EQ(std::size(Localization::Languages()), 1);

	EXPECT_EQ(Text("File"), "");
	EXPECT_EQ(Text("Exit"), "");

	Localization::Text::Add("File", "File");
	Localization::Text::Add("Exit", "Exit");

	EXPECT_EQ(Text("File"), "File");
	EXPECT_EQ(Text("Exit"), "Exit");
}

TEST(Localization, DuplicateText) {
	Localization::Reset();

	Localization::Language("English");
	Localization::Text::Add("File", "Exit");
	Localization::Text::Add("File", "File");

	EXPECT_EQ(Text("File"), "File");
	EXPECT_EQ(Text("Exit"), "");
}

TEST(Localization, DuplicateLanguage) {
	Localization::Reset();

	EXPECT_EQ(std::size(Localization::Languages()), 0);

	Localization::Language("English");
	Localization::Language("English");

	EXPECT_EQ(std::size(Localization::Languages()), 1);

	Localization::Text::Add("File", "File");
	Localization::Text::Add("Exit", "Exit");

	EXPECT_EQ(Text("File"), "File");
	EXPECT_EQ(Text("Exit"), "Exit");
}

TEST(Localization, IncorrectInput) {
	Localization::Reset();

	EXPECT_EQ(std::size(Localization::Languages()), 0);

	Localization::Language("");
	EXPECT_EQ(std::size(Localization::Languages()), 1);

	Localization::Text::Add("", "File");
	EXPECT_EQ(Text(""), "File");

	Localization::Text::Add("", "Exit");
	EXPECT_EQ(Text(""), "Exit");

	Localization::Language("English");
	EXPECT_EQ(std::size(Localization::Languages()), 2);
	EXPECT_EQ(Text(""), "Exit");

	Localization::Text::Add("", "File");
	EXPECT_EQ(Text(""), "File");

	Localization::Language("");
	EXPECT_EQ(Text(""), "Exit");
}

int main(int argc, char* argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
