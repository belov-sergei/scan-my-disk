// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once

template <typename... ComponentTypes>
class ComponentGroup {
	std::tuple<ComponentTypes...> components;
};
