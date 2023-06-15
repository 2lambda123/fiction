from fiction.pyfiction import *
import unittest
import os


class TestChargeDistributionSurface(unittest.TestCase):

    def test_initialization(self):
        layout_one = sidb_layout((10, 10))
        layout_one.assign_cell_type((0, 1), sidb_technology.cell_type.NORMAL)
        layout_one.assign_cell_type((4, 1), sidb_technology.cell_type.NORMAL)
        layout_one.assign_cell_type((6, 1), sidb_technology.cell_type.NORMAL)

        charge_lyt = charge_distribution_surface(layout_one)

        self.assertEqual(charge_lyt.get_charge_state((0, 0, 1)), sidb_charge_state.NEGATIVE)
        self.assertEqual(charge_lyt.get_charge_state((4, 0, 1)), sidb_charge_state.NEGATIVE)
        self.assertEqual(charge_lyt.get_charge_state((6, 0, 1)), sidb_charge_state.NEGATIVE)

        charge_lyt.assign_charge_state((0, 0, 1), sidb_charge_state.NEUTRAL)
        charge_lyt.assign_charge_state((4, 0, 1), sidb_charge_state.NEGATIVE)
        charge_lyt.assign_charge_state((6, 0, 1), sidb_charge_state.NEGATIVE)
        self.assertEqual(charge_lyt.get_charge_state((0, 0, 1)), sidb_charge_state.NEUTRAL)
        self.assertEqual(charge_lyt.get_charge_state((4, 0, 1)), sidb_charge_state.NEGATIVE)
        self.assertEqual(charge_lyt.get_charge_state((6, 0, 1)), sidb_charge_state.NEGATIVE)
        charge_lyt.update_after_charge_change()
        self.assertFalse(charge_lyt.is_physically_valid())

        charge_lyt.assign_charge_state((0, 0, 1), sidb_charge_state.NEGATIVE)
        charge_lyt.assign_charge_state((4, 0, 1), sidb_charge_state.NEUTRAL)
        charge_lyt.assign_charge_state((6, 0, 1), sidb_charge_state.NEGATIVE)
        charge_lyt.update_after_charge_change()
        self.assertTrue(charge_lyt.is_physically_valid())

        self.assertNotEqual(charge_lyt.get_system_energy(), 0)

        charge_lyt.set_system_energy_to_zero()

        self.assertEqual(charge_lyt.get_system_energy(), 0)


if __name__ == '__main__':
    unittest.main()
