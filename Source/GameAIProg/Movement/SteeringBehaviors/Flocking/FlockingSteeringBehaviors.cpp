#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	if (pFlock->GetNrOfNeighbors() == 0)
		return SteeringOutput{};

	FTargetData SeekTarget{};
	SeekTarget.Position = pFlock->GetAverageNeighborPos();
	SetTarget(SeekTarget);
	return Seek::CalculateSteering(deltaT, pAgent);
}

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput Output{};

	if (pFlock->GetNrOfNeighbors() == 0)
		return Output;

	const TArray<ASteeringAgent*>& Neighbors = pFlock->GetNeighbors();
	const int NrOfNeighbors = pFlock->GetNrOfNeighbors();

	FVector2D SeparationForce = FVector2D::ZeroVector;

	for (int i = 0; i < NrOfNeighbors; ++i)
	{
		if (!IsValid(Neighbors[i])) continue;

		const FVector2D ToAgent = pAgent.GetPosition() - Neighbors[i]->GetPosition();
		const float Distance = ToAgent.Size();

		if (Distance > 0.f)
		{
			// y = 1/x, so closer = stronger push
			SeparationForce += ToAgent.GetSafeNormal() * (1.f / Distance);
		}
	}

	if (!SeparationForce.IsZero())
		Output.LinearVelocity = SeparationForce.GetSafeNormal() * pAgent.GetMaxLinearSpeed();

	return Output;
}

//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput Output{};

	if (pFlock->GetNrOfNeighbors() == 0)
		return Output;

	Output.LinearVelocity = pFlock->GetAverageNeighborVelocity();
	return Output;
}
