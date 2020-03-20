#include "windows-kill-library.h"
#include "sender.h"
#include "signal.h"
#include "stdafx.h"

namespace WindowsKillLibrary {

using std::string;

void sendSignal(DWORD signal_pid, DWORD signal_type) {
  Signal the_signal(signal_pid, signal_type);
  Sender::send(the_signal);
}

void warmUp(const string &which) { Sender::warmUp(which); }
}; // namespace WindowsKillLibrary
