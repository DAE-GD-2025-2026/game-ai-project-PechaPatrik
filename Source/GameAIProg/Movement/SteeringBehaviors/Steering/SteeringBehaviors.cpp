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

	// Debug rendering
	if (Agent.GetDebugRenderingEnabled())
	{
		UWorld* pWorld = Agent.GetWorld();

		// Draw line from agent to target
		DrawDebugLine(pWorld,
			FVector{ Agent.GetPosition(), 0.0f },
			FVector{ Target.Position, 0.0f },
			FColor::Green);

		// Draw target point
		DrawDebugPoint(pWorld,
			FVector{ Target.Position, 0.0f },
			10.0f,
			FColor::Green);
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

	// Debug rendering
	if (Agent.GetDebugRenderingEnabled())
	{
		UWorld* pWorld = Agent.GetWorld();

		// Draw line from target to agent
		DrawDebugLine(pWorld,
			FVector{ Target.Position, 0.0f },
			FVector{ Agent.GetPosition(), 0.0f },
			FColor::Red);

		// Draw target point
		DrawDebugPoint(pWorld,
			FVector{ Target.Position, 0.0f },
			10.0f,
			FColor::Red);
	}

	return Output;
}

SteeringOutput Arrive::CalculateSteering(float DeltaTime, ASteeringAgent& Agent)
{
	SteeringOutput Output{};

	const float AgentMaxSpeed = Agent.GetMaxLinearSpeed();

	const FVector2D DirectionToTarget{ Target.Position - Agent.GetPosition() };
	const float DistanceToTarget = DirectionToTarget.Size();

	if (Agent.GetDebugRenderingEnabled())
	{
		UWorld* pWorld = Agent.GetWorld();
		const FVector Target3D{ Target.Position, 0.0f };

		// Draw outer circle
		DrawDebugCircle(pWorld,
			Target3D,
			SlowRadius,
			16,
			FColor::Blue,
			false,
			-1.f,
			0,
			0.f,
			FVector(1, 0, 0),
			FVector(0, 1, 0));

		// Draw inner circle
		DrawDebugCircle(pWorld,
			Target3D,
			TargetRadius,
			16,
			FColor::Red,
			false, 
			-1.f, 
			0,
			0.f,
			FVector(1, 0, 0),
			FVector(0, 1, 0));

		// Draw line from agent to target with color based on speed
		FColor LineColor = FColor::Red;
		if (DistanceToTarget < TargetRadius)
		{
			LineColor = FColor::Green; // Arrived
		}
		else if (DistanceToTarget < SlowRadius)
		{
			LineColor = FColor::Yellow; // Slowing down
		}

		DrawDebugLine(pWorld,
			FVector{ Agent.GetPosition(), 0.0f },
			Target3D,
			LineColor);
	}

	if (DistanceToTarget < TargetRadius)
	{
		Output.LinearVelocity = FVector2D::ZeroVector;
		return Output;
	}

	float DesiredSpeed = AgentMaxSpeed;

	if (DistanceToTarget < SlowRadius)
	{
		DesiredSpeed = AgentMaxSpeed * (DistanceToTarget / SlowRadius);
	}

	if (!DirectionToTarget.IsZero())
	{
		const FVector2D NormalizedDirection{ DirectionToTarget.GetSafeNormal() };
		const FVector2D DesiredVelocity = NormalizedDirection * DesiredSpeed;
		Output.LinearVelocity = DesiredVelocity - Agent.GetLinearVelocity();
	}

	return Output;
}

SteeringOutput Face::CalculateSteering(float DeltaTime, ASteeringAgent& Agent)
{
	SteeringOutput Output{};
	Output.LinearVelocity = {};

	const FVector2D DirectionToTarget{ Target.Position - Agent.GetPosition() };

	if (!DirectionToTarget.IsZero())
	{
		const float DesiredOrientation = FMath::Atan2(DirectionToTarget.Y, DirectionToTarget.X);
		const float CurrentOrientation{ FMath::DegreesToRadians(Agent.GetRotation()) };

		float AngleDifference{ DesiredOrientation - CurrentOrientation };

		AngleDifference = FMath::Fmod(AngleDifference + PI, 2.0f * PI);
		if (AngleDifference < 0.0f)
		{
			AngleDifference += 2.0f * PI;
		}
		AngleDifference -= PI;

		const float MaxAngularSpeed{ FMath::DegreesToRadians(Agent.GetMaxAngularSpeed()) };
		Output.AngularVelocity = FMath::Clamp(AngleDifference / DeltaTime, -MaxAngularSpeed, MaxAngularSpeed);
	}

	if (Agent.GetDebugRenderingEnabled())
	{
		UWorld* pWorld = Agent.GetWorld();

		// Draw current forward direction (blue)
		const float CurrentRotationRad{ FMath::DegreesToRadians(Agent.GetRotation()) };
		const FVector2D CurrentForward{ FMath::Cos(CurrentRotationRad), FMath::Sin(CurrentRotationRad) };
		const FVector2D CurrentForwardEnd{ Agent.GetPosition() + (CurrentForward * 100.0f) };

		DrawDebugLine(pWorld,
			FVector{ Agent.GetPosition(), 0.0f },
			FVector{ CurrentForwardEnd, 0.0f },
			FColor::Blue);

		// Draw desired forward direction (green)
		if (!DirectionToTarget.IsZero())
		{
			const FVector2D DesiredForward{ DirectionToTarget.GetSafeNormal() };
			const FVector2D DesiredForwardEnd{ Agent.GetPosition() + (DesiredForward * 100.0f) };

			DrawDebugLine(pWorld,
				FVector{ Agent.GetPosition(), 0.0f },
				FVector{ DesiredForwardEnd, 0.0f },
				FColor::Green);
		}

		// Draw target point
		DrawDebugPoint(pWorld,
			FVector{ Target.Position, 0.0f },
			10.0f,
			FColor::Green);
	}

	return Output;
}

FVector2D TargetingBehavior::PredictTargetPosition(const ASteeringAgent& Agent, const FTargetData& InTarget) const
{
	const FVector2D ToTarget{ InTarget.Position - Agent.GetPosition() };
	const float Distance = ToTarget.Size();
	const float AgentSpeed = Agent.GetLinearVelocity().Size();

	if (AgentSpeed < 0.01f)
	{
		return InTarget.Position;
	}

	const float TimeToReach{ Distance / AgentSpeed };
	return InTarget.Position + (InTarget.LinearVelocity * TimeToReach);
}

SteeringOutput Pursuit::CalculateSteering(float DeltaTime, ASteeringAgent& Agent)
{
	SteeringOutput Output{};

	const FVector2D PredictedPosition{ PredictTargetPosition(Agent, Target) };
	const FVector2D DirectionToPrediction{ PredictedPosition - Agent.GetPosition() };

	if (!DirectionToPrediction.IsZero())
	{
		const FVector2D NormalizedDirection{ DirectionToPrediction.GetSafeNormal() };
		Output.LinearVelocity = NormalizedDirection * Agent.GetMaxLinearSpeed();
	}

	// Debug rendering
	if (Agent.GetDebugRenderingEnabled())
	{
		// Draw line to current target
		DrawDebugLine(Agent.GetWorld(),
			FVector{ Agent.GetPosition(), 0.0f },
			FVector{ Target.Position, 0.0f },
			FColor::Red);

		// Draw line to predicted position
		DrawDebugLine(Agent.GetWorld(),
			FVector{ Agent.GetPosition(), 0.0f },
			FVector{ PredictedPosition, 0.0f },
			FColor::Green);

		// Draw prediction point
		DrawDebugPoint(Agent.GetWorld(),
			FVector{ PredictedPosition, 0.0f },
			10.0f,
			FColor::Green);
	}

	return Output;
}

SteeringOutput Evade::CalculateSteering(float DeltaTime, ASteeringAgent& Agent)
{
	SteeringOutput Output{};
	const FVector2D PredictedPosition{ PredictTargetPosition(Agent, Target) };
	const FVector2D DirectionFromPrediction{ Agent.GetPosition() - PredictedPosition };

	if (!DirectionFromPrediction.IsZero())
	{
		const FVector2D NormalizedDirection{ DirectionFromPrediction.GetSafeNormal() };
		Output.LinearVelocity = NormalizedDirection * Agent.GetMaxLinearSpeed();
	}

	// Debug rendering
	if (Agent.GetDebugRenderingEnabled())
	{
		// Draw line from predicted position to agent
		DrawDebugLine(Agent.GetWorld(),
			FVector{ PredictedPosition, 0.0f },
			FVector{ Agent.GetPosition(), 0.0f },
			FColor::Red);

		// Draw prediction point
		DrawDebugPoint(Agent.GetWorld(),
			FVector{ PredictedPosition, 0.0f },
			10.0f,
			FColor::Red);
	}

	return Output;
}

SteeringOutput Wander::CalculateSteering(float DeltaTime, ASteeringAgent& Agent)
{
	const float RandomAngle{ FMath::RandRange(-MaxAngleChange, MaxAngleChange) };
	WanderAngle += RandomAngle;
	WanderAngle = FMath::Fmod(WanderAngle, 2.0f * PI);
    if (WanderAngle < 0.0f) WanderAngle += 2.0f * PI;

	const float AgentRotationRadians{ FMath::DegreesToRadians(Agent.GetRotation()) };
	const FVector2D AgentForward{ FMath::Cos(AgentRotationRadians), FMath::Sin(AgentRotationRadians) };
	const FVector2D CircleCenter{ Agent.GetPosition() + (AgentForward * WanderOffset) };
	const FVector2D CirclePoint{ FMath::Cos(WanderAngle), FMath::Sin(WanderAngle) };
	const FVector2D ScaledCirclePoint{ CirclePoint * WanderRadius };

	// Set target to circle point and seek to it
	Target.Position = CircleCenter + ScaledCirclePoint;

	// Debug rendering
	if (Agent.GetDebugRenderingEnabled())
	{
		UWorld* pWorld = Agent.GetWorld();

		// Draw the wander circle
		DrawDebugCircle(pWorld,
			FVector{ CircleCenter, 0.0f },
			WanderRadius,
			32,
			FColor::Blue,
			false,
			-1.f,
			0,
			0.f,
			FVector(1, 0, 0),
			FVector(0, 1, 0));

		// Draw line from agent to circle center
		DrawDebugLine(pWorld,
			FVector{ Agent.GetPosition(), 0.0f },
			FVector{ CircleCenter, 0.0f },
			FColor::Magenta);

		// Draw line from agent to wander target
		DrawDebugLine(pWorld,
			FVector{ Agent.GetPosition(), 0.0f },
			FVector{ Target.Position, 0.0f },
			FColor::Green);

		// Draw the target point
		DrawDebugPoint(pWorld,
			FVector{ Target.Position, 0.0f },
			10.0f,
			FColor::Green);
	}

	return Seek::CalculateSteering(DeltaTime, Agent);
}