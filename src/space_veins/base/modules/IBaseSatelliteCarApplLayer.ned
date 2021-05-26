//
// Copyright (C) 2007 Technische Universitaet Berlin (TUB), Germany, Telecommunication Networks Group
// Copyright (C) 2007 Technische Universiteit Delft (TUD), Netherlands
// Copyright (C) 2007 Universitaet Paderborn (UPB), Germany
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

package space_veins.base.modules;

// Interface for satellite car application layer modules
moduleinterface IBaseSatelliteCarApplLayer
{
    parameters:
        int headerLength @unit("bit"); // length of the application message header (in bits)
    gates:
        input lowerLayerIn; // from c2x network layer
        output lowerLayerOut; // to c2x network layer
        input lowerControlIn; // control from c2x network layer
        output lowerControlOut; // control to c2x network layer

        input satLowerLayerIn; // from satellite network layer
        output satLowerLayerOut; // to satellite network layer
        input satLowerControlIn; // control from satellite network layer
        output satLowerControlOut; // control to satellite network layer
}

