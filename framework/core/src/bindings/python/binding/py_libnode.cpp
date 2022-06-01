#include <node.h>

#include "py_libnode.h"

namespace kungfu::libnode {
namespace py = pybind11;

inline size_t get_total_size(const py::args &args) {
  size_t total_size = 0;
  for (int i = 0; i < args.size(); i++) {
    total_size += py::len(args[i]) + 1;
  }
  return total_size;
}

void run(const py::args &args, const py::kwargs &kwargs) {
  size_t argv_pos = 0;
  size_t argc = args.size();
  char **argv = new char *[argc];
  char *argv_buffer = new char[get_total_size(args)]; // argv must be allocated adjacently

  for (int i = 0; i < argc; i++) {
    std::string arg = args[i].cast<py::str>();
    argv[i] = argv_buffer + argv_pos;
    strncpy(argv[i], arg.c_str(), arg.length());
    argv[i][arg.length()] = '\0';
    argv_pos += arg.length() + 1;
  }

  node::Start(static_cast<int>(argc), argv);

  delete[] argv_buffer;
  delete[] argv;
}

void bind(py::module &&m) { m.def("run", &run); }
} // namespace kungfu::libnode