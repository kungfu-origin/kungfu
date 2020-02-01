//
// Created by Keren Dong on 2020/1/30.
//

#ifndef KUNGFU_PY_LONGFIST_HPP
#define KUNGFU_PY_LONGFIST_HPP

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <nlohmann/json.hpp>

#include <kungfu/longfist.h>

namespace kungfu::longfist
{
    namespace py = pybind11;
    namespace hana = boost::hana;

    void bind(pybind11::module &&m)
    {
        auto m_enum = m.def_submodule("enum");

        py::enum_<InstrumentType>(m_enum, "InstrumentType", py::arithmetic())
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

        py::enum_<CommissionRateMode>(m_enum, "CommissionRateMode", py::arithmetic())
                .value("ByAmount", CommissionRateMode::ByAmount)
                .value("ByVolume", CommissionRateMode::ByVolume)
                .export_values()
                .def("__eq__",
                     [](const CommissionRateMode &a, int b)
                     {
                         return static_cast<int>(a) == b;
                     });

        py::enum_<ExecType>(m_enum, "ExecType", py::arithmetic())
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

        py::enum_<Side>(m_enum, "Side", py::arithmetic())
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

        py::enum_<Offset>(m_enum, "Offset", py::arithmetic())
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

        py::enum_<HedgeFlag>(m_enum, "HedgeFlag", py::arithmetic())
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

        py::enum_<BsFlag>(m_enum, "BsFlag", py::arithmetic())
                .value("Unknown", BsFlag::Unknown)
                .value("Buy", BsFlag::Buy)
                .value("Sell", BsFlag::Sell)
                .export_values()
                .def("__eq__",
                     [](const BsFlag &a, int b)
                     {
                         return static_cast<int>(a) == b;
                     });

        py::enum_<OrderStatus>(m_enum, "OrderStatus", py::arithmetic())
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

        py::enum_<Direction>(m_enum, "Direction", py::arithmetic())
                .value("Long", Direction::Long)
                .value("Short", Direction::Short)
                .export_values()
                .def("__eq__",
                     [](const Direction &a, int b)
                     {
                         return static_cast<int>(a) == b;
                     }
                );

        py::enum_<PriceType>(m_enum, "PriceType", py::arithmetic())
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

        py::enum_<VolumeCondition>(m_enum, "VolumeCondition", py::arithmetic())
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

        py::enum_<TimeCondition>(m_enum, "TimeCondition", py::arithmetic())
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

        py::enum_<OrderActionFlag>(m_enum, "OrderActionFlag", py::arithmetic())
                .value("Cancel", OrderActionFlag::Cancel)
                .export_values()
                .def("__eq__",
                     [](const OrderActionFlag &a, int b)
                     {
                         return static_cast<int>(a) == b;
                     });

        py::enum_<LedgerCategory>(m_enum, "LedgerCategory", py::arithmetic())
                .value("Account", LedgerCategory::Account)
                .value("Strategy", LedgerCategory::Strategy)
                .export_values()
                .def("__eq__",
                     [](const LedgerCategory &a, int b)
                     {
                         return static_cast<int>(a) == b;
                     });

        auto m_types = m.def_submodule("types");

        hana::for_each(types::DATA_STRUCTS, [&](auto type)
        {
            using DataType = typename decltype(+hana::second(type))::type;

            auto py_class = py::class_<DataType>(m_types, hana::first(type).c_str());
            py_class.def(py::init<>());

            hana::for_each(hana::accessors<DataType>(), [&](auto it)
            {
                auto name = hana::first(it);
                auto accessor = hana::second(it);

                using RawType = decltype(accessor(std::forward<DataType &>(DataType{})));
                using AttrType = std::decay_t<RawType>;

                auto getter = [&](DataType &target)
                {
                    return accessor(target);
                };

                auto setter = [&](DataType &target, AttrType value)
                {
                    if constexpr (std::is_pointer<AttrType>::value)
                    {
                        memcpy(accessor(target), value, sizeof(RawType));
                    } else
                    {
                        accessor(target) = value;
                    }
                };

                py_class.def_property(name.c_str(), getter, setter);
            });

            py_class.def("__repr__", [&](DataType &target)
            {
                nlohmann::json j{};
                hana::for_each(hana::accessors<DataType>(), [&](auto it)
                {
                    auto name = hana::first(it);
                    auto accessor = hana::second(it);
                    j[name.c_str()] = accessor(target);
                });
                return j.dump();
            });

            py_class.def("__sizeof__", [&](DataType &target)
            {
                return sizeof(target);
            });
        });
    }
}

#endif //KUNGFU_PY_LONGFIST_HPP
