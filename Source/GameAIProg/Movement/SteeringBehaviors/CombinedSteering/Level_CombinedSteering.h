// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombinedSteeringBehaviors.h"
#include "GameAIProg/Shared/Level_Base.h"
#include "GameAIProg/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"
#include <memory>
#include "Level_CombinedSteering.generated.h"

UCLASS()
class GAMEAIPROG_API ALevel_CombinedSteering : public ALevel_Base
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALevel_CombinedSteering();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void BeginDestroy() override;

private:
	//Datamembers
	bool UseMouseTarget = true;
	bool CanDebugRender = true;

	ASteeringAgent* m_pWanderer{ nullptr };
	ASteeringAgent* m_pSeeker{ nullptr };

	// PrioritySteering(evade seeker > wander)
	std::unique_ptr<Evade> m_pWandererEvade{};
	std::unique_ptr<Wander> m_pWander{};
	std::unique_ptr<PrioritySteering> m_pWandererPrioritySteering{};

	// BlendedSteering(seek mouse + evade wanderer)
	std::unique_ptr<Seek> m_pSeek{};
	std::unique_ptr<Evade> m_pSeekerEvade{};
	std::unique_ptr<BlendedSteering> m_pBlendedSteering{};
};
