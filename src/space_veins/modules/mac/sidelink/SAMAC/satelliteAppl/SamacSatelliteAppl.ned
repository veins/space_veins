//
// Copyright (C) 2018 Christoph Sommer <sommer@ccs-labs.org>
// Copyright (C) 2023 Mario Franke <research@m-franke.net>
//
// Documentation for these modules is at http://sat.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

package space_veins.modules.mac.sidelink.SAMAC.satelliteAppl;

import org.car2x.veins.subprojects.veins_inet.VeinsInetApplicationBase;

simple SamacSatelliteAppl extends VeinsInetApplicationBase
{
    parameters:
        @class(SamacSatelliteAppl);
        double wlanSlot_s = default(0.001s) @unit(s);    // wlan slot duration
        double wlanScheduleInterval_s = default(1s) @unit(s);    // wlan schedule interval
        double maxV2SLatency_s = default(0.005s) @unit(s);     // Vehicle-to-Satellite latency to be considered by the wlan schedule
    gates:
}
