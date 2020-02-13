//
// Created by Keren Dong on 2020/1/1.
//

#ifndef KUNGFU_NODE_IO_H
#define KUNGFU_NODE_IO_H

#include <napi.h>

#include <kungfu/yijinjing/io.h>
#include "journal.h"

namespace kungfu::node
{
    class Locator : public Napi::ObjectWrap<Locator>, public yijinjing::data::locator
    {
    public:
        explicit Locator(const Napi::CallbackInfo &info);

        ~Locator();

        Napi::Value ToString(const Napi::CallbackInfo &info);

        [[nodiscard]] bool has_env(const std::string &name) const override;

        [[nodiscard]] std::string get_env(const std::string &name) const override;

        [[nodiscard]] std::string layout_dir(yijinjing::data::location_ptr location, yijinjing::data::layout l) const override;

        [[nodiscard]] std::string
        layout_file(yijinjing::data::location_ptr location, yijinjing::data::layout l, const std::string &name) const override;

        [[nodiscard]] std::string default_to_system_db(yijinjing::data::location_ptr location, const std::string &name) const override;

        [[nodiscard]] std::vector<int> list_page_id(yijinjing::data::location_ptr location, uint32_t dest_id) const override;

        [[nodiscard]] std::vector<yijinjing::data::location_ptr>
        list_locations(const std::string &category, const std::string &group, const std::string &name, const std::string &mode) const override;

        [[nodiscard]] std::vector<uint32_t> list_location_dest(yijinjing::data::location_ptr location) const override;

        static void Init(Napi::Env env, Napi::Object exports);

    private:
        Napi::ObjectReference locator_ref_;
        static Napi::FunctionReference constructor;
    };

    class IODevice : public Napi::ObjectWrap<IODevice>, public yijinjing::io_device
    {
    public:
        explicit IODevice(const Napi::CallbackInfo &info);

        Napi::Value ToString(const Napi::CallbackInfo &info);

        Napi::Value FindSessions(const Napi::CallbackInfo &info);

        Napi::Value OpenReader(const Napi::CallbackInfo &info);

        static yijinjing::data::locator_ptr GetLocator(const Napi::CallbackInfo &info, int index = 0);

        static yijinjing::data::location_ptr GetLocation(const Napi::CallbackInfo &info);

        static void Init(Napi::Env env, Napi::Object exports);

    private:
        static Napi::FunctionReference constructor;
    };

    class Session : public Napi::ObjectWrap<Session>
    {
    public:
        explicit Session(const Napi::CallbackInfo &info);

        Napi::Value ToString(const Napi::CallbackInfo &info);

        static void Init(Napi::Env env, Napi::Object exports);

    private:
        static Napi::FunctionReference constructor;
    };
}


#endif //KUNGFU_NODE_IO_H
