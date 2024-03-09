// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "Storage.h"

template <typename EventType>
class Event final {
public:
	~Event() = delete;
	
	using ReceiverAddressType = void*;
	using ReceiverCallbackType = std::function<void(EventType&)>;

	using DataType = std::unordered_map<ReceiverAddressType, ReceiverCallbackType>;

	template <typename ValueType>
	using EventForwardingEnabled = std::enable_if_t<std::is_same_v<std::decay_t<ValueType>, EventType>, bool>;

	template <typename ValueType, EventForwardingEnabled<ValueType>  = true>
	static void Send(ValueType&& event) {
		Storage<Event>::Read([&](const DataType& storage) {
			for (const auto& [receiver, callback] : storage) {
				callback(event);
			}
		});
	}

	template <typename... ArgumentTypes>
	static void Send(ArgumentTypes&&... arguments) {
		Send<EventType>({std::forward<ArgumentTypes>(arguments)...});
	}

	static void Receive(ReceiverAddressType receiver, ReceiverCallbackType callback) {
		Storage<Event>::Write([&](DataType& storage) {
			storage[receiver] = std::move(callback);
		});
	}
};
