#include "windows-kill-library.h"
#include <nan.h>
#include <stdexcept>

using WindowsKillLibrary::sendSignal;
using WindowsKillLibrary::SIGNAL_TYPE_CTRL_BREAK;
using WindowsKillLibrary::SIGNAL_TYPE_CTRL_C;
using WindowsKillLibrary::warmUp;

#define NODEWINDOWSKILL_VERSION "0.3.0"

void send(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  if (info.Length() < 2) {
    Nan::ThrowTypeError("Wrong number of arguments");
    return;
  }

  if (!info[0]->IsNumber() || !info[1]->IsNumber()) {
    Nan::ThrowTypeError("Wrong arguments");
    return;
  }

  DWORD signal_pid = (DWORD)info[0]->NumberValue();
  DWORD signal_type = (DWORD)info[1]->NumberValue();

  try {
    if (signal_type == 1) {
      sendSignal(signal_pid, SIGNAL_TYPE_CTRL_BREAK);
    } else {
      sendSignal(signal_pid, SIGNAL_TYPE_CTRL_C);
    }
    info.GetReturnValue().Set(0);
  } catch (const std::invalid_argument &exception) {
    if (strcmp(exception.what(), "ESRCH") == 0) {
      info.GetReturnValue().Set(Nan::New<v8::String>("ESRCH").ToLocalChecked());
      return;
    }
    info.GetReturnValue().Set(Nan::New<v8::String>("ESYS").ToLocalChecked());
  } catch (const std::runtime_error &exception) {
    if (strcmp(exception.what(), "EPERM") == 0) {
      info.GetReturnValue().Set(Nan::New<v8::String>("EPERM").ToLocalChecked());
      return;
    }
    info.GetReturnValue().Set(Nan::New<v8::String>("ESYS").ToLocalChecked());
  } catch (std::exception) {
    info.GetReturnValue().Set(Nan::New<v8::String>("ESYS").ToLocalChecked());
  }
}

void warmUp(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  try {
    warmUp();
  } catch (const std::invalid_argument &exception) {
    if (strcmp(exception.what(), "Invalid which argument.") == 0) {
      info.GetReturnValue().Set(Nan::New<v8::String>("Invalid which argument.").ToLocalChecked());
      return;
    }
    info.GetReturnValue().Set(Nan::New<v8::String>("ESYS").ToLocalChecked());
  } catch (std::exception) {
    info.GetReturnValue().Set(Nan::New<v8::String>("ESYS").ToLocalChecked());
  }
}

void Init(v8::Local<v8::Object> exports) {
  exports->Set(Nan::New("send").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(send)->GetFunction());
  exports->Set(Nan::New("warmUp").ToLocalChecked(), Nan::New<v8::FunctionTemplate>(warmUp)->GetFunction());
}

NODE_MODULE(windowskill, Init)
