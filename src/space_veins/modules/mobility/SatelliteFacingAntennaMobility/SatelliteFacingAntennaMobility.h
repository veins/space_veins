//
// Copyright (C) OpenSim Ltd.
// Copyright (C) 2022 Mario Franke <research@m-franke.net>
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

#include "space_veins/space_veins.h"
#include "inet/mobility/single/FacingMobility.h"

namespace space_veins {

class SPACE_VEINS_API SatelliteFacingAntennaMobility : public inet::FacingMobility {

public:
    SatelliteFacingAntennaMobility()
        : FacingMobility()
    {
    }
    ~SatelliteFacingAntennaMobility() override
    {
    }

protected:
    inet::deg azimuthDeg;
    inet::deg elevationDeg;
    inet::m distanceMeter;

protected:
    virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleSelfMessage(cMessage *msg) override { throw cRuntimeError("Unknown self message"); }

public:
    virtual inet::Coord getCurrentPosition() override;
    virtual inet::Coord getCurrentVelocity() override;
    virtual inet::Coord getCurrentAcceleration() override;

    virtual inet::Quaternion getCurrentAngularPosition() override;
    virtual inet::Quaternion getCurrentAngularVelocity() override { return inet::Quaternion::NIL; }
    virtual inet::Quaternion getCurrentAngularAcceleration() override { return inet::Quaternion::NIL; }
};
} // namespace space_veins
