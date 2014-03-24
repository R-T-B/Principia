// Units.h

#pragma once

#include "PhysicalQuantities.h"
#include "NamedQuantities.h"

namespace PhysicalQuantities {
#pragma region SI base units
// From the BIPM's SI brochure 8, section 2.1.2, table 1,
// http://www.bipm.org/en/si/si_brochure/chapter2/2-1/.
Unit<Length>            const Metre    = Unit<Length>(Metres(1));
Unit<Mass>              const Kilogram = Unit<Mass>(Kilograms(1));
Unit<Time>              const Second   = Unit<Time>(Seconds(1));
Unit<Current>           const Ampere   = Unit<Current>(Amperes(1));
Unit<Temperature>       const Kelvin   = Unit<Temperature>(Kelvins(1));
Unit<Amount>            const Mole     = Unit<Amount>(Moles(1));
Unit<LuminousIntensity> const Candela  = Unit<LuminousIntensity>(Candelas(1));
// Nonstandard.
Unit<Winding>           const Cycle    = Unit<Winding>(Cycles(1));
#pragma endregion
#pragma region Coherent derived units in the SI with special names and symbols
// From the BIPM's SI brochure 8, section 2.2.2, table 3,
// http://www.bipm.org/en/si/si_brochure/chapter2/2-2/table3.html.
// We exclude the Becquerel, Gray and Sievert as they are weakly typed.
// The Celsius only really makes sense as an affine temperature and is not taken
// care of here.
// Note the nonstandard definition of the Hertz, with a dimensionful cycle.

// The radian and steradian are mostly useless, but we keep them in case their
// can improve readability.
Unit<Angle>      const Radian    = Unit<Angle>(Dimensionless(1));
Unit<SolidAngle> const Steradian = Unit<SolidAngle>(Dimensionless(1));
// Dimensionful units.
Unit<Frequency>           const Hertz   = Cycle / Second;
Unit<Force>               const Newton  = Metre * Kilogram / (Second * Second);
Unit<Pressure>            const Pascal  = Newton / (Metre * Metre);
Unit<Energy>              const Joule   = Newton * Metre;
Unit<Power>               const Watt    = Joule / Second;
Unit<Charge>              const Coulomb = Ampere * Second;
Unit<Voltage>             const Volt    = Watt / Ampere;
Unit<Capacitance>         const Farad   = Coulomb / Volt;
Unit<Resistance>          const Ohm     = Volt / Ampere;
Unit<Conductance>         const Siemens = Ampere / Volt;
Unit<MagneticFlux>        const Weber   = Volt * Second;
Unit<MagneticFluxDensity> const Tesla   = Weber / (Metre * Metre);
Unit<Inductance>          const Henry   = Weber / Ampere;
Unit<LuminousFlux>        const Lumen   = Candela / Steradian;
Unit<CatalyticActivity>   const Katal   = Mole / Second;
#pragma endregion
#pragma region Non-SI units accepted for use with the SI
// From the BIPM's SI brochure 8, section 4.1, table 6,
Unit<Volume>   const Litre  = Unit<Volume>(1e-3 * (Metre * Metre * Metre));
#pragma endregion
}