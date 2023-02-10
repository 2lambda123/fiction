//
// Created by marcel on 03.06.22.
//

#ifndef PYFICTION_CLOCKED_LAYOUT_HPP
#define PYFICTION_CLOCKED_LAYOUT_HPP

#include "pyfiction/types.hpp"

#include <fiction/layouts/clocking_scheme.hpp>
#include <fiction/traits.hpp>

#include <fmt/format.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <set>
#include <string>

namespace pyfiction
{

namespace detail
{

template <typename LytBase, typename ClockedLyt>
void clocked_layout(pybind11::module& m, const std::string& topology)
{
    namespace py = pybind11;
    using namespace pybind11::literals;

    /**
     * Clocked Cartesian layout.
     */
    py::class_<ClockedLyt, LytBase>(m, fmt::format("clocked_{}_layout", topology).c_str())
        .def(py::init<>())
        .def(py::init<const fiction::aspect_ratio<ClockedLyt>&>(), "dimension"_a)
        .def(py::init(
                 [](const fiction::aspect_ratio<ClockedLyt>& dimension, const std::string& scheme_name)
                 {
                     if (const auto scheme = fiction::get_clocking_scheme<ClockedLyt>(scheme_name); scheme.has_value())
                     {
                         return ClockedLyt{dimension, *scheme};
                     }

                     throw std::runtime_error("Given name does not refer to a supported clocking scheme");
                 }),
             "dimension"_a, "clocking_scheme"_a = "2DDWave")

        .def("assign_clock_number", &ClockedLyt::assign_clock_number)
        .def("get_clock_number", &ClockedLyt::get_clock_number)
        .def("num_clocks", &ClockedLyt::num_clocks)
        .def("is_regularly_clocked", &ClockedLyt::is_regularly_clocked)

        .def("is_incoming_clocked", &ClockedLyt::is_incoming_clocked)
        .def("is_outgoing_clocked", &ClockedLyt::is_outgoing_clocked)

        .def("incoming_clocked_zones", &ClockedLyt::incoming_clocked_zones)
        .def("outgoing_clocked_zones", &ClockedLyt::outgoing_clocked_zones)

        .def("in_degree", &ClockedLyt::in_degree)
        .def("out_degree", &ClockedLyt::out_degree)
        .def("degree", &ClockedLyt::degree)

        ;
}

}  // namespace detail

void clocked_layouts(pybind11::module& m)
{
    detail::clocked_layout<py_cartesian_layout, py_cartesian_clocked_layout>(m, "cartesian");
    detail::clocked_layout<py_hexagonal_layout, py_hexagonal_clocked_layout>(m, "hexagonal");
}

}  // namespace pyfiction

#endif  // PYFICTION_CLOCKED_LAYOUT_HPP
