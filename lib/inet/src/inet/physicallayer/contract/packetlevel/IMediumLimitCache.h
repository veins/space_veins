//
// Copyright (C) 2013 OpenSim Ltd.
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
// ---------------------------------------------------------------------------------
//
// This file is modified compared to the original version of INET such that it works
// with space_Veins <https://github.com/veins/space_veins>. Please use a tool like
// 'meld' in order to see the differences.
// Author of the modifications is Mario Franke <research@m-franke.net>.
//

#ifndef __INET_IMEDIUMLIMITCACHE_H
#define __INET_IMEDIUMLIMITCACHE_H

#include "inet/physicallayer/contract/packetlevel/IPrintableObject.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"

namespace inet {

namespace physicallayer {

/**
 * This interface is used to cache various medium limits among the radios.
 */
class INET_API IMediumLimitCache : public IPrintableObject
{
  public:
    /**
     * Notifies the cache when a radio is added to the medium.
     */
    virtual void addRadio(const IRadio *radio) = 0;

    /**
     * Notifies the cache when a radio is removed from the medium.
     */
    virtual void removeRadio(const IRadio *radio) = 0;

    /**
     * Returns the minimum possible position among the radios, the coordinate
     * values are in the range (-infinity, +infinity) or NaN if unspecified.
     */
    virtual Coord getMinConstraintArea() const = 0;

    /**
     * Returns the maximum possible position among the radios, the coordinate
     * values are in the range (-infinity, +infinity) or NaN if unspecified.
     */
    virtual Coord getMaxConstraintArea() const = 0;

    /**
     * Returns the maximum possible speed among the radios, the value is
     * in the range [0, +infinity) or NaN if unspecified.
     */
    virtual mps getMaxSpeed() const = 0;

    /**
     * Returns the maximum possible transmission power among the radios, the
     * value is in the range [0, +infinity) or NaN if unspecified.
     */
    virtual W getMaxTransmissionPower() const = 0;

    /**
     * Returns the minimum possible interference power among the radios, the
     * value is in the range [0, +infinity) or NaN if unspecified.
     */
    virtual W getMinInterferencePower() const = 0;

    /**
     * Returns the minimum possible reception power among the radios, the
     * value is in the range [0, +infinity) or NaN if unspecified.
     */
    virtual W getMinReceptionPower() const = 0;

    /**
     * Returns the maximum possible antenna gain among the radios, the value
     * is in the range [1, +infinity) or NaN if unspecified.
     */
    virtual double getMaxAntennaGain() const = 0;

    /**
     * Returns the minimum required signal interference time among the radios,
     * the value is in the range [0, +infinity) or NaN if unspecified.
     */
    virtual const simtime_t& getMinInterferenceTime() const = 0;

    /**
     * Returns the maximum possible transmission durations among the radios,
     * the value is in the range [0, +infinity) or NaN if unspecified.
     */
    virtual const simtime_t& getMaxTransmissionDuration() const = 0;

    /**
     * Returns the maximum possible communication range among the radios, the
     * value is in the range [0, +infinity) or NaN if unspecified.
     */
    virtual m getMaxCommunicationRange() const = 0;

    /**
     * Returns the maximum possible interference range among the radios, the
     * value is in the range [0, +infinity) or NaN if unspecified.
     */
    virtual m getMaxInterferenceRange() const = 0;

    /**
     * Returns the maximum possible communication range for a given radio, the
     * value is in the range [0, +infinity) or NaN if unspecified.
     */
    virtual m getMaxCommunicationRange(const IRadio *radio) const = 0;

    /**
     * Returns the maximum possible interference range for a given radio, the
     * value is in the range [0, +infinity) or NaN if unspecified.
     */
    virtual m getMaxInterferenceRange(const IRadio *radio) const = 0;

    /**
     * Returns the minimum required elevation angle such that a vehicle and
     * a satellite can communicate, NaN if unspecified.
     */
    virtual deg getMinElevationAngleV2S() const = 0;
    /**
     * Returns the minimum required elevation angle such that a drone and
     * a satellite can communicate, NaN if unspecified.
     */
    virtual deg getMinElevationAngleD2S() const = 0;
    /**
    * Returns whether satellite to satellite communication is disabled.
    */
    virtual bool getDisableS2SCommunication() const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IMEDIUMLIMITCACHE_H

