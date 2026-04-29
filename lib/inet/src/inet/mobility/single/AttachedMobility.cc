//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

// The following list shows changes made by Mario Franke
// Copyright (C) 2026 Mario Franke <research@m-franke.net>
// Documentation for space_Veins is at http://sat.car2x.org/
// SPDX-License-Identifier: GPL-2.0-or-later
// 
// 2026-04-21 update lastPosition such that the position
//            of this AttachMobility is correctly displayed
//            in OMNeT++'s GUI using the WATCH(lastPosition)
//            command in inet::MobilityBase

#include "inet/common/geometry/common/Quaternion.h"
#include "inet/mobility/single/AttachedMobility.h"

namespace inet {

Define_Module(AttachedMobility);

void AttachedMobility::initialize(int stage)
{
    MobilityBase::initialize(stage);
    EV_TRACE << "initializing AttachedMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        mobility = getModuleFromPar<IMobility>(par("mobilityModule"), this, true);
        positionOffset.x = par("offsetX");
        positionOffset.y = par("offsetY");
        positionOffset.z = par("offsetZ");
        auto alpha = deg(par("offsetHeading"));
        auto offsetElevation = deg(par("offsetElevation"));
        // NOTE: negation is needed, see IMobility comments on orientation
        auto beta = -offsetElevation;
        auto gamma = deg(par("offsetBank"));
        orientationOffset = Quaternion(EulerAngles(alpha, beta, gamma));
        isZeroOffset = positionOffset == Coord::ZERO;
        check_and_cast<cModule *>(mobility)->subscribe(IMobility::mobilityStateChangedSignal, this);
    }
    else if (stage == INITSTAGE_SINGLE_MOBILITY) {
        // update lastPosition of this AttachedMobility such that the WATCH(lastPosition) commands displays the correct position
        RotationMatrix rotation(mobility->getCurrentAngularPosition().toEulerAngles());
        lastPosition = mobility->getCurrentPosition() + rotation.rotateVector(positionOffset);
    }
}

void AttachedMobility::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    if (IMobility::mobilityStateChangedSignal == signal) {
        // update lastPosition of this AttachedMobility such that the WATCH(lastPosition) commands displays the correct position
        RotationMatrix rotation(mobility->getCurrentAngularPosition().toEulerAngles());
        lastPosition = mobility->getCurrentPosition() + rotation.rotateVector(positionOffset);
        emitMobilityStateChangedSignal();
    }
}

Coord AttachedMobility::getCurrentPosition()
{
    if (isZeroOffset)
        return mobility->getCurrentPosition();
    else {
        RotationMatrix rotation(mobility->getCurrentAngularPosition().toEulerAngles());
        return mobility->getCurrentPosition() + rotation.rotateVector(positionOffset);
    }
}

Coord AttachedMobility::getCurrentVelocity()
{
    if (isZeroOffset)
        return mobility->getCurrentVelocity();
    else {
        RotationMatrix rotation(mobility->getCurrentAngularPosition().toEulerAngles());
        Coord rotatedOffset = rotation.rotateVector(positionOffset);
        Quaternion quaternion(mobility->getCurrentAngularVelocity());
        Coord rotationAxis;
        double rotationAngle;
        quaternion.getRotationAxisAndAngle(rotationAxis, rotationAngle);
        auto additionalVelocity = rotationAngle == 0 ? Coord::ZERO : rotationAxis % rotatedOffset * rotationAngle;
        return mobility->getCurrentVelocity() + additionalVelocity;
    }
}

Coord AttachedMobility::getCurrentAcceleration()
{
    if (isZeroOffset)
        return mobility->getCurrentAcceleration();
    else {
        // TODO:
        return Coord::NIL;
    }
}

Quaternion AttachedMobility::getCurrentAngularPosition()
{
    auto angularPosition = mobility->getCurrentAngularPosition();
    angularPosition *= Quaternion(orientationOffset);
    return angularPosition;
}

Quaternion AttachedMobility::getCurrentAngularVelocity()
{
    return mobility->getCurrentAngularVelocity();
}

Quaternion AttachedMobility::getCurrentAngularAcceleration()
{
    return mobility->getCurrentAngularAcceleration();
}

} // namespace inet

