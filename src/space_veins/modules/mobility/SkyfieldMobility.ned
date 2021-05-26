// SPDX-FileCopyrightText: 2021 Mario Franke <research@m-franke.net>
//
// SPDX-License-Identifier: GPL-2.0-or-later

package space_veins.modules.mobility;

import org.car2x.veins.base.modules.BaseMobility;

// Mobility module for spacecrafts based on the python library Skyfield

simple SkyfieldMobility extends BaseMobility
{
    parameters:
        @class(space_veins::SkyfieldMobility);
        double updateInterval @unit("s") = default(0.1s); // time interval to update the hosts position (seconds)
        double referencePointLatitude = default(49.583332); // latitude value of the reference point
        double referencePointLongitude = default(11.016667); // longitude value of the reference point
//         double elevationAngle @unit(deg) = default(0); // elevation angle of the satellite with respect to the reference position.
//         double azimuthAngle @unit(deg) = default(0); // azimuth angle of the satellite with respect to the reference position.
//         double distance @unit(m) = default(-1); // distance between spacecraft and reference point. -1 means the spacecraft is below horizon.
}

