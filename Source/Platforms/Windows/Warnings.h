// Copyright ❤️ 2023-2024, Sergei Belov
#pragma once

// Macro for temporarily suppressing all compiler warnings.
#define WARNINGS_IGNORE \
__pragma(warning(push, 0))

// Macro for restoring the previous compiler warning settings.
#define WARNINGS_NOTICE \
__pragma(warning(pop))
