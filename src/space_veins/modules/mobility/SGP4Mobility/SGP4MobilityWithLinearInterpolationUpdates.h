//
// Copyright (C) 2006-2012 Christoph Sommer <christoph.sommer@uibk.ac.at>
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

#pragma once

#include "space_veins/modules/mobility/SGP4Mobility/SGP4MobilityBase.h"

namespace space_veins {

class SPACE_VEINS_API SGP4MobilityWithLinearInterpolationUpdates : public SGP4MobilityBase {

public:
    SGP4MobilityWithLinearInterpolationUpdates()
        : SGP4MobilityBase(false)
    {
    }

    ~SGP4MobilityWithLinearInterpolationUpdates() override
    {
    }

protected:
    // methods

    void initialize(int) override;
    void finish() override;
    bool nextPeriodicPositionUpdate() override;
    bool nextGetCurrentPositionUpdate() override;
    void updateManagementDataAndStatistics(updateType_t type, bool updated) override;
    inet::Coord linearInterpolation(simtime_t t);
    void nextLinearInterpolationPositionUpdate();
    void recordLinearInterpolationSpeedMeterPerSecond(const double speed);
    void recordLinearInterpolationComputation(inet::deg elevationAngleToSop);

    // linear interpolation mode for getCurrentPosition()
    calculatedSgp4SatellitePositions_t linearInterpolationReference; // the satellite position used for linear interpolation, also used for returnNearestPeriodicPosition, it should always contain the satellite's position at lastPeriodicUpdate + updateInterval
    inet::Coord linearInterpolationVector = inet::Coord::NIL; // direction used for linear interpolation, vector is normalized sucht that it can be multiplied with a velocity
    double linearInterpolationSpeedMeterPerSecond;

    // Signals
    static const simsignal_t linearInterpolationSpeedMeterPerSecondSignal;
    static const simsignal_t linearInterpolationComputationsSignal;

    // Statistics
    int numberOfLinearInterpolationComputations = 0;

    class SPACE_VEINS_API SGP4MobilityWithLinearInterpolationUpdatesAccess {
    public:
        SGP4MobilityWithLinearInterpolationUpdates* get(cModule* host)
        {
            SGP4MobilityWithLinearInterpolationUpdates* sgp4 = veins::FindModule<SGP4MobilityWithLinearInterpolationUpdates*>::findSubModule(host);
            ASSERT(sgp4);
            return sgp4;
        };
    };
};
} // namespace space_veins
