#include "SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"

SteeringOutput Seek::CalculateSteering(float DeltaTime, ASteeringAgent& Agent)
{
	SteeringOutput Output{};

	const FVector2D DirectionToTarget{ Target.Position - Agent.GetPosition() };

	if (!DirectionToTarget.IsZero())
	{
		const FVector2D NormalizedDirection{ DirectionToTarget.GetSafeNormal() };
		Output.LinearVelocity = NormalizedDirection * Agent.GetMaxLinearSpeed();
	}

	return Output;
}

SteeringOutput Flee::CalculateSteering(float DeltaTime, ASteeringAgent& Agent)
{
	SteeringOutput Output{};

	const FVector2D DirectionFromTarget{ Agent.GetPosition() - Target.Position };

	if (!DirectionFromTarget.IsZero())
	{
		const FVector2D NormalizedDirection{ DirectionFromTarget.GetSafeNormal() };
		Output.LinearVelocity = NormalizedDirection * Agent.GetMaxLinearSpeed();
	}

	return Output;
}