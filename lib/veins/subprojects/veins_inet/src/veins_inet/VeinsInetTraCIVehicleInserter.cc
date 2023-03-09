//
// Copyright (C) 2006-2018 Christoph Sommer <sommer@ccs-labs.org>
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
// -------------------------------------------------------------------------
//
// This file is added to the original version of Veins and used by
// space_Veins <https://github.com/veins/space_veins>.
// Author of this file is Mario Franke <research@m-franke.net>.
//

#include "veins_inet/VeinsInetTraCIVehicleInserter.h"
#include "veins_inet/VeinsInetMobility.h"

#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"

#include "veins/base/utils/FindModule.h"

Define_Module(veins::VeinsInetTraCIVehicleInserter);

using namespace veins;

VeinsInetTraCIVehicleInserter::VeinsInetTraCIVehicleInserter()
{
}

VeinsInetTraCIVehicleInserter::~VeinsInetTraCIVehicleInserter()
{
}

void VeinsInetTraCIVehicleInserter::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details)
{
    if (signalID == TraCIScenarioManager::traciModuleAddedSignal) {
        ASSERT(manager->isConnected());
        cModule* mod = check_and_cast<cModule*>(obj);
        auto* mob = FindModule<VeinsInetMobility*>::findSubModule(mod);
        ASSERT(mob != nullptr);
        std::string nodeId = mob->getExternalId();
        if (queuedVehicles.find(nodeId) != queuedVehicles.end()) {
            queuedVehicles.erase(nodeId);
        }
    }
}
