#pragma once

#if defined(_WIN32) || defined(_WIN64)
#if defined(AETHERNET_EXPORTS)

#define AETHERNET_API __declspec(dllexport)
#else
// Using the DLL
#define AETHERNET_API __declspec(dllimport)
#endif
#else
// Non-Windows (Linux/macOS)
#define AETHERNET_API
#endif