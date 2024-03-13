#ifndef DISPI_BQ27427_BQ27427_CFG_TEMPLATE_H_
#define DISPI_BQ27427_BQ27427_CFG_TEMPLATE_H_

// chem id == maximum charging voltage
#define BQ27427_CHEM_ID         BQ27427_CHEM_ID_4_35_V

// Design Capacity = nominal battery capacity
#define BQ27427_DESIGN_CAPACITY_mAh     1340

// Design Energy = Design Capacity x battery nominal voltage (Design Voltage)
#define BQ27427_DESIGN_ENERGY_mWh       4960

// Terminate Voltage = minimum operating voltage of your system (0% capacity)
#define BQ27427_TERMINATE_VOLTAGE_mV    3200

// Taper Rate = Design Capacity / (0.1 x Taper Current)
// Should be set to a value slightly higher than the taper current detection threshold of the charger
// Taper Current: chi carica smette a questa corrente
#define BQ27427_TAPER_RATE_hx10         100

#else
#   warning bq27427_cfg_template.h incluso
#endif
