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

SteeringOutput Arrive::CalculateSteering(float DeltaTime, ASteeringAgent& Agent)
{
	SteeringOutput Output{};

	if (OriginalMaxSpeed == 0.0f)
	{
		OriginalMaxSpeed = Agent.GetMaxLinearSpeed();
	}

	const FVector2D DirectionToTarget{ Target.Position - Agent.GetPosition() };
	const float DistanceToTarget = DirectionToTarget.Size();

	if (DistanceToTarget < TargetRadius)
	{
		Output.LinearVelocity = FVector2D::ZeroVector;
		Agent.SetMaxLinearSpeed(OriginalMaxSpeed);
		return Output;
	}

	float ActualSpeed{ 0.0f };

	if (DistanceToTarget > SlowRadius)
	{
		ActualSpeed = OriginalMaxSpeed;
	}
	else
	{
		ActualSpeed = OriginalMaxSpeed * (DistanceToTarget / SlowRadius);
	}

	Agent.SetMaxLinearSpeed(ActualSpeed);

	const FVector2D NormalizedDirection{ DirectionToTarget.GetSafeNormal() };
	Output.LinearVelocity = NormalizedDirection * ActualSpeed;

	return Output;
}