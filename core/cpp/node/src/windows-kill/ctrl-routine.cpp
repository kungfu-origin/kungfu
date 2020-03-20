#include "ctrl-routine.h"
#include "stdafx.h"
#include <DbgHelp.h>

namespace WindowsKillLibrary {

using std::error_code;
using std::runtime_error;
using std::string;
using std::system_category;
using std::system_error;

/// <summary>
/// a refrence to the *this used for using in SetConsoleCtrlHandler custom handler method (customConsoleCtrlHandler)
/// </summary>
CtrlRoutine *CtrlRoutine::current_routine = NULL;

CtrlRoutine::CtrlRoutine(DWORD event_type) {
  this->event_type = event_type;
  this->address = (LPTHREAD_START_ROUTINE)NULL;
  this->found_address_event = (HANDLE)NULL;
}

DWORD CtrlRoutine::getEventType(void) { return this->event_type; }

LPTHREAD_START_ROUTINE CtrlRoutine::getAddress(void) {
  /*
          TODO: Currently caller should call findAddress before getAddress. Otherwise NULL will be returned.
          Maybe it's better to remove the findAddress call (or make it a private method) and use below commented code
          to find the address if address is NULL.
  */
  /*
          NOTE: The reason for commenting the below snippet code and forcing the caller to call the findAddress
          before call getAddress is performance and speed. Becasue current approach will get the ctrl routine address
     first and then the remote process open and start will happen. So if there is any prbolem (exception) in getting the
     ctrl routine address, the remote process open will not fire. If we change the current approach, the findAddress
     method will not fire until the remote process startRemoteThread method call. So the remote process open method will
     be called even we can't find the ctrl routine address and send the signal. So current approach is  better for
     performance and speed.
  */
  /*
  if (this->address == NULL) {
          CtrlRoutine::findAddress();
  }
  */
  return this->address;
}

void CtrlRoutine::findAddress() {
  if (this->address == NULL) {
    /*
            Why set a refrence to "this"? Well because it's impossible set a non static class method as ctrl handler of
       SetConsoleCtrlHandler. So set a refrence to "this" and then use this refrence "CtrlRoutine::current_routine" for
       future needs.
    */
    CtrlRoutine::current_routine = &*this;

    this->found_address_event = CreateEvent(NULL, true, false, NULL);

    if (this->found_address_event == NULL) {
      throw system_error(error_code(GetLastError(), system_category()), "ctrl-routine:findAddress:CreateEvent");
    }

    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlRoutine::customConsoleCtrlHandler, true)) {
      this->removeCustomConsoleCtrlHandler();
      this->closeFoundAddressEvent();
      throw system_error(error_code(GetLastError(), system_category()),
                         "ctrl-routine:findAddress:SetConsoleCtrlHandlert");
    }

    if (!GenerateConsoleCtrlEvent(this->event_type, 0)) {
      this->removeCustomConsoleCtrlHandler();
      this->closeFoundAddressEvent();
      throw system_error(error_code(GetLastError(), system_category()),
                         "ctrl-routine:findAddress:GenerateConsoleCtrlEvent");
    }

    DWORD dwWaitResult = WaitForSingleObject(this->found_address_event, INFINITE);
    if (dwWaitResult == WAIT_FAILED) {
      this->removeCustomConsoleCtrlHandler();
      this->closeFoundAddressEvent();
      throw system_error(error_code(GetLastError(), system_category()), "ctrl-routine:findAddress:WaitForSingleObject");
    }

    if (this->address == NULL) {
      this->removeCustomConsoleCtrlHandler();
      this->closeFoundAddressEvent();
      throw runtime_error(string("ctrl-routine:findAddress:checkAddressIsNotNull"));
    }

    this->removeCustomConsoleCtrlHandler();
    this->closeFoundAddressEvent();
  }
  // TODO: remove;
  // std::cout << this->event_type << " : " << this->address << "\n";
}

void CtrlRoutine::removeCustomConsoleCtrlHandler(void) {
  if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlRoutine::customConsoleCtrlHandler, false)) {
    throw system_error(error_code(GetLastError(), system_category()), "ctrl-routine:removeCustomConsoleCtrlHandler");
  }
}

void CtrlRoutine::closeFoundAddressEvent(void) {
  if (this->found_address_event != NULL) {
    if (!CloseHandle(this->found_address_event)) {
      throw system_error(error_code(GetLastError(), system_category()), "ctrl-routine:closeFoundAddressEvent");
    }
    this->found_address_event = (HANDLE)NULL;
  }
}

BOOL CtrlRoutine::customConsoleCtrlHandler(DWORD ctrl_type) {
  if (ctrl_type != CtrlRoutine::current_routine->getEventType()) {
    return FALSE;
  }
  // TODO: remove;
  // std::cout << "TYPE: " << CtrlRoutine::current_routine->getType() << "\n";

  CtrlRoutine::current_routine->findAddressByStackBackTrace();

  SetEvent(CtrlRoutine::current_routine->found_address_event);

  /* TODO: Remove. No Exception Available here.
  if (!SetEvent(CtrlRoutine::current_routine->found_address_event)) {
          throw runtime_error(string("Cannot set event for found address event handle. Code: ") +
  std::to_string(GetLastError()));
  }
  */

  return TRUE;
}

// TODO: a rewrite needed.
void CtrlRoutine::findAddressByStackBackTrace() {
  LPVOID ctrlRoutine;

  USHORT count = CaptureStackBackTrace((ULONG)2, (ULONG)1, &ctrlRoutine, NULL);
  if (count != 1) {
    return;
    /*	TODO: Remove. No Exception Available here.
            throw runtime_error(string("Cannot capture stack back trace."));
    */
  }

  HANDLE hProcess = GetCurrentProcess();
  if (!SymInitialize(hProcess, NULL, TRUE)) {
    return;
    /*	TODO: Remove. No Exception Available here.
            throw runtime_error(string("Cannot SymInitialize. Code: ") + std::to_string(GetLastError()));
    */
  }

  ULONG64 buffer[(sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR) + sizeof(ULONG64) - 1) / sizeof(ULONG64)];
  PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;
  pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
  pSymbol->MaxNameLen = MAX_SYM_NAME;

  LPVOID funcCtrlRoutine = NULL;
  DWORD64 dwDisplacement = 0;
  if (!SymFromAddr(hProcess, (DWORD64)ctrlRoutine, &dwDisplacement, pSymbol)) {
    return;
    /*	TODO: Remove. No Exception Available here.
            throw runtime_error(string("Cannot SymFromAddr. Code: ") + std::to_string(GetLastError()));
    */
  }
  funcCtrlRoutine = reinterpret_cast<LPVOID>(pSymbol->Address);

  SymCleanup(hProcess);
  this->address = (LPTHREAD_START_ROUTINE)funcCtrlRoutine;
}
} // namespace WindowsKillLibrary
