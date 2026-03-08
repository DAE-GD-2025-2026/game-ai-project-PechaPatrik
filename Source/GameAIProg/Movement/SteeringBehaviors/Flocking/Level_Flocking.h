// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Flock.h"
#include "Shared/Level_Base.h"
#include "Level_Flocking.generated.h"

UCLASS()
class GAMEAIPROG_API ALevel_Flocking : public ALevel_Base
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALevel_Flocking();

	virtual void Tick(float DeltaTime) override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool bUseMouseTarget{true};

	int const FlockSize{100};

	TUniquePtr<Flock> pFlock{};
	
	UPROPERTY(EditAnywhere, Category = "Flocking")
	ASteeringAgent* pAgentToEvade{nullptr}; // non owning ref
};
