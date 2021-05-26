// SPDX-FileCopyrightText: 2021 Mario Franke <research@m-franke.net>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <iostream>

#include "space_veins/space_veins.h"

namespace space_veins {

class SPACE_VEINS_API RelativeSatellitePosition {
    public:
        double altitude_deg;
        double azimuth_deg;
        double distance_km;

    public:
        RelativeSatellitePosition()
            : altitude_deg(0.0)
            , azimuth_deg(0.0)
            , distance_km(0.0)
        {
        }

        RelativeSatellitePosition(double pAltitdue_deg, double pAzimuth_deg, double pDistance_km)
            : altitude_deg(pAltitdue_deg)
            , azimuth_deg(pAzimuth_deg)
            , distance_km(pDistance_km)
        {
        }

        friend std::ostream& operator<<(std::ostream& os, const RelativeSatellitePosition& rsp);

};
}  // namespace space_veins
