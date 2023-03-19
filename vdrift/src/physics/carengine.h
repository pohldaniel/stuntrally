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

#ifndef _CARENGINE_H
#define _CARENGINE_H

#include "driveshaft.h"
#include "LinearMath/btVector3.h"
#include "spline.h"
#include "macros.h"

#include <iosfwd>
#include <vector>

class PTree;

struct CarEngineInfo
{
	btScalar displacement; ///< used to calculate engine friction
	btScalar maxpower; ///< max power output
	btScalar redline; ///< the redline in RPMs (used only for the redline graphics)
	btScalar rpm_limit; ///< peak engine RPMs after which limiting occurs
	btScalar idle_throttle; ///< idle throttle percentage; this is calculated algorithmically
	btScalar idle_throttle_slope; ///< idle throttle rpm slope
	btScalar start_rpm; ///< initial condition RPM
	btScalar stall_rpm; ///< RPM at which the engine dies
	btScalar fuel_rate; ///< fuel rate kg/Ws based on fuel heating value(4E7) and engine efficiency(0.35)
	btScalar friction[3]; ///< friction torque coefficients
	Spline<btScalar> torque_curve;
	btVector3 position;
	btScalar inertia;
	btScalar mass;
	btScalar nos_mass; ///< amount of nitrous oxide in kg
	btScalar nos_boost; ///< max nitrous oxide power boost in Watt
	btScalar nos_fuel_ratio; ///< nos to fuel ratio(5)

	/// default constructor makes an S2000-like car
	/// SetTorqueCurve() has to be called explicitly to initialise carengineinfo
	CarEngineInfo();

	/// load engine from config file
	bool Load(const PTree & cfg, std::ostream & error_output);

	btScalar GetMaxTorque() const;

	btScalar GetTorque(btScalar throttle, btScalar rpm) const;

	btScalar GetFrictionTorque(btScalar throttle, btScalar rpm) const;
};

class CarEngine
{
public:
	CarEngine();

	void Init(const CarEngineInfo & info);

	btScalar GetDisplacement() const
	{
		return info.displacement;
	}

	btScalar GetMaxPower() const
	{
		return info.maxpower;
	}

	btScalar GetMaxTorque() const
	{
		return info.GetMaxTorque();
	}

	btScalar GetRPMLimit() const
	{
		return info.rpm_limit;
	}

	btScalar GetRedline() const
	{
		return info.redline;
	}

	btScalar GetIdleThrottle() const
	{
		return info.idle_throttle;
	}

	btScalar GetStartRPM() const
	{
		return info.start_rpm;
	}

	btScalar GetStallRPM() const
	{
		return info.stall_rpm;
	}

	const btVector3 & GetPosition() const
	{
		return info.position;
	}

	btScalar GetInertia() const
	{
		return info.inertia;
	}

	btScalar GetMass() const
	{
		return info.mass;
	}

	btScalar GetRPM() const
	{
		return shaft.ang_velocity * btScalar(30 / M_PI);
	}

	btScalar GetThrottle() const
	{
		return throttle_position;
	}

	btScalar GetAngularVelocity() const
	{
		return shaft.ang_velocity ;
	}

	///return the sum of all torques acting on the engine (except clutch forces)
	btScalar GetTorque() const
	{
		return combustion_torque + friction_torque;
	}

	///fuel consumtion rate kg/s per simulation step
	btScalar FuelRate() const
	{
		return info.fuel_rate * combustion_torque * shaft.ang_velocity;
	}

	///returns true if the engine is combusting fuel
	bool GetCombustion() const
	{
		return !stalled;
	}

	///return remaining nos fraction
	btScalar GetNosAmount() const
	{
		return nos_mass / info.nos_mass;
	}

	void StartEngine()
	{
		shaft.ang_velocity = info.start_rpm * btScalar(M_PI / 30.0);
	}

	///set the throttle position where 0.0 is no throttle and 1.0 is full throttle
	void SetThrottle(btScalar value)
	{
		throttle_position = value;
	}

	/// nitrous injection boost factor 0.0 - 1.0
	void SetNosBoost(btScalar value)
	{
		nos_boost_factor = value;
	}

	void SetOutOfGas(bool value)
	{
		out_of_gas = value;
	}

	DriveShaft & GetShaft()
	{
		return shaft;
	}

	void Update(btScalar dt);

	template <class Stream>
	void DebugPrint(Stream & out) const
	{
		out << "---Engine---" << "\n";
		out << "Throttle position: " << throttle_position << "\n";
		out << "Combustion torque: " << combustion_torque << "\n";
		out << "Friction torque: " << friction_torque << "\n";
		out << "Total torque: " << GetTorque() << "\n";
		out << "RPM: " << GetRPM() << "\n";
		out << "Rev limit exceeded: " << rev_limit_exceeded << "\n";
		out << "Running: " << !stalled << "\n";
	}

	template <class Serializer>
	bool Serialize(Serializer & s)
	{
		_SERIALIZE_(s, shaft.ang_velocity);
		_SERIALIZE_(s, throttle_position);
		_SERIALIZE_(s, out_of_gas);
		_SERIALIZE_(s, rev_limit_exceeded);
		return true;
	}

private:
	CarEngineInfo info;

	DriveShaft shaft;
	btScalar combustion_torque;
	btScalar friction_torque;

	btScalar throttle_position;
	btScalar nos_boost_factor;
	btScalar nos_mass;
	bool rev_limit_exceeded;
	bool out_of_gas;
	bool stalled;
};

#endif
