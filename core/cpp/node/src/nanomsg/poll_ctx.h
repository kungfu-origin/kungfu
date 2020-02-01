#pragma once

#include <nan.h>

class PollCtx {
  private:
    const Nan::Callback callback;
    uv_os_sock_t sockfd; // for libnanomsg
    void begin_poll (const int s, const bool is_sender);
  public:
    uv_poll_t poll_handle; // for libuv
    PollCtx (const int s, const bool is_sender,
        const v8::Local<v8::Function> cb);
    void invoke_callback (const int events) const;
    static v8::Local<v8::Value> WrapPointer (void* ptr, size_t length);
    static PollCtx* UnwrapPointer (v8::Local<v8::Value> buffer);
};
