// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SteeringBehaviors.h"
#include <vector>
#include <memory>
#include <string>

#include "GameAIProg/Shared/Level_Base.h"
#include "Level_SteeringBehaviors.generated.h"

UCLASS()
class GAMEAIPROG_API ALevel_SteeringBehaviors : public ALevel_Base
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALevel_SteeringBehaviors();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual void BeginDestroy() override;

private:
	enum class BehaviorTypes
	{
		Seek,
		Flee,
		Arrive,
		Face,
		Pursuit,
		Evade,
		Wander,

		// @ End
		Count
	};

	struct ImGui_Agent final
	{
		ASteeringAgent* Agent{nullptr};
		std::unique_ptr<ISteeringBehavior> Behavior{nullptr};
		int SelectedBehavior{static_cast<int>(BehaviorTypes::Seek)};
		int SelectedTarget = -1;
	};
	
	std::vector<ImGui_Agent> SteeringAgents{};
	std::vector<std::string> TargetLabels{};
	
	int AgentIndexToRemove = -1;
	
	bool AddAgent(BehaviorTypes BehaviorType = BehaviorTypes::Wander, bool AutoOrient = true);
	void RemoveAgent(unsigned int Index);
	void SetAgentBehavior(ImGui_Agent& Agent);

	void RefreshTargetLabels();
	void UpdateTarget(ImGui_Agent& Agent);
	void RefreshAgentTargets(unsigned int IndexRemoved);
};
