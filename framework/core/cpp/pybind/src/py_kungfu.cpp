/*****************************************************************************
 * Copyright [www.kungfu-trader.com]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

#include "py_libnode.hpp"
#include "py_longfist.hpp"
#include "py_wingchun.hpp"
#include "py_yijinjing.hpp"

namespace py = pybind11;
namespace hana = boost::hana;

PYBIND11_MODULE(pykungfu, m) {
  kungfu::libnode::bind(m.def_submodule("libnode"));
  kungfu::longfist::bind(m.def_submodule("longfist"));
  kungfu::yijinjing::bind(m.def_submodule("yijinjing"));
  kungfu::wingchun::bind(m.def_submodule("wingchun"));
}
