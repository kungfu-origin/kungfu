#pragma once
#include <Windows.h>
#include <string>

#ifdef _USRDLL
#ifdef WINDOWSKILLLIBRARY_EXPORTS
#define WINDOWSKILLLIBRARY_API __declspec(dllexport)
#else
#define WINDOWSKILLLIBRARY_API __declspec(dllimport)
#endif
#else
#define WINDOWSKILLLIBRARY_API
#endif

#define WINDOWSKILLLIBRARY_VERSION "3.1.3"

namespace WindowsKillLibrary {
/// <summary>
/// Signal type of CTRL + C
/// </summary>
const DWORD SIGNAL_TYPE_CTRL_C = CTRL_C_EVENT;

/// <summary>
/// Signal type of CTRL + Break
/// </summary>
const DWORD SIGNAL_TYPE_CTRL_BREAK = CTRL_BREAK_EVENT;

/// <summary>
/// Sends the signal.
/// </summary>
/// <param name="signal_pid">The signal target process id.</param>
/// <param name="signal_type">The signal type.</param>
void WINDOWSKILLLIBRARY_API sendSignal(DWORD signal_pid, DWORD signal_type);

/// <summary>
/// Calls sender warm-up method.
/// </summary>
/// <param name="which">Which ctr-routine to warm-up</param>
void WINDOWSKILLLIBRARY_API warmUp(const std::string &which = "ALL");
}; // namespace WindowsKillLibrary
