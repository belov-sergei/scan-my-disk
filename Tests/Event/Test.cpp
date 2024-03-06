// Copyright ❤️ 2023-2024, Sergei Belov

#include <gtest/gtest.h>
#include <Event.h>

struct TestEvent {
	int counter = 0;
};

TEST(Event, Test) {
	TestEvent myEvent;
	myEvent.counter = 5;

	Event<TestEvent>::Receive(nullptr, [](TestEvent& event) {
		std::cout << event.counter << std::endl;
	});

	Event<TestEvent>::Send(myEvent);
}

int main(int argc, char* argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
