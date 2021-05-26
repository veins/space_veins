//
// Copyright (C) 2011 David Eckhoff <eckhoff@cs.fau.de>
// Copyright (C) 2021 Mario Franke <research@m-franke.net>
//
// Documentation for these modules is at http://veins.car2x.org/
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

#include "space_veins/base/modules/BaseSatelliteCarApplLayer.h"

using namespace space_veins;

void BaseSatelliteCarApplLayer::initialize(int stage)
{
    BaseApplLayer::initialize(stage);
    if (stage == 0) {

        // find gate
        satLowerLayerIn = findGate("satLowerLayerIn");
        satLowerLayerOut = findGate("satLowerLayerOut");
        satLowerControlIn = findGate("satLowerControlIn");
        satLowerControlOut = findGate("satLowerControlOut");
    }
}

void BaseSatelliteCarApplLayer::sendViaC2X(cMessage* msg)
{
    send(msg, lowerLayerOut);
}

void BaseSatelliteCarApplLayer::sendViaSatellite(cMessage* msg)
{
    send(msg, satLowerLayerOut);
}

void BaseSatelliteCarApplLayer::sendDelayedViaC2X(cMessage* msg, simtime_t delay)
{
    sendDelayed(msg, delay, lowerLayerOut);
}

void BaseSatelliteCarApplLayer::sendDelayedViaSatellite(cMessage* msg, simtime_t delay)
{
    sendDelayed(msg, delay, satLowerLayerOut);
}

void BaseSatelliteCarApplLayer::handleLowerMsg(cMessage* msg)
{
    EV_DEBUG << "BaseSatelliteCarApplLayer received msg from lower layer." << std::endl;
    delete (msg);
}

void BaseSatelliteCarApplLayer::handleSelfMsg(cMessage* msg)
{
    EV_DEBUG << "BaseSatelliteCarApplLayer received self msg." << std::endl;
}

void BaseSatelliteCarApplLayer::finish()
{
    EV_DEBUG << "BaseSatelliteCarApplLayer finished." << std::endl;
}

BaseSatelliteCarApplLayer::~BaseSatelliteCarApplLayer()
{
}
