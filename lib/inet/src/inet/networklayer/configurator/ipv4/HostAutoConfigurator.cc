/*
 * Copyright (C) 2009 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * ---------------------------------------------------------------------------------
 *
 * This file is modified compared to the original version of INET such that it works
 * with space_Veins <https://github.com/veins/space_veins>. Please use a tool like
 * 'meld' in order to see the differences.
 * Author of the modifications is Mario Franke <research@m-franke.net>.
 *
 */

#include <algorithm>
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/configurator/ipv4/HostAutoConfigurator.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"

namespace inet {

Define_Module(HostAutoConfigurator);

void HostAutoConfigurator::initialize(int stage)
{
    if (stage == INITSTAGE_NETWORK_CONFIGURATION) {
        for (int i = 0; i < interfaceTable->getNumInterfaces(); i++)
            interfaceTable->getInterface(i)->addProtocolData<Ipv4InterfaceData>();
    }
    OperationalBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
    }
//     else if (stage == INITSTAGE_NETWORK_INTERFACE_CONFIGURATION) {
//         for (int i = 0; i < interfaceTable->getNumInterfaces(); i++)
//             interfaceTable->getInterface(i)->addProtocolData<Ipv4InterfaceData>();
//     }
}

void HostAutoConfigurator::finish()
{
}

void HostAutoConfigurator::handleMessageWhenUp(cMessage *apMsg)
{
}

void HostAutoConfigurator::setupNetworkLayer()
{
    EV_INFO << "host auto configuration started" << std::endl;

    std::string interfaces = par("interfaces");

    Ipv4Address addressBaseSatelliteNic = Ipv4Address(par("addressBaseSatelliteNic").stringValue());
    Ipv4Address netmaskSatelliteNic = Ipv4Address(par("netmaskSatelliteNic").stringValue());

    Ipv4Address addressBaseWlanNic = Ipv4Address(par("addressBaseWlanNic").stringValue());
    Ipv4Address netmaskWlanNic = Ipv4Address(par("netmaskWlanNic").stringValue());

    std::string mcastGroups = par("mcastGroups").stdstringValue();

    // get our host module
    cModule *host = getContainingNode(this);

    Ipv4Address myAddressSatelliteNic = Ipv4Address(addressBaseSatelliteNic.getInt() + uint32(host->getId()));
    Ipv4Address myAddressWlanNic = Ipv4Address(addressBaseWlanNic.getInt() + uint32(host->getId()));

    // address test
    if (!Ipv4Address::maskedAddrAreEqual(myAddressSatelliteNic, addressBaseSatelliteNic, netmaskSatelliteNic))
        throw cRuntimeError("Generated IP address for the satellite nic is out of specified address range");
    if (!Ipv4Address::maskedAddrAreEqual(myAddressWlanNic, addressBaseWlanNic, netmaskWlanNic))
        throw cRuntimeError("Generated IP address for the wlan nic is out of specified address range");

    // get our routing table
    IIpv4RoutingTable *routingTable = L3AddressResolver().getIpv4RoutingTableOf(host);
    if (!routingTable)
        throw cRuntimeError("No routing table found");

    // look at all interface table entries
    cStringTokenizer interfaceTokenizer(interfaces.c_str());
    const char *ifname;
    while ((ifname = interfaceTokenizer.nextToken()) != nullptr) {
        InterfaceEntry *ie = interfaceTable->findInterfaceByName(ifname);
        if (!ie)
            throw cRuntimeError("No such interface '%s'", ifname);

        auto ipv4Data = ie->getProtocolData<Ipv4InterfaceData>();
        // assign IP Address to all connected interfaces
        // TODO: Does not recognize loopback interface -> gets not Ipv4Address
        if (ie->isLoopback()) {
            EV_INFO << "interface " << ifname << " gets " << Ipv4Address::LOOPBACK_ADDRESS.str() << "/" << Ipv4Address::LOOPBACK_NETMASK.str() << std::endl;

            ipv4Data->setIPAddress(Ipv4Address::LOOPBACK_ADDRESS);
            ipv4Data->setNetmask(Ipv4Address::LOOPBACK_NETMASK);
        }
        else{
            if (std::string("wlan0").compare(ifname) == 0 || std::string("samacNic").compare(ifname) == 0) {
                EV_INFO << "interface " << ifname << " gets " << myAddressWlanNic.str() << "/" << netmaskWlanNic.str() << std::endl;

                ipv4Data->setIPAddress(myAddressWlanNic);
                ipv4Data->setNetmask(netmaskWlanNic);
                ie->setBroadcast(true);

            }
            else if (std::string("satNic0").compare(ifname) == 0) {
                EV_INFO << "interface " << ifname << " gets " << myAddressSatelliteNic.str() << "/" << netmaskSatelliteNic.str() << std::endl;

                ipv4Data->setIPAddress(myAddressSatelliteNic);
                ipv4Data->setNetmask(netmaskSatelliteNic);
                ie->setBroadcast(true);
            }
            else{
                throw cRuntimeError("HostAutoConfigurator does not configure %s", ifname);
            }
            // associate interface with default multicast groups
            ipv4Data->joinMulticastGroup(Ipv4Address::ALL_HOSTS_MCAST);
            ipv4Data->joinMulticastGroup(Ipv4Address::ALL_ROUTERS_MCAST);

            // associate interface with specified multicast groups
            cStringTokenizer interfaceTokenizer(mcastGroups.c_str());
            const char *mcastGroup_s;
            while ((mcastGroup_s = interfaceTokenizer.nextToken()) != nullptr) {
                Ipv4Address mcastGroup(mcastGroup_s);
                ipv4Data->joinMulticastGroup(mcastGroup);
            }
        }
    }
}

} // namespace inet

