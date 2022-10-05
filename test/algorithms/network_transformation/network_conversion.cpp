//
// Created by marcel on 21.05.21.
//

#include "catch.hpp"
#include "utils/blueprints/layout_blueprints.hpp"
#include "utils/blueprints/network_blueprints.hpp"
#include "utils/equivalence_checking_utils.hpp"

#include <fiction/algorithms/network_transformation/fanout_substitution.hpp>
#include <fiction/algorithms/network_transformation/network_balancing.hpp>
#include <fiction/algorithms/network_transformation/network_conversion.hpp>
#include <fiction/layouts/cartesian_layout.hpp>
#include <fiction/layouts/clocked_layout.hpp>
#include <fiction/layouts/gate_level_layout.hpp>
#include <fiction/layouts/tile_based_layout.hpp>
#include <fiction/networks/technology_network.hpp>

#include <mockturtle/networks/aig.hpp>
#include <mockturtle/networks/buffered.hpp>
#include <mockturtle/networks/crossed.hpp>
#include <mockturtle/networks/mig.hpp>
#include <mockturtle/networks/xag.hpp>
#include <mockturtle/traits.hpp>

#include <type_traits>

using namespace fiction;

template <typename Ntk>
void to_x(const Ntk& ntk)
{
    SECTION("MIG")
    {
        const auto converted_mig = convert_network<mockturtle::mig_network>(ntk);

        check_eq(ntk, converted_mig);
    }

    SECTION("AIG")
    {
        const auto converted_aig = convert_network<mockturtle::aig_network>(ntk);

        check_eq(ntk, converted_aig);
    }

    SECTION("XAG")
    {
        const auto converted_xag = convert_network<mockturtle::xag_network>(ntk);

        check_eq(ntk, converted_xag);
    }

    SECTION("TEC")
    {
        const auto converted_tec = convert_network<technology_network>(ntk);

        check_eq(ntk, converted_tec);
    }
}

template <typename Ntk>
uint32_t get_num_buffers(const Ntk& ntk)
{
    static_assert(mockturtle::is_network_type_v<Ntk>, "Ntk is not a network type");
    static_assert(mockturtle::has_is_pi_v<Ntk>, "Ntk does not implement the is_pi function");
    static_assert(mockturtle::has_is_constant_v<Ntk>, "Ntk does not implement the is_constant function");
    static_assert(mockturtle::has_foreach_node_v<Ntk>, "Ntk does not implement the foreach_node function");
    static_assert(mockturtle::has_is_buf_v<Ntk>, "Ntk does not implement the is_buf function");

    uint32_t num_buffers = 0u;

    ntk.foreach_node(
        [&ntk, &num_buffers](const auto& n)
        {
            if (ntk.is_constant(n) || ntk.is_pi(n))
            {
                return;
            }

            if (ntk.is_buf(n))
            {
                ++num_buffers;
            }
        });

    return num_buffers;
}

template <typename Ntk>
void to_buf_x(const Ntk& ntk)
{
    SECTION("BUF MIG")
    {
        const auto converted_mig = convert_network<mockturtle::buffered_mig_network>(ntk);

        CHECK(get_num_buffers(ntk) == get_num_buffers(converted_mig));
        check_eq(ntk, converted_mig);
    }

    SECTION("BUF AIG")
    {
        const auto converted_aig = convert_network<mockturtle::buffered_aig_network>(ntk);

        CHECK(get_num_buffers(ntk) == get_num_buffers(converted_aig));
        check_eq(ntk, converted_aig);
    }
}

template <typename Ntk>
uint32_t get_num_crossings(const Ntk& ntk)
{
    static_assert(mockturtle::is_network_type_v<Ntk>, "Ntk is not a network type");
    static_assert(mockturtle::has_is_pi_v<Ntk>, "Ntk does not implement the is_pi function");
    static_assert(mockturtle::has_is_constant_v<Ntk>, "Ntk does not implement the is_constant function");
    static_assert(mockturtle::has_foreach_node_v<Ntk>, "Ntk does not implement the foreach_node function");

    uint32_t num_crossings = 0u;

    ntk.foreach_node(
        [&ntk, &num_crossings](const auto& n)
        {
            if (ntk.is_constant(n) || ntk.is_pi(n))
            {
                return;
            }

            if constexpr (mockturtle::has_is_crossing_v<Ntk>)
            {
                if (ntk.is_crossing(n))
                {
                    ++num_crossings;
                }
            }
            else if constexpr (is_gate_level_layout_v<Ntk>)
            {
                if (ntk.is_buf(n))
                {
                    if (const auto& at = ntk.above(ntk.get_tile(n));
                        at != ntk.get_tile(n) && ntk.is_buf(ntk.get_node(at)))
                    {
                        ++num_crossings;
                    }
                }
            }
        });

    return num_crossings;
}

template <typename Ntk>
void to_cross_x(const Ntk& ntk)
{
    SECTION("CROSS kLUT")
    {
        const auto converted_klut = convert_network<mockturtle::crossed_klut_network>(ntk);

        CHECK(get_num_crossings(ntk) == get_num_crossings(converted_klut));
        check_eq(ntk, converted_klut);
    }
    SECTION("CROSS BUF kLUT")
    {
        const auto converted_klut = convert_network<mockturtle::buffered_crossed_klut_network>(ntk);

        CHECK(get_num_buffers(ntk) == get_num_buffers(converted_klut));
        CHECK(get_num_crossings(ntk) == get_num_crossings(converted_klut));
        check_eq(ntk, converted_klut);
    }
}

TEST_CASE("Name conservation", "[network-conversion]")
{
    auto maj = blueprints::maj1_network<mockturtle::names_view<mockturtle::mig_network>>();
    maj.set_network_name("maj");

    const auto converted_maj = convert_network<mockturtle::names_view<fiction::technology_network>>(maj);

    // network name
    CHECK(converted_maj.get_network_name() == "maj");

    // PI names
    CHECK(converted_maj.get_name(converted_maj.make_signal(2)) == "a");
    CHECK(converted_maj.get_name(converted_maj.make_signal(3)) == "b");
    CHECK(converted_maj.get_name(converted_maj.make_signal(4)) == "c");

    // PO names
    CHECK(converted_maj.get_output_name(0) == "f");
}

TEST_CASE("Simple network conversion", "[network-conversion]")
{
    SECTION("MIG to X")
    {
        const auto mig = blueprints::maj1_network<mockturtle::mig_network>();

        to_x(mig);
    }
    SECTION("AIG to X")
    {
        const auto aig = blueprints::maj1_network<mockturtle::aig_network>();

        to_x(aig);
    }
    SECTION("XAG to X")
    {
        const auto xag = blueprints::maj1_network<mockturtle::xag_network>();

        to_x(xag);
    }
    SECTION("TEC to X")
    {
        const auto tec = blueprints::maj1_network<fiction::technology_network>();

        to_x(tec);
    }
}

TEST_CASE("Complex network conversion", "[network-conversion]")
{
    SECTION("MIG to X")
    {
        to_x(blueprints::maj4_network<mockturtle::mig_network>());
        to_x(blueprints::nary_operation_network<mockturtle::mig_network>());
    }
    SECTION("AIG to X")
    {
        to_x(blueprints::maj4_network<mockturtle::aig_network>());
        to_x(blueprints::nary_operation_network<mockturtle::aig_network>());
    }
    SECTION("XAG to X")
    {
        to_x(blueprints::maj4_network<mockturtle::xag_network>());
        to_x(blueprints::nary_operation_network<mockturtle::xag_network>());
    }
    SECTION("TEC to X")
    {
        to_x(blueprints::maj4_network<fiction::technology_network>());
        to_x(blueprints::nary_operation_network<fiction::technology_network>());
    }
}

TEST_CASE("Constant inverted signal recovery conversion", "[network-conversion]")
{
    SECTION("MIG to X")
    {
        to_x(blueprints::constant_gate_input_maj_network<mockturtle::mig_network>());
    }
    SECTION("AIG to X")
    {
        to_x(blueprints::constant_gate_input_maj_network<mockturtle::aig_network>());
    }
    SECTION("XAG to X")
    {
        to_x(blueprints::constant_gate_input_maj_network<mockturtle::xag_network>());
    }
    // no test for technology_network because it does not support inverted signals
}

TEST_CASE("Layout conversion", "[network-conversion]")
{
    using gate_layout = gate_level_layout<clocked_layout<tile_based_layout<cartesian_layout<offset::ucoord_t>>>>;

    SECTION("Gate layout to X")
    {
        to_x(blueprints::and_or_gate_layout<gate_layout>());
        to_x(blueprints::xor_maj_gate_layout<gate_layout>());
    }
    SECTION("Gate layout to buffered X")
    {
        to_buf_x(blueprints::unbalanced_and_layout<gate_layout>());
        to_buf_x(blueprints::crossing_layout<gate_layout>());
    }
    SECTION("Gate layout to crossed X")
    {
        to_cross_x(blueprints::unbalanced_and_layout<gate_layout>());
        to_cross_x(blueprints::crossing_layout<gate_layout>());
    }
}

TEST_CASE("Consistent network size after multiple conversions", "[network-conversion]")
{
    const auto tec = blueprints::se_coloring_corner_case_network<technology_network>();

    const auto converted = convert_network<technology_network>(
        convert_network<technology_network>(convert_network<technology_network>(tec)));

    CHECK(tec.size() == converted.size());

    const auto converted_aig = convert_network<technology_network>(blueprints::maj4_network<mockturtle::aig_network>());

    const auto converted_converted_aig = convert_network<technology_network>(
        convert_network<technology_network>(convert_network<technology_network>(converted_aig)));

    CHECK(converted_aig.size() == converted_converted_aig.size());
}

TEST_CASE("Consistent network size after fanout substitution and conversion", "[network-conversion]")
{
    const auto substituted_aig =
        fanout_substitution<technology_network>(blueprints::maj4_network<mockturtle::aig_network>());
    const auto converted_substituted_aig = convert_network<technology_network>(substituted_aig);
    CHECK(substituted_aig.size() == converted_substituted_aig.size());

    const auto substituted_tec = fanout_substitution<technology_network>(
        blueprints::fanout_substitution_corner_case_network<technology_network>());
    const auto converted_substituted_tec = convert_network<technology_network>(substituted_tec);
    CHECK(substituted_tec.size() == converted_substituted_tec.size());
}

TEST_CASE("Consistent network size after balancing and conversion", "[network-conversion]")
{
    const auto balanced_aig =
        network_balancing<technology_network>(blueprints::maj4_network<mockturtle::aig_network>());
    const auto converted_balanced_aig = convert_network<technology_network>(balanced_aig);
    CHECK(balanced_aig.size() == converted_balanced_aig.size());

    const auto balanced_tec = network_balancing<technology_network>(
        blueprints::fanout_substitution_corner_case_network<technology_network>());
    const auto converted_balanced_tec = convert_network<technology_network>(balanced_tec);
    CHECK(balanced_tec.size() == converted_balanced_tec.size());
}
