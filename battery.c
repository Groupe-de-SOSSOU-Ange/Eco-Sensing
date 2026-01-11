#include "battery.h"

double consume_battery(double battery_energy, double E_tx) {
    return battery_energy - E_tx;
}
