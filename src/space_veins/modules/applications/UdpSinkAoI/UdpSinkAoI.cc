// Copyright (C) 2025 Mario Franke <research@m-franke.net>
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

#include "space_veins/modules/applications/UdpSinkAoI/UdpSinkAoI.h"

using namespace space_veins;

Define_Module(space_veins::UdpSinkAoI);

void UdpSinkAoI::processPacket(inet::Packet *msg)
{
    UdpSink::processPacket(msg);
    if (lastReception >= SimTime::ZERO) {
        simtime_t now = simTime();
        simtime_t timeSinceLastReception = now - lastReception;
        lastReception = now;
        ageOfInformation += 0.5 * timeSinceLastReception;
    }
}