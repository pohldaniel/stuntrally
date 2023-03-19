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

#ifndef _DRIVESHAFT_H
#define _DRIVESHAFT_H

#include "LinearMath/btScalar.h"

struct DriveShaft
{
	btScalar inertia;
	btScalar inv_inertia;
	btScalar ang_velocity;
	btScalar angle;

	DriveShaft() : inertia(1), inv_inertia(1), ang_velocity(0), angle(0) {}

	void applyImpulse(btScalar impulse)
	{
		ang_velocity += inv_inertia * impulse;
	}

	void integrate(btScalar dt)
	{
		angle += ang_velocity * dt;
	}
};

#endif // _DRIVESHAFT_H

