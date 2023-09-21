#pragma once

#if defined(_WIN32)
#include "stream_windows.ipp"
#elif defined(__APPLE__)
#include "stream_macos.ipp"
#elif defined(__linux__)
#include "stream_linux.ipp"
#else
#include "stream_android.ipp"
#endif



