#pragma once

#include <Movement/SteeringBehaviors/SteeringHelpers.h>
#include "Kismet/KismetMathLibrary.h"

class ASteeringAgent;

// SteeringBehavior base, all steering behaviors should derive from this.
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	// Override to implement your own behavior
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent & Agent) = 0;

	void SetTarget(const FTargetData& NewTarget) { Target = NewTarget; }
	
	template<class T, std::enable_if_t<std::is_base_of_v<ISteeringBehavior, T>>* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	FTargetData Target;
};

class Seek : public ISteeringBehavior
{
public:
	virtual SteeringOutput CalculateSteering(float DeltaTime, ASteeringAgent& Agent) override;
};

class Flee : public ISteeringBehavior
{
public:
	virtual SteeringOutput CalculateSteering(float DeltaTime, ASteeringAgent& Agent) override;
};

class Arrive : public ISteeringBehavior
{
public:
	virtual SteeringOutput CalculateSteering(float DeltaTime, ASteeringAgent& Agent) override;

	void SetSlowRadius(float InRadius) { SlowRadius = InRadius; }
	void SetTargetRadius(float InRadius) { TargetRadius = InRadius; }

private:
	float SlowRadius{ 500.0f };
	float TargetRadius{ 50.0f };
	float OriginalMaxSpeed{ 0.0f };
};

class Face : public ISteeringBehavior
{
public:
	virtual SteeringOutput CalculateSteering(float DeltaTime, ASteeringAgent& Agent) override;
};

class TargetingBehavior : public ISteeringBehavior
{
protected:
	FVector2D PredictTargetPosition(const ASteeringAgent& Agent, const FTargetData& InTarget) const;
};

class Pursuit : public TargetingBehavior
{
public:
	virtual SteeringOutput CalculateSteering(float DeltaTime, ASteeringAgent& Agent) override;
};

class Evade : public TargetingBehavior
{
public:
	virtual SteeringOutput CalculateSteering(float DeltaTime, ASteeringAgent& Agent) override;
};
