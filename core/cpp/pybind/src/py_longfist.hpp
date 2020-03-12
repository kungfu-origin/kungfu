//
// Created by Keren Dong on 2020/1/30.
//

#ifndef KUNGFU_PY_LONGFIST_HPP
#define KUNGFU_PY_LONGFIST_HPP

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include <sstream>

#include <kungfu/longfist/longfist.h>
#include <kungfu/longfist/serialize/sql.h>
#include <kungfu/common.h>

namespace pybind11::detail
{
    template<size_t Size>
    struct type_caster<kungfu::array<char, Size>>
    {
        using ArrayType = kungfu::array<char, Size>;
        using value_conv = make_caster<char>;

        bool load(handle src, bool convert)
        {
            if (!isinstance<str>(src))
                return false;
            std::string &&s = reinterpret_borrow<str>(src);
            if (s.length() > Size)
                return false;
            strcpy(value.value, s.c_str());
            return true;
        }

        template<typename T>
        static handle cast(T &&src, return_value_policy policy, handle parent)
        {
            return str(src.value).release();
        }

    PYBIND11_TYPE_CASTER(ArrayType, _("String[") + value_conv::name + _("[") + _<Size>() + _("]") + _("]"));
    };

    template<typename ValueType, size_t Size>
    struct type_caster<kungfu::array<ValueType, Size>>
    {
        using ArrayType = kungfu::array<ValueType, Size>;
        using value_conv = make_caster<ValueType>;

        bool load(handle src, bool convert)
        {
            if (!isinstance<sequence>(src))
                return false;
            auto l = reinterpret_borrow<sequence>(src);
            if (l.size() > Size)
                return false;
            size_t ctr = 0;
            for (auto it : l)
            {
                value_conv conv;
                if (!conv.load(it, convert))
                    return false;
                value.value[ctr++] = cast_op<ValueType &&>(std::move(conv));
            }
            return true;
        }

        template<typename T>
        static handle cast(T &&src, return_value_policy policy, handle parent)
        {
            list l(src.size());
            for (size_t index = 0; index < Size; index++)
            {
                auto &&value = src.value[index];
                auto value_ = reinterpret_steal<object>(value_conv::cast(forward_like<T>(value), policy, parent));
                PyList_SET_ITEM(l.ptr(), (ssize_t) index, value_.release().ptr());
            }
            return l.release();
        }

    PYBIND11_TYPE_CASTER(ArrayType, _("List[") + value_conv::name + _("[") + _<Size>() + _("]") + _("]"));
    };
}

namespace kungfu::longfist
{
    namespace py = pybind11;
    namespace hana = boost::hana;

    void bind(pybind11::module &&m)
    {
        using namespace kungfu::longfist::enums;
        auto m_enums = m.def_submodule("enums");

        py::enum_<InstrumentType>(m_enums, "InstrumentType", py::arithmetic())
                .value("Unknown", InstrumentType::Unknown)
                .value("Stock", InstrumentType::Stock)
                .value("Future", InstrumentType::Future)
                .value("Bond", InstrumentType::Bond)
                .value("StockOption", InstrumentType::StockOption)
                .value("Fund", InstrumentType::Fund)
                .value("TechStock", InstrumentType::TechStock)
                .value("Index", InstrumentType::Index)
                .value("Repo", InstrumentType::Repo)
                .export_values()
                .def("__eq__",
                     [](const InstrumentType &a, int b)
                     {
                         return static_cast<int>(a) == b;
                     }
                );

        py::enum_<CommissionRateMode>(m_enums, "CommissionRateMode", py::arithmetic())
                .value("ByAmount", CommissionRateMode::ByAmount)
                .value("ByVolume", CommissionRateMode::ByVolume)
                .export_values()
                .def("__eq__",
                     [](const CommissionRateMode &a, int b)
                     {
                         return static_cast<int>(a) == b;
                     });

        py::enum_<ExecType>(m_enums, "ExecType", py::arithmetic())
                .value("Unknown", ExecType::Unknown)
                .value("Cancel", ExecType::Cancel)
                .value("Trade", ExecType::Trade)
                .export_values()
                .def("__eq__",
                     [](const ExecType &a, int b)
                     {
                         return static_cast<int>(a) == b;
                     }
                );

        py::enum_<Side>(m_enums, "Side", py::arithmetic())
                .value("Buy", Side::Buy)
                .value("Sell", Side::Sell)
                .value("Lock", Side::Lock)
                .value("Unlock", Side::Unlock)
                .value("Exec", Side::Exec)
                .value("Drop", Side::Drop)
                .export_values()
                .def("__eq__",
                     [](const Side &a, int b)
                     {
                         return static_cast<int>(a) == b;
                     }
                );

        py::enum_<Offset>(m_enums, "Offset", py::arithmetic())
                .value("Open", Offset::Open)
                .value("Close", Offset::Close)
                .value("CloseToday", Offset::CloseToday)
                .value("CloseYesterday", Offset::CloseYesterday)
                .export_values()
                .def("__eq__",
                     [](const Offset &a, int b)
                     {
                         return static_cast<int>(a) == b;
                     }
                );

        py::enum_<HedgeFlag>(m_enums, "HedgeFlag", py::arithmetic())
                .value("Speculation", HedgeFlag::Speculation)
                .value("Arbitrage", HedgeFlag::Arbitrage)
                .value("Hedge", HedgeFlag::Hedge)
                .value("Covered", HedgeFlag::Covered)
                .export_values()
                .def("__eq__",
                     [](const HedgeFlag &a, int b)
                     {
                         return static_cast<int>(a) == b;
                     }
                );

        py::enum_<BsFlag>(m_enums, "BsFlag", py::arithmetic())
                .value("Unknown", BsFlag::Unknown)
                .value("Buy", BsFlag::Buy)
                .value("Sell", BsFlag::Sell)
                .export_values()
                .def("__eq__",
                     [](const BsFlag &a, int b)
                     {
                         return static_cast<int>(a) == b;
                     });

        py::enum_<OrderStatus>(m_enums, "OrderStatus", py::arithmetic())
                .value("Unknown", OrderStatus::Unknown)
                .value("Submitted", OrderStatus::Submitted)
                .value("Pending", OrderStatus::Pending)
                .value("Cancelled", OrderStatus::Cancelled)
                .value("Error", OrderStatus::Error)
                .value("Filled", OrderStatus::Filled)
                .value("PartialFilledNotActive", OrderStatus::PartialFilledNotActive)
                .value("PartialFilledActive", OrderStatus::PartialFilledActive)
                .export_values()
                .def("__eq__",
                     [](const OrderStatus &a, int b)
                     {
                         return static_cast<int>(a) == b;
                     }
                );

        py::enum_<Direction>(m_enums, "Direction", py::arithmetic())
                .value("Long", Direction::Long)
                .value("Short", Direction::Short)
                .export_values()
                .def("__eq__",
                     [](const Direction &a, int b)
                     {
                         return static_cast<int>(a) == b;
                     }
                );

        py::enum_<PriceType>(m_enums, "PriceType", py::arithmetic())
                .value("Any", PriceType::Any)
                .value("FakBest5", PriceType::FakBest5)
                .value("Fak", PriceType::Fak)
                .value("Fok", PriceType::Fok)
                .value("Limit", PriceType::Limit)
                .value("ForwardBest", PriceType::ForwardBest)
                .value("ReverseBest", PriceType::ReverseBest)
                .export_values()
                .def("__eq__",
                     [](const PriceType &a, int b)
                     {
                         return static_cast<int>(a) == b;
                     }
                );

        py::enum_<VolumeCondition>(m_enums, "VolumeCondition", py::arithmetic())
                .value("Any", VolumeCondition::Any)
                .value("Min", VolumeCondition::Min)
                .value("All", VolumeCondition::All)
                .export_values()
                .def("__eq__",
                     [](const VolumeCondition &a, int b)
                     {
                         return static_cast<int>(a) == b;
                     }
                );

        py::enum_<TimeCondition>(m_enums, "TimeCondition", py::arithmetic())
                .value("IOC", TimeCondition::IOC)
                .value("GFD", TimeCondition::GFD)
                .value("GTC", TimeCondition::GTC)
                .export_values()
                .def("__eq__",
                     [](const TimeCondition &a, int b)
                     {
                         return static_cast<int>(a) == b;
                     }
                );

        py::enum_<OrderActionFlag>(m_enums, "OrderActionFlag", py::arithmetic())
                .value("Cancel", OrderActionFlag::Cancel)
                .export_values()
                .def("__eq__",
                     [](const OrderActionFlag &a, int b)
                     {
                         return static_cast<int>(a) == b;
                     });

        py::enum_<LedgerCategory>(m_enums, "LedgerCategory", py::arithmetic())
                .value("Account", LedgerCategory::Account)
                .value("Strategy", LedgerCategory::Strategy)
                .export_values()
                .def("__eq__",
                     [](const LedgerCategory &a, int b)
                     {
                         return static_cast<int>(a) == b;
                     });

        auto m_types = m.def_submodule("types");
        auto m_state = m.def_submodule("state");

        hana::for_each(StateDataTypes, [&](auto type)
        {
            using DataType = typename decltype(+hana::second(type))::type;

            auto py_class = py::class_<DataType>(m_types, hana::first(type).c_str());
            py_class.def(py::init<>());

            hana::for_each(hana::accessors<DataType>(), [&](auto it)
            {
                auto name = hana::first(it);
                auto accessor = hana::second(it);
                py_class.def_readwrite(name.c_str(), member_pointer_trait<decltype(accessor)>().pointer());
            });

            py_class.def("__repr__", &DataType::to_string);

            py_class.def_property_readonly("uid", &DataType::uid);
            py_class.def("__hash__", &DataType::uid);

            py_class.def("__eq__", [&](DataType &a, DataType &b)
            {
                return a.uid() == b.uid();
            });

            py_class.def("__sizeof__", [&](DataType &target)
            {
                return sizeof(target);
            });

            py::class_<kungfu::state<DataType>>(m_state, hana::first(type).c_str())
                    .def_readonly("source", &kungfu::state<DataType>::source)
                    .def_readonly("update_time", &kungfu::state<DataType>::update_time)
                    .def_readonly("data", &kungfu::state<DataType>::data);
        });
    }
}

#endif //KUNGFU_PY_LONGFIST_HPP
