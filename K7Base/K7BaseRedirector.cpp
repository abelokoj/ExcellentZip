/*
 * PROJECT:    ExcellentZip Platform Base Library (K7Base)
 * FILE:       K7BaseRedirector.cpp
 * PURPOSE:    Implementation for ExcellentZip Platform Base API Redirector
 *
 * LICENSE:    The MIT License
 *
 * MAINTAINER: MouriNaruto (Kenji.Mouri@outlook.com)
 */

#include "K7BasePrivate.h"

// Here are the linker-time redirections to replace the original APIs.
// Implementations only for x64 and ARM64, if you want to learn how to achieve
// that in x86, please refer to ExcellentZip.Shared.ModernExperienceShims project in
// the historical ExcellentZip versions source code.

#ifndef K7_REDIRECT
#define K7_REDIRECT(Source, Target) \
    extern "C" __declspec(selectany) void const* const __imp_##Source = \
        reinterpret_cast<void const*>(::Target); \
    __pragma(comment(linker, "/include:__imp_" #Source))
#endif // !K7_REDIRECT

K7_REDIRECT(FileTimeToLocalFileTime, K7BaseModernFileTimeToLocalFileTime);
K7_REDIRECT(LocalFileTimeToFileTime, K7BaseModernLocalFileTimeToFileTime);
