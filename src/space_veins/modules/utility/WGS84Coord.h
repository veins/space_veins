//
// Copyright (C) 2004 Telecommunication Networks Group (TKN) at Technische Universitaet Berlin, Germany.
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

// author:      Christian Frank
// part of:     framework implementation developed by tkn

// author:      Mario Franke, adapted the class for WGS84Coord


#pragma once

#include "space_veins/space_veins.h"

namespace space_veins {
class WGS84Coord;
}

#include "veins/base/utils/FWMath.h"

namespace space_veins {

/**
 * @brief Class for storing 3D coordinates.
 *
 * Some comparison and basic arithmetic operators are implemented.
 *
 * @ingroup utils
 * @author Christian Frank
 */
class SPACE_VEINS_API WGS84Coord : public cObject {
public:
    /** @brief Constant with all values set to 0. */
    static const WGS84Coord ZERO;

public:
    /** @name lat, lon and alt coordinate of the position. */
    /*@{*/
    double lat;
    double lon;
    double alt;
    /*@}*/

private:
    void copy(const WGS84Coord& other)
    {
        lat = other.lat;
        lon = other.lon;
        alt = other.alt;
    }

public:
    /** @brief Default constructor. */
    WGS84Coord()
        : lat(0.0)
        , lon(0.0)
        , alt(0.0)
    {
    }

    /** @brief Initializes a coordinate. */
    WGS84Coord(double lat, double lon, double alt = 0.0)
        : lat(lat)
        , lon(lon)
        , alt(alt)
    {
    }

    /** @brief Initializes coordinate from other coordinate. */
    WGS84Coord(const WGS84Coord& other)
        : cObject(other)
    {
        copy(other);
    }

    /** @brief Returns a string with the value of the coordinate. */
#if OMNETPP_VERSION < 0x600
    std::string info() const override;
#else
    std::string info() const;
    std::string str() const override
    {
        return info();
    }
#endif

    /** @brief Subtracts two coordinate vectors. */
    friend WGS84Coord operator-(const WGS84Coord& a, const WGS84Coord& b)
    {
        WGS84Coord tmp(a);
        tmp -= b;
        return tmp;
    }

    /**
     * @brief Assigns coordinate vector 'other' to this.
     *
     * This operator can change the dimension of the coordinate.
     */
    WGS84Coord& operator=(const WGS84Coord& other)
    {
        if (this == &other) return *this;
        cObject::operator=(other);
        copy(other);
        return *this;
    }

    /**
     * @brief Subtracts coordinate vector 'a' from this.
     */
    WGS84Coord& operator-=(const WGS84Coord& a)
    {
        lat -= a.lat;
        lon -= a.lon;
        alt -= a.alt;
        return *this;
    }

    /**
     * @brief Tests whether two coordinate vectors are equal.
     *
     * Because coordinates are floating point values, this is done using an epsilon comparison.
     * @see veins::math::almost_equal
     *
     */
    friend bool operator==(const WGS84Coord& a, const WGS84Coord& b)
    {
        // FIXME: this implementation is not transitive
        return veins::math::almost_equal(a.lat, b.lat) && veins::math::almost_equal(a.lon, b.lon) && veins::math::almost_equal(a.alt, b.alt);
    }

    /**
     * @brief Tests whether two coordinate vectors are not equal.
     *
     * Negation of the operator==.
     */
    friend bool operator!=(const WGS84Coord& a, const WGS84Coord& b)
    {
        return !(a == b);
    }
};

inline std::ostream& operator<<(std::ostream& os, const WGS84Coord& coord)
{
    return os << "(lat(deg) " << coord.lat << ", lon(deg) " << coord.lon << ", alt(m) " << coord.alt << ")";
}

inline std::string WGS84Coord::info() const
{
    std::stringstream os;
    os << *this;
    return os.str();
}

} // namespace space_veins
