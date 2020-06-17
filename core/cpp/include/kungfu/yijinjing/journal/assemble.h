//
// Created by Keren Dong on 2020/5/22.
//

#ifndef YIJINJING_ASSEMBLE_H
#define YIJINJING_ASSEMBLE_H

#include <kungfu/yijinjing/journal/journal.h>

namespace kungfu::yijinjing::journal {
class sink {
public:
  sink();
  virtual ~sink() = default;
  virtual void put(const data::location_ptr &location, uint32_t dest_id, const frame_ptr &frame) = 0;
  virtual void close(){};
  [[nodiscard]] publisher_ptr get_publisher();

private:
  publisher_ptr publisher_;
};
DECLARE_PTR(sink)

class null_sink : public sink {
public:
  null_sink() = default;
  void put(const data::location_ptr &location, uint32_t dest_id, const frame_ptr &frame) override{};
};

class copy_sink : public sink {
public:
  explicit copy_sink(data::locator_ptr locator);
  void put(const data::location_ptr &location, uint32_t dest_id, const frame_ptr &frame) override;

private:
  data::locator_ptr locator_;
  std::unordered_map<uint32_t, std::unordered_map<uint32_t, writer_ptr>> writers_ = {};
};

class assemble {
public:
  explicit assemble(const std::vector<data::locator_ptr> &locators, const std::string &mode = "*",
                    const std::string &category = "*", const std::string &group = "*", const std::string &name = "*");

  virtual ~assemble() = default;

  assemble operator+(assemble &other);

  void operator>>(const sink_ptr &sink);

  bool data_available();

  void next();

  frame_ptr current_frame();

protected:
  std::vector<reader_ptr> readers_ = {};
  reader_ptr current_reader_ = {};

private:
  const std::string &mode_;
  const std::string &category_;
  const std::string &group_;
  const std::string &name_;
  publisher_ptr publisher_;
  std::vector<data::locator_ptr> locators_ = {};

  void sort();
};
DECLARE_PTR(assemble)
} // namespace kungfu::yijinjing::journal
#endif // YIJINJING_ASSEMBLE_H
