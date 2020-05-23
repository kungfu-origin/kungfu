//
// Created by Keren Dong on 2020/5/22.
//

#ifndef YIJINJING_ASSEMBLE_H
#define YIJINJING_ASSEMBLE_H

#include <kungfu/yijinjing/journal/journal.h>

namespace kungfu::yijinjing::journal {
class assemble {
public:
  explicit assemble(const std::vector<data::locator_ptr> &locators, const std::string &mode = "*",
                    const std::string &category = "*", const std::string &group = "*", const std::string &name = "*");

  virtual ~assemble() = default;

  assemble operator+(assemble &other);

  void operator>>(const data::locator_ptr &locator);

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

  bool data_available();
  void next();
  void sort();
};
DECLARE_PTR(assemble)
} // namespace kungfu::yijinjing::journal
#endif // YIJINJING_ASSEMBLE_H
