#include "sender.h"
#include "ctrl-routine.h"
#include "remote-process.h"
#include "signal.h"

namespace WindowsKillLibrary {

using std::invalid_argument;
using std::string;

/*
        TODO: According to the test, the ctrl routine address for both of CTRL_C_EVENT and CTRL_BREAK_EVENT are same.
        So maybe it's not necessary to have separated ctr routine for each of events.
*/
/*
        NOTE: I've just tested the addresses on my own laptop (Windows 7 64bit). Also i can't find any document/article
   about this topic. So i think there is not enough evidence to merge these ctrl routines.
*/
/// <summary>
/// Ctrl routine for CTRL_C Signal.
/// </summary>
CtrlRoutine Sender::ctrl_c_routine = CtrlRoutine(CTRL_C_EVENT);

/// <summary>
/// Ctrl routine for CTRL_BREAK Signal.
/// </summary>
CtrlRoutine Sender::ctrl_break_routine = CtrlRoutine(CTRL_BREAK_EVENT);

void Sender::send(Signal the_signal) {
  RemoteProcess the_remote_process;
  the_remote_process.setSignal(&the_signal);

  if (the_signal.getType() == CTRL_C_EVENT) {
    ctrl_c_routine.findAddress();
    the_remote_process.setCtrlRoutine(&Sender::ctrl_c_routine);
  } else {
    ctrl_break_routine.findAddress();
    the_remote_process.setCtrlRoutine(&Sender::ctrl_break_routine);
  }

  the_remote_process.open();
  the_remote_process.startRemoteThread();
}

void Sender::warmUp(const string &which) {
  string all("ALL");
  string sigInt("SIGINT");
  string sigBreak("SIGBREAK");

  if (which.compare(all) == 0) {
    ctrl_c_routine.findAddress();
    ctrl_break_routine.findAddress();
  } else if (which.compare(sigInt) == 0) {
    ctrl_c_routine.findAddress();
  } else if (which.compare(sigBreak) == 0) {
    ctrl_break_routine.findAddress();
  } else {
    throw invalid_argument(string("Invalid which argument."));
  }
}
} // namespace WindowsKillLibrary
