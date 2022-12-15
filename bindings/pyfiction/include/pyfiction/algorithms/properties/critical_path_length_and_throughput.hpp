//
// Created by marcel on 15.12.22.
//

#ifndef PYFICTION_CRITICAL_PATH_LENGTH_AND_THROUGHPUT_HPP
#define PYFICTION_CRITICAL_PATH_LENGTH_AND_THROUGHPUT_HPP

#include "pybind11/pybind11.h"
#include "pyfiction/types.hpp"

#include <fiction/algorithms/properties/critical_path_length_and_throughput.hpp>

namespace pyfiction
{

namespace detail
{

template <typename Lyt>
void critical_path_length_and_throughput(pybind11::module& m)
{
    using namespace pybind11::literals;

    m.def(
        "critical_path_length_and_throughput",
        [](const Lyt& lyt) -> std::pair<uint64_t, uint64_t>
        {
            fiction::critical_path_length_and_throughput_stats stats{};
            fiction::critical_path_length_and_throughput(lyt, &stats);

            return {stats.critical_path_length, stats.throughput};
        },
        "layout"_a);
}

}  // namespace detail

inline void critical_path_length_and_throughput(pybind11::module& m)
{
    detail::critical_path_length_and_throughput<py_cartesian_gate_layout>(m);
    detail::critical_path_length_and_throughput<py_hexagonal_gate_layout>(m);
}

}  // namespace pyfiction

#endif  // PYFICTION_CRITICAL_PATH_LENGTH_AND_THROUGHPUT_HPP
