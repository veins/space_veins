// Copyright (C) 2026 Mario Franke <research@m-franke.net>
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

#pragma once

#include "inet/applications/udpapp/UdpBasicApp.h"

#include "space_veins/space_veins.h"
#include "space_veins/modules/mobility/SGP4Mobility/SGP4MobilityBase.h"

namespace space_veins {

class SPACE_VEINS_API UdpTxOverHorizon : public inet::UdpBasicApp {

public:
    UdpTxOverHorizon()
        : UdpBasicApp()
    {
    }

    ~UdpTxOverHorizon() override
    {
    }
protected:
    // methodes
    virtual void initialize(int stage) override;
    virtual void processSend() override;
protected:
    SGP4MobilityBase* sgp4Mobility;

    bool enableHorizonThreshold;
    double horizonThreshold;
};
} // end namespace space_Veins