//
// Copyright (C) 2025 Mario Franke <research@m-franke.net>
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

package space_veins.modules.applications.UdpSinkAoI;

import inet.applications.udpapp.UdpSink;

simple UdpSinkAoI extends UdpSink
{
    parameters:
        @class(space_veins::UdpSinkAoI);
        @display("i=block/sink");

        @signal[space_veins_udpSinkAoI_ageOfInformation](type=simtime_t);
        @statistic[ageOfInformation](source=space_veins_udpSinkAoI_ageOfInformation; record=count, vector);
}