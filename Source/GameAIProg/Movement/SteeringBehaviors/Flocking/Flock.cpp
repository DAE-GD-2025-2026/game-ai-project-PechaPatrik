#include "Flock.h"
#include "FlockingSteeringBehaviors.h"
#include "Shared/ImGuiHelpers.h"


Flock::Flock(
	UWorld* pWorld,
	TSubclassOf<ASteeringAgent> AgentClass,
	int FlockSize,
	float WorldSize,
	ASteeringAgent* const pAgentToEvade,
	bool bTrimWorld)
	: pWorld{ pWorld }
	, FlockSize{ FlockSize }
	, WorldSize{ WorldSize }
	, pAgentToEvade{ pAgentToEvade }
	, bTrimWorld{ bTrimWorld }
{
	pSeparationBehavior = std::make_unique<Separation>(this);
	pCohesionBehavior = std::make_unique<Cohesion>(this);
	pVelMatchBehavior = std::make_unique<VelocityMatch>(this);
	pSeekBehavior = std::make_unique<Seek>();
	pWanderBehavior = std::make_unique<Wander>();
	pEvadeBehavior = std::make_unique<Evade>();
	pEvadeBehavior->SetEvadeRadius(400.f);

	// BlendedSteering combines all flocking behaviors
	pBlendedSteering = std::make_unique<BlendedSteering>(
		std::vector<BlendedSteering::WeightedBehavior>{
			{ pSeparationBehavior.get(), 0.6f },
			{ pCohesionBehavior.get(), 0.3f },
			{ pVelMatchBehavior.get(), 0.3f },
			{ pSeekBehavior.get(), 0.4f },
			{ pWanderBehavior.get(), 0.2f },
	}
	);

	// PrioritySteering: evade the target agent first, flock otherwise
	pPrioritySteering = std::make_unique<PrioritySteering>(
		std::vector<ISteeringBehavior*>{
		pEvadeBehavior.get(),
			pBlendedSteering.get()
	}
	);

	Neighbors.SetNum(FlockSize);
	Agents.SetNum(FlockSize);

	// Spawn agents scattered randomly in the world
	const float HalfWorld = WorldSize * 0.5f;
	for (int i = 0; i < FlockSize; ++i)
	{
		FVector SpawnPos{
			FMath::RandRange(-HalfWorld, HalfWorld),
			FMath::RandRange(-HalfWorld, HalfWorld),
			90.f
		};

		ASteeringAgent* pAgent = pWorld->SpawnActor<ASteeringAgent>(
			AgentClass, SpawnPos, FRotator::ZeroRotator);

		if (pAgent)
		{
			pAgent->SetSteeringBehavior(pPrioritySteering.get());
			Agents[i] = pAgent;
		}
	}
}

Flock::~Flock()
{
	for (ASteeringAgent* pAgent : Agents)
	{
		if (IsValid(pAgent))
			pAgent->Destroy();
	}
	Agents.Empty();
}

void Flock::Tick(float DeltaTime)
{
	if (pAgentToEvade && IsValid(pAgentToEvade))
	{
		FTargetData EvadeTarget{};
		EvadeTarget.Position = pAgentToEvade->GetPosition();
		EvadeTarget.LinearVelocity = pAgentToEvade->GetLinearVelocity();
		EvadeTarget.Orientation = pAgentToEvade->GetRotation();
		pEvadeBehavior->SetTarget(EvadeTarget);
	}

	for (ASteeringAgent* pAgent : Agents)
	{
		if (!IsValid(pAgent)) continue;

		RegisterNeighbors(pAgent);
		pAgent->Tick(DeltaTime);

		if (bTrimWorld)
		{
			FVector2D Pos = pAgent->GetPosition();

			if (Pos.X > WorldSize)  Pos.X = -WorldSize;
			if (Pos.X < -WorldSize) Pos.X = WorldSize;
			if (Pos.Y > WorldSize)  Pos.Y = -WorldSize;
			if (Pos.Y < -WorldSize) Pos.Y = WorldSize;

			pAgent->SetActorLocation(FVector{ Pos, 90.f });
		}
	}
}

void Flock::RenderDebug()
{
	for (ASteeringAgent* pAgent : Agents)
	{
		if (IsValid(pAgent))
			pAgent->SetDebugRenderingEnabled(DebugRenderSteering);
	}

	if (DebugRenderNeighborhood)
		RenderNeighborhood();
}

void Flock::ImGuiRender(ImVec2 const& WindowPos, ImVec2 const& WindowSize)
{
#ifdef PLATFORM_WINDOWS
#pragma region UI
	//UI
	{
		//Setup
		bool bWindowActive = true;
		ImGui::SetNextWindowPos(WindowPos);
		ImGui::SetNextWindowSize(WindowSize);
		ImGui::Begin("Gameplay Programming", &bWindowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("LMB: place target");
		ImGui::Text("WASD: move cam.");
		ImGui::Text("Scrollwheel: zoom cam.");
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text("Flocking");
		ImGui::Spacing();

		ImGui::Checkbox("Debug Steering", &DebugRenderSteering);
		ImGui::Checkbox("Debug Neighborhood", &DebugRenderNeighborhood);
		ImGui::Spacing();

		ImGui::Text("Behavior Weights");
		ImGui::Spacing();

		ImGuiHelpers::ImGuiSliderFloatWithSetter("Separation",
			pBlendedSteering->GetWeightedBehaviorsRef()[0].Weight, 0.f, 1.f,
			[this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[0].Weight = InVal; }, "%.2f");

		ImGuiHelpers::ImGuiSliderFloatWithSetter("Cohesion",
			pBlendedSteering->GetWeightedBehaviorsRef()[1].Weight, 0.f, 1.f,
			[this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[1].Weight = InVal; }, "%.2f");

		ImGuiHelpers::ImGuiSliderFloatWithSetter("Velocity match",
			pBlendedSteering->GetWeightedBehaviorsRef()[2].Weight, 0.f, 1.f,
			[this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[2].Weight = InVal; }, "%.2f");

		ImGuiHelpers::ImGuiSliderFloatWithSetter("Seek",
			pBlendedSteering->GetWeightedBehaviorsRef()[3].Weight, 0.f, 1.f,
			[this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[3].Weight = InVal; }, "%.2f");

		ImGuiHelpers::ImGuiSliderFloatWithSetter("Wander",
			pBlendedSteering->GetWeightedBehaviorsRef()[4].Weight, 0.f, 1.f,
			[this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[4].Weight = InVal; }, "%.2f");

		ImGui::End();
	}
#pragma endregion
#endif
}

void Flock::RenderNeighborhood()
{
	if (Agents.Num() == 0 || !IsValid(Agents[0])) return;

	ASteeringAgent* pFirst = Agents[0];

	RegisterNeighbors(pFirst);

	DrawDebugCircle(pWorld,
		FVector{ pFirst->GetPosition(), 0.f },
		NeighborhoodRadius, 24, FColor::Green,
		false, -1.f, 0, 0.f,
		FVector(1, 0, 0), FVector(0, 1, 0));

	for (int i = 0; i < NrOfNeighbors; ++i)
	{
		if (!IsValid(Neighbors[i])) continue;
		DrawDebugPoint(pWorld,
			FVector{ Neighbors[i]->GetPosition(), 0.f },
			20.f, FColor::Yellow);
	}
}

#ifndef GAMEAI_USE_SPACE_PARTITIONING
void Flock::RegisterNeighbors(ASteeringAgent* const pAgent)
{
	NrOfNeighbors = 0;

	for (ASteeringAgent* pOther : Agents)
	{
		if (!IsValid(pOther) || pOther == pAgent) continue;

		const float Distance = FVector2D::Distance(
			pAgent->GetPosition(), pOther->GetPosition());

		if (Distance < NeighborhoodRadius)
		{
			Neighbors[NrOfNeighbors] = pOther;
			++NrOfNeighbors;
		}
	}
}
#endif

FVector2D Flock::GetAverageNeighborPos() const
{
	if (NrOfNeighbors == 0)
		return FVector2D::ZeroVector;

	FVector2D Sum = FVector2D::ZeroVector;
	for (int i = 0; i < NrOfNeighbors; ++i)
	{
		if (IsValid(Neighbors[i]))
			Sum += Neighbors[i]->GetPosition();
	}
	return Sum / static_cast<float>(NrOfNeighbors);
}

FVector2D Flock::GetAverageNeighborVelocity() const
{
	if (NrOfNeighbors == 0)
		return FVector2D::ZeroVector;

	FVector2D Sum = FVector2D::ZeroVector;
	for (int i = 0; i < NrOfNeighbors; ++i)
	{
		if (IsValid(Neighbors[i]))
			Sum += Neighbors[i]->GetLinearVelocity();
	}
	return Sum / static_cast<float>(NrOfNeighbors);
}

void Flock::SetTarget_Seek(FSteeringParams const& Target)
{
	pSeekBehavior->SetTarget(Target);
}

