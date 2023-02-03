//
// Created by Jan Drewniok on 18.01.23.
//

#ifndef FICTION_IS_GROUNDSTATE_HPP
#define FICTION_IS_GROUNDSTATE_HPP

#include "fiction/algorithms/simulation/sidb/exhaustive_ground_state_simulation.hpp"
#include "fiction/algorithms/simulation/sidb/minimum_energy.hpp"
#include "fiction/algorithms/simulation/sidb/quicksim.hpp"
#include "fiction/traits.hpp"

#include <cmath>

namespace fiction
{

/**
 * This function checks if the ground state is found by the *quicksim* algorithm.
 *
 * @tparam Lyt Cell-level layout type.
 * @param quicksim_results All found physically valid charge distribution surfaces obtained by the quicksim algorithm
 * (see quicksim.hpp).
 * @param exhaustive_results All valid charge distribution surfaces determined by ExGS (see
 * exhaustive_ground_state_simulation.hpp).
 * @return Returns `true` if the relative difference between the lowest energies of the two sets is less than 0.00001,
 * `false` otherwise.
 */
template <typename Lyt>
[[nodiscard]] bool is_groundstate(const quicksim_stats<Lyt>& quicksim_results,
                                  const exgs_stats<Lyt>&     exhaustive_results)
{
    static_assert(is_cell_level_layout_v<Lyt>, "Lyt is not a cell-level layout");
    static_assert(has_sidb_technology_v<Lyt>, "Lyt is not an SiDB layout");
    static_assert(has_siqad_coord_v<Lyt>, "Lyt is not based on SiQAD coordinates");

    if (exhaustive_results.valid_lyts.empty())
    {
        return false;
    }

    const auto min_energy_exact  = minimum_energy(exhaustive_results.valid_lyts);
    const auto min_energy_new_ap = minimum_energy(quicksim_results.valid_lyts);

    return std::abs(min_energy_exact - min_energy_new_ap) / min_energy_exact < physical_constants::POP_STABILITY_ERR;
}

}  // namespace fiction

#endif  // FICTION_IS_GROUNDSTATE_HPP