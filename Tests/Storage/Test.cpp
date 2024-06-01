// Copyright ❤️ 2023-2024, Sergei Belov

#include <gtest/gtest.h>
#include <Parallel.h>
#include <Storage.h>

struct TestStorage {
	using DataType = std::vector<int>;
};

TEST(Storage, Write) {
	Storage<TestStorage>::Write([](TestStorage::DataType& storage) {
		storage.clear();

		for (int number : { 1, 2, 3 }) {
			storage.emplace_back(number);
		}

		EXPECT_EQ(storage.size(), 3);
	});

	Storage<TestStorage>::Write([](TestStorage::DataType& storage) {
		EXPECT_EQ(storage.size(), 3);

		for (int number : { 1, 2, 3 }) {
			EXPECT_EQ(storage[number - 1], number);
		}

		storage.clear();
		EXPECT_TRUE(storage.empty());
	});

	Storage<TestStorage>::Write([](const TestStorage::DataType& storage) {
		EXPECT_TRUE(storage.empty());
	});
}

TEST(Storage, Read) {
	Storage<TestStorage>::Read([](const TestStorage::DataType& storage) {
		EXPECT_TRUE(storage.empty());
	});

	Storage<TestStorage>::Write([](TestStorage::DataType& storage) {
		EXPECT_TRUE(storage.empty());

		for (int number : { 1, 2, 3 }) {
			storage.emplace_back(number);
		}
	});

	Storage<TestStorage>::Read([](const TestStorage::DataType& storage) {
		EXPECT_EQ(storage.size(), 3);

		for (int number : { 1, 2, 3 }) {
			EXPECT_EQ(storage[number - 1], number);
		}
	});
}

TEST(Storage, Recursion) {
	Storage<TestStorage>::Write([](TestStorage::DataType& storage) {
		storage.clear();

		Storage<TestStorage>::Write([](TestStorage::DataType& storage) {
			EXPECT_TRUE(storage.empty());
			for (int number : { 1, 2, 3 }) {
				storage.emplace_back(number);
			}
		});

		EXPECT_EQ(storage.size(), 3);
	});

	Storage<TestStorage>::Read([](const TestStorage::DataType& storage) {
		EXPECT_EQ(storage.size(), 3);

		Storage<TestStorage>::Read([](const TestStorage::DataType& storage) {
			for (int number : { 1, 2, 3 }) {
				EXPECT_EQ(storage[number - 1], number);
			}
		});
	});

	Storage<TestStorage>::Read([](const TestStorage::DataType& storage) {
		for (int number : { 1, 2, 3 }) {
			EXPECT_EQ(storage[number - 1], number);
		}

		Storage<TestStorage>::Write([](TestStorage::DataType& storage) {
			storage.clear();
			EXPECT_TRUE(storage.empty());

			for (int number : { 1, 2, 3 }) {
				storage.emplace_back(number);
			}

			EXPECT_EQ(storage.size(), 3);
		});

		for (int number : { 1, 2, 3 }) {
			EXPECT_EQ(storage[number - 1], number);
		}
	});

	Storage<TestStorage>::Write([](TestStorage::DataType& storage) {
		storage.clear();
		EXPECT_TRUE(storage.empty());

		for (int number : { 1, 2, 3 }) {
			storage.emplace_back(number);
		}

		EXPECT_EQ(storage.size(), 3);

		Storage<TestStorage>::Read([](const TestStorage::DataType& storage) {
			for (int number : { 1, 2, 3 }) {
				EXPECT_EQ(storage[number - 1], number);
			}
		});
	});

	Storage<TestStorage>::Read([](const TestStorage::DataType& storage) {
		Storage<TestStorage>::Write([](TestStorage::DataType& storage) {
			storage.clear();
			EXPECT_TRUE(storage.empty());

			Storage<TestStorage>::Read([](const TestStorage::DataType& storage) {
				Storage<TestStorage>::Read([](const TestStorage::DataType& storage) {
					EXPECT_EQ(storage.size(), 0);

					Storage<TestStorage>::Write([](TestStorage::DataType& storage) {
						storage.emplace_back(1);
						storage.emplace_back(2);

						Storage<TestStorage>::Read([](const TestStorage::DataType& storage) {
							EXPECT_EQ(storage.size(), 2);

							Storage<TestStorage>::Write([](TestStorage::DataType& storage) {
								Storage<TestStorage>::Write([](TestStorage::DataType& storage) {
									storage.emplace_back(3);

									Storage<TestStorage>::Read([](const TestStorage::DataType& storage) {
										EXPECT_EQ(storage.size(), 3);

										for (int number : { 1, 2, 3 }) {
											EXPECT_EQ(storage[number - 1], number);
										}
									});
								});
							});
						});
					});
				});
			});
		});
	});
}

TEST(Storage, MultithreadedWrite) {
	Storage<TestStorage>::Write([](TestStorage::DataType& storage) {
		storage.clear();
		EXPECT_TRUE(storage.empty());
	});

	size_t writers    = 0;
	const auto worker = [&] {
		Storage<TestStorage>::Write([&](TestStorage::DataType& storage) {
			EXPECT_EQ(++writers, 1);

			const size_t size = storage.size();
			for (int number : { 1, 2, 3 }) {
				storage.emplace_back(number);
			}

			EXPECT_EQ(storage.size(), size + 3);
			EXPECT_EQ(--writers, 0);
		});
	};

	std::vector<std::thread> threads(100);

	for (auto& thread : threads) {
		thread = std::thread(worker);
	}

	for (auto& thread : threads) {
		thread.join();
	}

	EXPECT_EQ(writers, 0);

	Storage<TestStorage>::Write([](TestStorage::DataType& storage) {
		EXPECT_EQ(storage.size(), 300);

		int expected = 0;
		for (int number : storage) {
			EXPECT_EQ(number, 1 + expected++ % 3);
		}
	});
}

TEST(Storage, MultithreadedRead) {
	Storage<TestStorage>::Write([](TestStorage::DataType& storage) {
		storage.clear();
		EXPECT_TRUE(storage.empty());

		for (int number : { 1, 2, 3, 4, 5 }) {
			storage.emplace_back(number);
		}

		EXPECT_EQ(storage.size(), 5);
	});

	std::mutex mutex;

	size_t readers = 0;
	size_t maximum = 0;

	const auto worker = [&] {
		Storage<TestStorage>::Read([&](const TestStorage::DataType& storage) {
			{
				std::unique_lock lock(mutex);
				maximum = std::max<size_t>(maximum, ++readers);
			}

			EXPECT_EQ(std::accumulate(storage.begin(), storage.end(), 0), 15);

			using namespace std::chrono_literals;
			std::this_thread::sleep_for(100ms);

			{
				std::unique_lock lock(mutex);
				--readers;
			}
		});
	};

	std::vector<std::thread> threads(100);

	for (auto& thread : threads) {
		thread = std::thread(worker);
	}

	for (auto& thread : threads) {
		thread.join();
	}

	EXPECT_GT(maximum, 50);
	EXPECT_EQ(readers, 0);
}

TEST(Storage, MultithreadedRecirsion) {
	const auto worker = [&] {
		using namespace std::chrono_literals;

		Storage<TestStorage>::Read([](const TestStorage::DataType& storage) {
			std::this_thread::sleep_for(2ms);

			Storage<TestStorage>::Write([](TestStorage::DataType& storage) {
				Storage<TestStorage>::Read([](const TestStorage::DataType& storage) {
					Storage<TestStorage>::Read([](const TestStorage::DataType& storage) {
						Storage<TestStorage>::Write([](TestStorage::DataType& storage) {
							storage.clear();
							EXPECT_TRUE(storage.empty());

							for (int number : { 1, 2, 3 }) {
								storage.emplace_back(number);
							}

							EXPECT_EQ(storage.size(), 3);
							Storage<TestStorage>::Read([](const TestStorage::DataType& storage) {
								std::this_thread::yield();

								Storage<TestStorage>::Write([](TestStorage::DataType& storage) {
									Storage<TestStorage>::Write([](TestStorage::DataType& storage) {
										Storage<TestStorage>::Read([](const TestStorage::DataType& storage) {
											EXPECT_EQ(storage.size(), 3);

											for (int number : { 1, 2, 3 }) {
												EXPECT_EQ(storage[number - 1], number);
											}

											std::this_thread::sleep_for(5ms);
										});
									});
								});
							});
						});

						std::this_thread::sleep_for(1ms);
					});
				});
			});
		});
	};

	std::vector<std::thread> threads(100);

	for (auto& thread : threads) {
		thread = std::thread(worker);
	}

	for (auto& thread : threads) {
		thread.join();
	}
}

TEST(Storage, ConcurrentReadWrite) {
	using namespace std::chrono_literals;

	Storage<TestStorage>::Write([](TestStorage::DataType& storage) {
		storage.clear();
		EXPECT_TRUE(storage.empty());
	});

	std::thread reader;

	const auto readerWorker = []() {
		Storage<TestStorage>::Read([](const TestStorage::DataType& storage) {
			const size_t size = storage.size();

			for (int number : { 1, 2, 3, 4, 5 }) {
				std::this_thread::sleep_for(5ms);
				EXPECT_EQ(storage.size(), size);
			}
		});
	};

	const auto writerWorker = [&] {
		reader = std::thread(readerWorker);

		Storage<TestStorage>::Write([](TestStorage::DataType& storage) {
			for (int number : { 1, 2, 3, 4, 5 }) {
				storage.emplace_back(number);

				std::this_thread::sleep_for(10ms);
			}
		});
	};

	std::thread writer = std::thread(writerWorker);

	writer.join();
	reader.join();
}

int main(int argc, char* argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
