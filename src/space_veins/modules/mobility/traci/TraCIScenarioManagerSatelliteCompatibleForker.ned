//
// Copyright (C) 2006-2016 Christoph Sommer <christoph.sommer@uibk.ac.at>
// Copyright (C) 2021 Mario Franke <research@m-franke.net>
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

package space_veins.modules.mobility.traci;

import org.car2x.veins.modules.mobility.traci.TraCIScenarioManagerForker;

//
// Extends the TraCIScenarioManagerForker to fork with space_Veins
//
// All other functionality is provided by the TraCIScenarioManagerForker.
//
// See the space_Veins website <a href="http://sat.car2x.org/"> for a tutorial, documentation, and publications </a>.
//
// @author Christoph Sommer, Florian Hagenauer, Mario Franke
//
// @see TraCIMobility
// @see TraCIScenarioManager
//
simple TraCIScenarioManagerSatelliteCompatibleForker extends TraCIScenarioManagerForker
{
    parameters:
        @class(space_veins::TraCIScenarioManagerSatelliteCompatibleForker);
}

