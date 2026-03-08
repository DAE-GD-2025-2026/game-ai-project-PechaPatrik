#include "Level_CombinedSteering.h"

#include "imgui.h"


// Sets default values
ALevel_CombinedSteering::ALevel_CombinedSteering()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevel_CombinedSteering::BeginPlay()
{
	Super::BeginPlay();

	m_pWanderer = GetWorld()->SpawnActor<ASteeringAgent>(
		SteeringAgentClass, FVector{ 0.f, 0.f, 90.f }, FRotator::ZeroRotator);

	if (IsValid(m_pWanderer))
	{
		m_pWandererEvade = std::make_unique<Evade>();
		m_pWandererEvade->SetEvadeRadius(300.f);

		m_pWander = std::make_unique<Wander>();

		m_pWandererPrioritySteering = std::make_unique<PrioritySteering>(
			std::vector<ISteeringBehavior*>{
			m_pWandererEvade.get(),
				m_pWander.get()
		}
		);

		m_pWanderer->SetSteeringBehavior(m_pWandererPrioritySteering.get());
	}

	m_pSeeker = GetWorld()->SpawnActor<ASteeringAgent>(
		SteeringAgentClass, FVector{ 200.f, 0.f, 90.f }, FRotator::ZeroRotator);

	if (IsValid(m_pSeeker))
	{
		m_pSeek = std::make_unique<Seek>();

		m_pSeekerEvade = std::make_unique<Evade>();
		m_pSeekerEvade->SetEvadeRadius(300.f);

		m_pBlendedSteering = std::make_unique<BlendedSteering>(
			std::vector<BlendedSteering::WeightedBehavior>{
				{ m_pSeek.get(), 0.7f },
				{ m_pSeekerEvade.get(), 0.5f },
		}
		);

		m_pSeeker->SetSteeringBehavior(m_pBlendedSteering.get());
	}
}

void ALevel_CombinedSteering::BeginDestroy()
{
	Super::BeginDestroy();

	if (IsValid(m_pWanderer)) m_pWanderer->Destroy();
	if (IsValid(m_pSeeker))   m_pSeeker->Destroy();
}

// Called every frame
void ALevel_CombinedSteering::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
#pragma region UI
	//UI
	{
		//Setup
		bool windowActive = true;
		ImGui::SetNextWindowPos(WindowPos);
		ImGui::SetNextWindowSize(WindowSize);
		ImGui::Begin("Game AI", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	
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
		ImGui::Spacing();
	
		ImGui::Text("Combined Steering");
		ImGui::Spacing();
		ImGui::Spacing();
	
		if (ImGui::Checkbox("Debug Rendering", &CanDebugRender))
		{
			if (IsValid(m_pWanderer)) m_pWanderer->SetDebugRenderingEnabled(CanDebugRender);
			if (IsValid(m_pSeeker))   m_pSeeker->SetDebugRenderingEnabled(CanDebugRender);
		}

		ImGui::Checkbox("Use Mouse Target", &UseMouseTarget);

		ImGui::Checkbox("Trim World", &TrimWorld->bShouldTrimWorld);
		if (TrimWorld->bShouldTrimWorld)
		{
			ImGuiHelpers::ImGuiSliderFloatWithSetter("Trim Size",
				TrimWorld->GetTrimWorldSize(), 1000.f, 3000.f,
				[this](float InVal) { TrimWorld->SetTrimWorldSize(InVal); });
		}
		
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
	
		ImGui::Text("Behavior Weights");
		ImGui::Spacing();


		if (m_pBlendedSteering)
		{
			ImGuiHelpers::ImGuiSliderFloatWithSetter("Seek",
				m_pBlendedSteering->GetWeightedBehaviorsRef()[0].Weight, 0.f, 1.f,
				[this](float InVal) { m_pBlendedSteering->GetWeightedBehaviorsRef()[0].Weight = InVal; }, "%.2f");

			ImGuiHelpers::ImGuiSliderFloatWithSetter("Evade Wanderer",
				m_pBlendedSteering->GetWeightedBehaviorsRef()[1].Weight, 0.f, 1.f,
				[this](float InVal) { m_pBlendedSteering->GetWeightedBehaviorsRef()[1].Weight = InVal; }, "%.2f");
		}
	
		//End
		ImGui::End();
	}
#pragma endregion

	// Combined Steering Update
	if (m_pSeek && UseMouseTarget)
		m_pSeek->SetTarget(MouseTarget);

	if (m_pWandererEvade && IsValid(m_pSeeker))
	{
		FTargetData SeekerTarget{};
		SeekerTarget.Position = m_pSeeker->GetPosition();
		SeekerTarget.LinearVelocity = m_pSeeker->GetLinearVelocity();
		SeekerTarget.Orientation = m_pSeeker->GetRotation();
		m_pWandererEvade->SetTarget(SeekerTarget);
	}

	if (m_pSeekerEvade && IsValid(m_pWanderer))
	{
		FTargetData WandererTarget{};
		WandererTarget.Position = m_pWanderer->GetPosition();
		WandererTarget.LinearVelocity = m_pWanderer->GetLinearVelocity();
		WandererTarget.Orientation = m_pWanderer->GetRotation();
		m_pSeekerEvade->SetTarget(WandererTarget);
	}
}
