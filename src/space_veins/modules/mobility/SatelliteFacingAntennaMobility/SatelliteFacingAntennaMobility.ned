//
// Copyright (C) 2022 Mario Franke <research@m-franke.net>
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

package space_veins.modules.mobility.SatelliteFacingAntennaMobility;

import inet.mobility.single.FacingMobility;

//
// Mobility module for vehicle antennas which should face in the direction
// of a satellite. This mobility is meant to replace a construct of a
// inet::SuperpositioningMobility based on an AttachedMobility and a
// inet::FacingMobility.
// The orientation of the SuperpositioningMobility will not face the
// FacingMobility's targetMobility when the AttachedMobility's
// orientation is not the default one pointing along OMNeT++'s X-axis.
// The SuperpositioningMobility will add the AttachedMobility's orientation
// and the orientation of the FacingMobility. As a result, the
// SuperpositioningMobility will not face the targetMobility.
//
// The sourceMobility should always be assigned to the nodes's mobility to
// which the antenna is attached to. The SatelliteFacingAntennaMobility's
// position is based on the subjectModule's position.
//
// The SatelliteFacingAntennaMobility's orientation is inherited from
// inet::FacingMobility. Thus, it faces towards the targetMobility.
//
simple SatelliteFacingAntennaMobility extends FacingMobility
{
    parameters:
        @class(space_veins::SatelliteFacingAntennaMobility);
}
