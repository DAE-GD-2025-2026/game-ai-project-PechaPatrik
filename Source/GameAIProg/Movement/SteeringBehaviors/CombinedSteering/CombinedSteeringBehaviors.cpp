
#include "CombinedSteeringBehaviors.h"
#include <algorithm>
#include "../SteeringAgent.h"

BlendedSteering::BlendedSteering(const std::vector<WeightedBehavior>& WeightedBehaviors)
	:WeightedBehaviors(WeightedBehaviors)
{};

//****************
//BLENDED STEERING
SteeringOutput BlendedSteering::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput BlendedSteering = {};
	float TotalWeight{ 0.0f };

    for (const auto& WeightedBehavior : WeightedBehaviors)
    {
        if (WeightedBehavior.pBehavior && WeightedBehavior.Weight > 0.0f)
        {
            SteeringOutput BehaviorOutput = WeightedBehavior.pBehavior->CalculateSteering(DeltaT, Agent);

            if (BehaviorOutput.IsValid)
            {
                BlendedSteering.LinearVelocity += BehaviorOutput.LinearVelocity * WeightedBehavior.Weight;
                BlendedSteering.AngularVelocity += BehaviorOutput.AngularVelocity * WeightedBehavior.Weight;
                TotalWeight += WeightedBehavior.Weight;
            }
        }
    }

    // Normalize by total weight
    if (TotalWeight > 0.0f)
    {
		float InvTotal = 1.f / TotalWeight;
		BlendedSteering.LinearVelocity *= InvTotal;
		BlendedSteering.AngularVelocity *= InvTotal;
        BlendedSteering.IsValid = true;
    }

	if (Agent.GetDebugRenderingEnabled())
		DrawDebugDirectionalArrow(
			Agent.GetWorld(),
			Agent.GetActorLocation(),
			Agent.GetActorLocation() + FVector{BlendedSteering.LinearVelocity, 0} * (Agent.GetMaxLinearSpeed() * DeltaT),
			30.f, FColor::Red
			);

	return BlendedSteering;
}

//*****************
//PRIORITY STEERING
SteeringOutput PrioritySteering::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering = {};

	for (ISteeringBehavior* const pBehavior : m_PriorityBehaviors)
	{
		Steering = pBehavior->CalculateSteering(DeltaT, Agent);

		if (Steering.IsValid)
			break;
	}

	//If non of the behavior return a valid output, last behavior is returned
	return Steering;
}