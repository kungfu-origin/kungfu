//
// Created by Keren Dong on 2020/3/25.
//

#include <kungfu/common.h>
#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/log.h>
#include <kungfu/yijinjing/time.h>
#include <tabulate/table.hpp>

#define TIME_FORMAT "%T.%N"

using namespace tabulate;
using namespace kungfu::longfist;
using namespace kungfu::longfist::enums;
using namespace kungfu::longfist::types;
using namespace kungfu::yijinjing::data;
using namespace kungfu::yijinjing::journal;

namespace kungfu::yijinjing {
struct console_table {
  Table table = {};
  int32_t width;
  int32_t height;
  int rows_count;

  console_table(int32_t console_width, int32_t console_height)
      : width(console_width), height(console_height), rows_count(0) {
    table.add_row({"gen_time", "trigger_time", "source", "dest", "msg_type", "data"});
    rows_count = 1;
  }

  ~console_table() {
    if (rows_count > 1) {
      print();
    }
  }

  void reset() {
    table = {};
    rows_count = 0;
  }

  void print() {
    table.format().padding(0).border(" ").hide_border().show_border_right();
    table.column(0).format().width(20).font_align(FontAlign::left);
    table.column(1).format().width(20).font_align(FontAlign::left);
    table.column(2).format().width(30).font_align(FontAlign::left);
    table.column(3).format().width(30).font_align(FontAlign::left);
    table.column(4).format().width(24).font_align(FontAlign::left);
    if (width > 130) {
      table.column(5).format().width(width - 130);
    }
    std::cout << table << std::endl;
  }

  void add_row(const std::vector<std::variant<std::string, const char *, Table>> &cells) {
    table.add_row(cells);
    rows_count++;

    if (rows_count >= height) {
      print();
      reset();
    }
  }
};

io_device_console::io_device_console(data::location_ptr home, int32_t console_width, int32_t console_height)
    : io_device(std::move(home), false, true), console_width_(console_width), console_height_(console_height) {}

void io_device_console::trace(int64_t begin_time, int64_t end_time, bool in, bool out) {
  std::unordered_map<uint32_t, location_ptr> locations = {};
  for (auto location : home_->locator->list_locations(".*", ".*", ".*", ".*")) {
    locations.emplace(location->uid, location);
  }

  auto reader = open_reader_to_subscribe();

  if (in) {
    auto uid_str = fmt::format("{:08x}", home_->uid);
    auto master_cmd_location = location::make_shared(mode::LIVE, category::SYSTEM, "master", uid_str, get_locator());
    auto master_home_location = location::make_shared(mode::LIVE, category::SYSTEM, "master", "master", get_locator());

    reader->join(master_cmd_location, home_->uid, begin_time);
    reader->join(master_home_location, location::PUBLIC, begin_time);
  }
  if (out) {
    for (auto dest_id : get_locator()->list_location_dest(home_)) {
      reader->join(home_, dest_id, begin_time);
    }
  }

  console_table table(console_width_, console_height_);

  while (reader->data_available() and reader->current_frame()->gen_time() <= end_time) {
    auto frame = reader->current_frame();
    auto dest_name = frame->dest() == location::PUBLIC ? "public" : locations.at(frame->dest())->uname;
    bool type_found = false;
    boost::hana::for_each(AllTypes, [&](auto type) {
      using DataType = typename decltype(+boost::hana::second(type))::type;
      if (frame->msg_type() == DataType::tag) {
        table.add_row({
            time::strftime(frame->gen_time(), TIME_FORMAT),     //
            time::strftime(frame->trigger_time(), TIME_FORMAT), //
            locations.at(frame->source())->uname,               //
            dest_name,                                          //
            DataType::type_name.c_str(),                        //
            frame->data<DataType>().to_string()                 //
        });
        type_found = true;
      }
    });
    if (not type_found) {
      auto location_uname = reader->current_page()->get_location()->uname;
      auto dest_id = reader->current_page()->get_dest_id();
      SPDLOG_ERROR("{}/{:08x} msg_type {} not found", location_uname, dest_id, frame->msg_type());
      break;
    }
    if (frame->dest() == home_->uid and frame->msg_type() == RequestReadFrom::tag) {
      auto request = frame->data<RequestReadFrom>();
      auto source_location = locations.at(request.source_id);
      reader->join(source_location, home_->uid, request.from_time);
    }
    if (frame->dest() == home_->uid and frame->msg_type() == RequestReadFromPublic::tag) {
      auto request = frame->data<RequestReadFromPublic>();
      auto source_location = locations.at(request.source_id);
      reader->join(source_location, location::PUBLIC, request.from_time);
    }
    if (frame->dest() == home_->uid and frame->msg_type() == Deregister::tag) {
      reader->disjoin(location::make_shared(frame->data<Deregister>(), get_locator())->uid);
    }
    reader->next();
  }
}
} // namespace kungfu::yijinjing