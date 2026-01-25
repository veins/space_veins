//
// Copyright (C) 2026 Mario Franke <research@m-franke.net>
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
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

package space_veins.modules.applications.UdpTxOverHorizon;

import inet.applications.udpapp.UdpBasicApp;

simple UdpTxOverHorizon extends UdpBasicApp
{
    parameters:
        @class(space_veins::UdpTxOverHorizon);
        @display("i=block/sink");
        bool enableHorizonThreshold = default(true);
        double horizonThreshold = default(-10000m) @unit(m); // threshold values for transmitting packets. Only if the satellite's z-coordinate is greater then threshold, packets will be transmitted.
}