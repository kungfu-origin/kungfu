//
// Created by Keren Dong on 2021/8/13.
//

#ifndef KUNGFU_PY_LIBNODE_HPP
#define KUNGFU_PY_LIBNODE_HPP

#include <pybind11/pybind11.h>

namespace kungfu::libnode {

void bind(pybind11::module &&m);

} // namespace kungfu::libnode
#endif // KUNGFU_PY_LIBNODE_HPP
