//
// Created by Keren Dong on 2020/5/22.
//

#ifndef YIJINJING_ASSEMBLE_H
#define YIJINJING_ASSEMBLE_H

#include <kungfu/yijinjing/journal/journal.h>

namespace kungfu::yijinjing::journal {
class sink {
public:
  virtual ~sink() = default;

  [[nodiscard]] virtual data::locator_ptr get_target_locator(const frame_ptr &frame) = 0;
};
DECLARE_PTR(sink)

class fixed_sink : public sink {
public:
  explicit fixed_sink(data::locator_ptr locator) : locator_(std::move(locator)) {}

  [[nodiscard]] data::locator_ptr get_target_locator(const frame_ptr &frame) override { return locator_; }

private:
  data::locator_ptr locator_;
};

class assemble {
public:
  explicit assemble(const std::vector<data::locator_ptr> &locators, const std::string &mode = "*",
                    const std::string &category = "*", const std::string &group = "*", const std::string &name = "*");

  virtual ~assemble() = default;

  assemble operator+(assemble &other);

  void operator>>(const sink_ptr &sink);

private:
  const std::string &mode_;
  const std::string &category_;
  const std::string &group_;
  const std::string &name_;
  bool used_ = false;
  publisher_ptr publisher_;
  std::vector<data::locator_ptr> locators_ = {};
  std::vector<reader_ptr> readers_ = {};
  reader_ptr current_reader_ = {};
  std::unordered_map<uint32_t, std::unordered_map<uint32_t, writer_ptr>> writer_maps_;

  bool data_available();
  void next();
  void sort();
  writer_ptr &get_writer(const sink_ptr &sink);
};
DECLARE_PTR(assemble)
} // namespace kungfu::yijinjing::journal
#endif // YIJINJING_ASSEMBLE_H
