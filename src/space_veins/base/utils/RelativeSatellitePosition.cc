// SPDX-FileCopyrightText: 2021 Mario Franke <research@m-franke.net>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "space_veins/base/utils/RelativeSatellitePosition.h"

namespace space_veins{

std::ostream& operator<<(std::ostream& os, const RelativeSatellitePosition& rsp)
{
    os << "altitude_deg: " << rsp.altitude_deg << ", azimuth_deg: " << rsp.azimuth_deg << ", distance_km: " << rsp.distance_km;
    return os;
}

} // end namespace space_veins
