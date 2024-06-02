// Copyright ❤️ 2023-2024, Sergei Belov
#pragma once

// Macro for temporarily suppressing all compiler warnings.
#define WARNINGS_IGNORE \
_Pragma("clang diagnostic push") \
_Pragma("clang diagnostic ignored \"-Weverything\"")

// Macro for restoring the previous compiler warning settings.
#define WARNINGS_NOTICE \
_Pragma("clang diagnostic pop")
