/************************************************************************/
/*                                                                      */
/* This file is part of VDrift.                                         */
/*                                                                      */
/* VDrift is free software: you can redistribute it and/or modify       */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or    */
/* (at your option) any later version.                                  */
/*                                                                      */
/* VDrift is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU General Public License for more details.                         */
/*                                                                      */
/* You should have received a copy of the GNU General Public License    */
/* along with VDrift.  If not, see <http://www.gnu.org/licenses/>.      */
/*                                                                      */
/************************************************************************/

#ifndef _CARWHEEL_H
#define _CARWHEEL_H

#include "driveshaft.h"
#include "macros.h"

class CarWheel
{
public:
	CarWheel() : radius(0.3), width(0.2), mass(20) {}

	btScalar GetRotation() const
	{
		return shaft.angle;
	}

	btScalar GetRPM() const
	{
		return shaft.ang_velocity * btScalar(30 / M_PI);
	}

	btScalar GetAngularVelocity() const
	{
		return shaft.ang_velocity;
	}

	void SetAngularVelocity(btScalar value)
	{
		shaft.ang_velocity = value;
	}

	void SetMass(btScalar value)
	{
		mass = value;
	}

	btScalar GetMass() const
	{
		return mass;
	}

	void SetWidth(float value)
	{
		width = value;
	}

	btScalar GetWidth() const
	{
		return width;
	}

	void SetRadius(float value)
	{
		radius = value;
	}

	btScalar GetRadius() const
	{
		return radius;
	}

	void SetInertia(btScalar value)
	{
		shaft.inertia = value;
		shaft.inv_inertia = 1 / value;
	}

	btScalar GetInertia() const
	{
		return shaft.inertia;
	}

	DriveShaft & GetShaft()
	{
		return shaft;
	}

	void Integrate(btScalar dt)
	{
		shaft.integrate(dt);
	}

	template <class Stream>
	void DebugPrint(Stream & out) const
	{
		out << "---Wheel---" << "\n";
		out << "RPM: " << GetRPM() << "\n";
	}

	template <class Serializer>
	bool Serialize(Serializer & s)
	{
		_SERIALIZE_(s, shaft.ang_velocity);
		_SERIALIZE_(s, shaft.angle);
		return true;
	}

private:
	DriveShaft shaft;
	btScalar radius;
	btScalar width;
	btScalar mass;
};

#endif
