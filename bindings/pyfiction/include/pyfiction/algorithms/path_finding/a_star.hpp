//
// Created by marcel on 15.12.22.
//

#ifndef PYFICTION_A_STAR_HPP
#define PYFICTION_A_STAR_HPP

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pyfiction/types.hpp"

#include <fiction/algorithms/path_finding/a_star.hpp>
#include <fiction/traits.hpp>
#include <fiction/utils/routing_utils.hpp>

namespace pyfiction
{

namespace detail
{

template <typename Lyt>
void a_star(pybind11::module& m)
{
    using namespace pybind11::literals;

    m.def(
        "a_star",
        [](const Lyt& lyt, const fiction::coordinate<Lyt>& source, const fiction::coordinate<Lyt>& target,
           const fiction::a_star_params& params)
        {
            return static_cast<std::vector<fiction::coordinate<Lyt>>>(
                fiction::a_star<fiction::layout_coordinate_path<Lyt>, Lyt>(lyt, {source, target},
                                                                           fiction::manhattan_distance_functor<Lyt>(),
                                                                           fiction::unit_cost_functor<Lyt>(), params));
        },
        "layout"_a, "source"_a, "target"_a, "params"_a = fiction::a_star_params{});

    m.def("a_star_distance", &fiction::a_star_distance<Lyt, double>, "layout"_a, "source"_a, "target"_a);
}

}  // namespace detail

inline void a_star(pybind11::module& m)
{
    namespace py = pybind11;

    py::class_<fiction::a_star_params>(m, "a_star_params")
        .def(py::init<>())
        .def_readwrite("crossings", &fiction::a_star_params::crossings)

        ;

    detail::a_star<py_cartesian_clocked_layout>(m);
    detail::a_star<py_cartesian_gate_layout>(m);
    detail::a_star<py_hexagonal_clocked_layout>(m);
    detail::a_star<py_hexagonal_gate_layout>(m);
}

}  // namespace pyfiction

#endif  // PYFICTION_A_STAR_HPP