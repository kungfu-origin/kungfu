#include "signal.h"
#include "stdafx.h"
#include <tlhelp32.h>

namespace WindowsKillLibrary {

using std::invalid_argument;
using std::string;

Signal::Signal(DWORD pid, DWORD_PTR type) {
  Signal::validateType(type);
  this->type = type;
  Signal::checkPidExist(pid);
  this->pid = pid;
}

DWORD_PTR Signal::getType() { return this->type; }

DWORD Signal::getPid() { return this->pid; }

void Signal::validateType(DWORD_PTR type) {
  if (!(type == CTRL_C_EVENT || type == CTRL_BREAK_EVENT)) {
    throw invalid_argument(string("EINVAL"));
  }
}

// TODO: Maybe there are better solution to check that pid exist.
// SOURCE:  http://cppip.blogspot.co.uk/2013/01/check-if-process-is-running.html
void Signal::checkPidExist(DWORD pid) {
  bool exist = false;
  HANDLE pss = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
  PROCESSENTRY32 pe = {0};
  pe.dwSize = sizeof(pe);
  if (Process32First(pss, &pe)) {
    do {
      if (pe.th32ProcessID == pid) {
        exist = true;
      }
    } while (Process32Next(pss, &pe));
  }
  CloseHandle(pss);

  if (!exist) {
    throw invalid_argument(string("ESRCH"));
  }
}
} // namespace WindowsKillLibrary