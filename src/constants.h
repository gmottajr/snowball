
#include <stdint.h>
#include <stdio.h>

#ifndef __SNOWBALL_CONSTANTS_H_
#define __SNOWBALL_CONSTANTS_H_

#ifdef __cplusplus
extern "C" {
#endif

// breaking.feature.fix
#if !defined(_SNOWBALL_VERSION) || !defined(_SNOWBALL_VERSION_NUMBER)
#error "_SNOWBALL_VERSION and _SNOWBALL_VERSION_NUMBER must be defined!"
#endif
#define _SNOWBALL_BUILD_DATE __DATE__
#define _SNOWBALL_BUILD_TIME __TIME__

// Exports
#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

#ifndef _SNOWBALL_PACkAGE_REGISTRY
// http to avoid needing to use an SSL library
#define _SNOWBALL_PACKAGE_REGISTRY "http://snowball-lang.github.io/packages/pkgs/"
#endif

// OS specific
#pragma region
#ifdef _WIN32
#define OS_NAME        "Windows 32-bit"
#define PATH_SEPARATOR "\\"
#define _SNOWBALL_OS   "windows"
#elif _WIN64
#define OS_NAME        "Windows 64-bit"
#define PATH_SEPARATOR "\\"
#define _SNOWBALL_OS   "windows"
#elif __APPLE__ || __MACH__
#define OS_NAME        "Mac OS_NAMEX"
#define PATH_SEPARATOR "/"
#define _SNOWBALL_OS   "macos"
#elif __linux__
#define OS_NAME        "Linux"
#define PATH_SEPARATOR "/"
#define _SNOWBALL_OS   "linux"
#elif __unix || __unix__
#define OS_NAME        "Unix"
#define PATH_SEPARATOR "/"
#define _SNOWBALL_OS   "unix"
#else
#define OS_NAME        "Unknown OS"
#define PATH_SEPARATOR "/"
#define _SNOWBALL_OS   "unknown"
#endif
#pragma endregion

// Values

#define _SNOWBALL_CONST_PTR "$const-pointer"
#define _SNOWBALL_MUT_PTR   "$mut-pointer"
#define _SNOWBALL_INT_IMPL  "$integer-impl"
#define _SNOWBALL_FUNC_IMPL "$function-impl"

// Make sure this is always correct!
#define _SNOWBALL_CONST_PTR_DECL "std::internal::preloads::$const-pointer"
#define _SNOWBALL_MUT_PTR_DECL   "std::internal::preloads::$mut-pointer"

#define _SNOWBALL_STR_FACTOR 16
#define _SNOWBALL_MAX_LENGTH 2147483647
#define _SNOWBALL_OUT_DEFAULT(x, t, s) \
  (".sn" PATH_SEPARATOR "bin" PATH_SEPARATOR + (os::Driver::getOutputFilename(x, t, s)))

#ifndef _SNOWBALL_ENABLE_INT64
#define _SNOWBALL_ENABLE_INT64 1
#endif

#if _SNOWBALL_ENABLE_INT64
typedef uint64_t snowball_int_t;
#define _SNOWBALL_INT_MAX 9223372036854775807
#define _SNOWBALL_INT_MIN (-_SNOWBALL_INT_MAX - 1LL)
#else
typedef uint32_t snowball_int_t;
#define _SNOWBALL_INT_MAX 2147483647
#define _SNOWBALL_INT_MIN -2147483648
#endif

#define SN_INT_MAX_POWER 8388608
#define SN_MAX_MACRO_DEPTH 2048

#define UNREACHABLE \
  do { std::abort(); } while (0);

// Debug
#undef NDEBUG

#ifndef _SN_DEBUG
#define _SN_DEBUG true
#endif

#define LINE_SEPARATOR "------------------"
#if _SN_DEBUG
#define _SNOWBALL_BUILD_TYPE "Debug"

#define _SNOWBALL_LEXER_DEBUG    0
#define _SNOWBALL_PARSER_DEBUG   0
#define _SNOWBALL_CODEGEN_DEBUG  0
#define _SNOWBALL_BYTECODE_DEBUG 1
#define _SNOWBALL_SYMTABLE_DEBUG 0
#define _SNOWBALL_TIMERS_DEBUG   0
#define _SNOWBALL_FREE_DEBUG     0 // todo

#define PRINT_LINE(...) \
  printf(__VA_ARGS__); \
  printf("\n"); \
  fflush(stdout);

#if _SNOWBALL_LEXER_DEBUG
#define DEBUG_LEXER(...) PRINT_LINE(__VA_ARGS__)
#else
#define DEBUG_LEXER(...)
#endif

#if _SNOWBALL_PARSER_DEBUG
#define DEBUG_PARSER(...) PRINT_LINE(__VA_ARGS__)
#else
#define DEBUG_PARSER(...)
#endif

#if _SNOWBALL_TIMERS_DEBUG
#define DEBUG_TIMER(...) PRINT_LINE(__VA_ARGS__)
#else
#define DEBUG_TIMER(...)
#endif

#if _SNOWBALL_SYMTABLE_DEBUG
#define DEBUG_SYMTABLE(depth, ...) \
  printf("%*s", depth * 4, " "); \
  PRINT_LINE(__VA_ARGS__)
#else
#define DEBUG_SYMTABLE(...)
#endif

#if _SNOWBALL_CODEGEN_DEBUG
#define DEBUG_CODEGEN(...) PRINT_LINE(__VA_ARGS__)
#else
#define DEBUG_CODEGEN(...)
#endif

#if _SNOWBALL_FREE_DEBUG
#define DEBUG_FREE(...) PRINT_LINE(__VA_ARGS__)
#else
#define DEBUG_FREE(...)
#endif

#define DEBUG_ALWAYS(...) PRINT_LINE(__VA_ARGS__)

#define DUMP(varname)   fprintf(stderr, "%s (%s) = %i\n", #varname, typeid(varname).name(), varname);
#define DUMP_S(varname) fprintf(stderr, "%s (%s) = %s\n", #varname, typeid(varname).name(), varname);

#else
#define _SNOWBALL_BUILD_TYPE "Default"

#define DEBUG_PARSER(...)
#define DEBUG_SYMTABLE(...)
#define DEBUG_CODEGEN(...)
#define DEBUG_FREE(...)

#define DEBUG_ALWAYS(...)

#define _SNOWBALL_LEXER_DEBUG    0
#define _SNOWBALL_PARSER_DEBUG   0
#define _SNOWBALL_CODEGEN_DEBUG  0
#define _SNOWBALL_BYTECODE_DEBUG 0
#define _SNOWBALL_SYMTABLE_DEBUG 0
#define _SNOWBALL_FREE_DEBUG     0
#endif

// LD
#pragma region

#ifndef _SNOWBALL_LIBRARY_OBJ
#error "_SNOWBALL_LIBRARY_OBJ must be defined! (e.g. \"snowball-objects\")"
#endif

#ifndef _SNOWBALL_LIBRARY_DIR
#error "_SNOWBALL_LIBRARY_DIR must be defined! (e.g. \"snowball-libs\")"
#endif

#ifndef _SNOWBALL_PACKAGES_DIR
#define _SNOWBALL_PACKAGES_DIR ".sn" PATH_SEPARATOR "deps"
#endif

#ifndef _SNOWBALL_LLVM_PACKAGE_VERSION
#error "_SNOWBALL_LLVM_PACKAGE_VERSION must be defined! (e.g. \"16.0.6\")"
#endif

// path of ld compiler used for linking
#ifndef LD_PATH
#error "LD_PATH must be defined! (e.g. \"/usr/bin/ld\")"
#endif

#ifndef STATICLIB_DIR
#error "STATICLIB_DIR path must be defined! (e.g. \"/usr/lib/\")"
#endif

#pragma endregion

// Optimizations
#ifndef _SNOWBALL_CAN_OPTIMIZE
#define _SNOWBALL_CAN_OPTIMIZE 1
#endif

// Function names
#define _SNOWBALL_FUNCTION_ENTRY "main"

// Keywords
#define _SNOWBALL_KEYWORD__IF        "if"
#define _SNOWBALL_KEYWORD__AS        "as"
#define _SNOWBALL_KEYWORD__FOR       "for"
#define _SNOWBALL_KEYWORD__ENUM      "enum"
#define _SNOWBALL_KEYWORD__TRUE      "true"
#define _SNOWBALL_KEYWORD__ELSE      "else"
#define _SNOWBALL_KEYWORD__CLASS     "class"
#define _SNOWBALL_KEYWORD__INTER     "interface"
#define _SNOWBALL_KEYWORD__EXTENDS   "extends"
#define _SNOWBALL_KEYWORD__IMPLS     "implements"
#define _SNOWBALL_KEYWORD__PRIVATE   "private"
#define _SNOWBALL_KEYWORD__PUBLIC    "public"
#define _SNOWBALL_KEYWORD__VIRTUAL   "virtual"
#define _SNOWBALL_KEYWORD__FALSE     "false"
#define _SNOWBALL_KEYWORD__OVERRIDE  "override"
#define _SNOWBALL_KEYWORD__WHILE     "while"
#define _SNOWBALL_KEYWORD__BREAK     "break"
#define _SNOWBALL_KEYWORD__EXTERN    "external"
#define _SNOWBALL_KEYWORD__SUPER     "super"
#define _SNOWBALL_KEYWORD__STATIC    "static"
#define _SNOWBALL_KEYWORD__IMPORT    "import"
#define _SNOWBALL_KEYWORD__UNSAFE    "unsafe"
#define _SNOWBALL_KEYWORD__CONSTEXPR "constexpr"
#define _SNOWBALL_KEYWORD__NAMESPACE "namespace"
#define _SNOWBALL_KEYWORD__STRUCT    "struct"
#define _SNOWBALL_KEYWORD__TYPEDEF   "type"
#define _SNOWBALL_KEYWORD__MUTABLE   "mut"
#define _SNOWBALL_KEYWORD__DO        "do"
#define _SNOWBALL_KEYWORD__RETURN    "return"
#define _SNOWBALL_KEYWORD__DECLTYPE  "decltype"
#define _SNOWBALL_KEYWORD__SWITCH    "switch"
#define _SNOWBALL_KEYWORD__CASE      "case"
#define _SNOWBALL_KEYWORD__DEFAULT   "default"
#define _SNOWBALL_KEYWORD__CONTINUE  "continue"
#define _SNOWBALL_KEYWORD__FUNCTION  "func"
#define _SNOWBALL_KEYWORD__MACRO     "macro"
#define _SNOWBALL_KEYWORD__OPERATOR  "operator"
#define _SNOWBALL_KEYWORD__VARIABLE  "let"
#define _SNOWBALL_KEYWORD__NEW       "new"
#define _SNOWBALL_KEYWORD__THROW     "throw"
#define _SNOWBALL_KEYWORD__CONSTANT  "const"
#define _SNOWBALL_KEYWORD__TRY       "try"
#define _SNOWBALL_KEYWORD__CATCH     "catch"

#define _SNOWBALL_LAMBDA_FUNCTIONS \
  { 'l', 'a', 'm', 'b', 'd', 'a', ' ', 'f', 'u', 'n', 'c', 't', 'i', 'o', 'n', 0 }
#define _SNOWBALL_LAMBDA_SIZE 17

// LLVM IR constants
#define _SN_CLASS_PREFIX  "class."
#define _SN_STRUCT_PREFIX "struct."
#define _SN_VTABLE_PREFIX "vtable."
#define _SN_ENUM_PREFIX   "enum."

#define _SN_MANGLE_PREFIX "_ZN$SN"

// Compiler lines
#if 0
#define _SNOWBALL_COMPILER_ENTRY \
  "Snowball " _SNOWBALL_VERSION " (" _SNOWBALL_BUILD_TYPE ": " _SNOWBALL_BUILD_DATE ", " _SNOWBALL_BUILD_TIME ")"
#define _SNOWBALL_COMPILER_ENTRY_BK \
  "\
Snowball " _SNOWBALL_VERSION " (" _SNOWBALL_BUILD_TYPE ": " _SNOWBALL_BUILD_DATE ", " _SNOWBALL_BUILD_TIME ")\n\
Type \"help\", \"copyright\", \"credits\" or \"license\" for more information.\
"
#endif

// Licenses
#define _SNOWBALL_LICENSE_NAME "MIT"
#define _SNOWBALL_LICENSE_TEXT \
  "\
The MIT License (MIT)\n\
\n\
Copyright (c) 2022 mauro-balades <mauro.balades@tutanota.com>\n\
\n\
Permission is hereby granted, free of charge, to any person obtaining a copy\n\
of this software and associated documentation files (the \"Software\"), to deal\n\
in the Software without restriction, including without limitation the rights\n\
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n\
copies of the Software, and to permit persons to whom the Software is\n\
furnished to do so, subject to the following conditions:\n\
\n\
The above copyright notice and this permission notice shall be included in\n\
all copies or substantial portions of the Software.\n\
\n\
THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n\
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n\
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n\
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n\
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n\
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN\n\
THE SOFTWARE."

#ifdef __cplusplus
}
#endif

#endif // __SNOWBALL_CONSTANTS_H_
