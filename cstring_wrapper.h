// cstring wrapper for Ubuntu 25.04+ GCC compatibility
// This prevents the broken libstdc++ <cstring> from being included
#ifndef CSTRING_WRAPPER_H
#define CSTRING_WRAPPER_H

// Use C string.h instead of broken C++ cstring
#include <string.h>

// Declare the functions that Source Engine and DXVK actually use
extern "C" {
	extern char *strncpy (char *__restrict __dest, const char *__restrict __src, size_t __n) noexcept;
	extern int strcmp (const char *__s1, const char *__s2) noexcept;
	extern size_t strlen (const char *__s) noexcept;
	extern char *strdup (const char *__s);
	extern int memcmp (const void *__s1, const void *__s2, size_t __n) noexcept;
	extern void *memcpy (void *__restrict __dest, const void *__restrict __src, size_t __n) noexcept;
}

// Provide std:: versions for DXVK
namespace std {
	using ::memcmp;
	using ::memcpy;
}

// Prevent real cstring from being included
#define _GLIBCXX_CSTRING 1
#define _CPP_CSTRING 1

#endif

