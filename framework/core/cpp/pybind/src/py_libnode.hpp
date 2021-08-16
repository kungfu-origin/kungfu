//
// Created by Keren Dong on 2021/8/13.
//

#ifndef KUNGFU_PY_LIBNODE_HPP
#define KUNGFU_PY_LIBNODE_HPP

#include <node.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace kungfu::libnode {
namespace py = pybind11;

void run(const py::args &args, const py::kwargs &kwargs) {
  int argc = static_cast<int>(args.size());
  char **argv = new char *[argc];
  for (int i = 0; i < argc; i++) {
    std::string arg = args[i].cast<py::str>();
    argv[i] = const_cast<char *>(arg.c_str());
  }
  node::Start(argc, argv);
  delete[] argv;
}

void bind(pybind11::module &&m) { m.def("run", &run); }
} // namespace kungfu::libnode
#endif // KUNGFU_PY_LIBNODE_HPP
