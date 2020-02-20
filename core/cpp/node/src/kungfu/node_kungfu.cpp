//
// Created by Keren Dong on 2019/12/25.
//

// Nolong need below because cmake-js provides a ${CMAKE_JS_SRC} to do the same thing:
// https://github.com/electron/electron/issues/19884

// This hack is to not link to node.exe on windows:
// https://github.com/electron/electron/issues/17326
//#ifdef _MSC_VER
//#define WIN32_LEAN_AND_MEAN
//#include <windows.h>
//#include <delayimp.h>
//
//static FARPROC WINAPI loadDLLHook(unsigned int event, DelayLoadInfo* info) {
//    if (event != dliNotePreLoadLibrary) { return NULL; }
//    if (_stricmp(info->szDll, "NODE.EXE") != 0) { return NULL; }
//
//    return (FARPROC)GetModuleHandle(NULL);
//}
//
//decltype(__pfnDliNotifyHook2) __pfnDliNotifyHook2 = loadDLLHook;
//#endif

#include "longfist.h"
#include "journal.h"
#include "io.h"
#include "watcher.h"

namespace kungfu::node
{
    Napi::Object InitAll(Napi::Env env, Napi::Object exports)
    {
        Longfist::Init(env, exports);
        Frame::Init(env, exports);
        Reader::Init(env, exports);
        IODevice::Init(env, exports);
        Session::Init(env, exports);
        Watcher::Init(env, exports);
        return exports;
    }

    NODE_API_MODULE(addon, InitAll)
}
