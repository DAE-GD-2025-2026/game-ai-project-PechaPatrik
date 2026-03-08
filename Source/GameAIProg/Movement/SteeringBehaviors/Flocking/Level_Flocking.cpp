// Fill out your copyright notice in the Description page of Project Settings.


#include "Level_Flocking.h"


// Sets default values
ALevel_Flocking::ALevel_Flocking()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevel_Flocking::BeginPlay()
{
	Super::BeginPlay();

	TrimWorld->SetTrimWorldSize(1000.f);
	TrimWorld->bShouldTrimWorld = true;

	pAgentToEvade = GetWorld()->SpawnActor<ASteeringAgent>(
		SteeringAgentClass, FVector{ 0.f, 0.f, 90.f }, FRotator::ZeroRotator);

	if (IsValid(pAgentToEvade))
	{
		pEvadeAgentBehavior = std::make_unique<Wander>();
		pAgentToEvade->SetSteeringBehavior(pEvadeAgentBehavior.get());
		pAgentToEvade->SetDebugRenderingEnabled(false);
	}

	pFlock = TUniquePtr<Flock>(
		new Flock(
			GetWorld(),
			SteeringAgentClass,
			FlockSize,
			TrimWorld->GetTrimWorldSize(),
			pAgentToEvade,
			true)
			);
}

// Called every frame
void ALevel_Flocking::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	pFlock->ImGuiRender(WindowPos, WindowSize);
	pFlock->Tick(DeltaTime);
	pFlock->RenderDebug();
	if (bUseMouseTarget)
		pFlock->SetTarget_Seek(MouseTarget);

	if (IsValid(pAgentToEvade))
	{
		DrawDebugPoint(GetWorld(),
			FVector{ pAgentToEvade->GetPosition(), 0.f },
			30.f, FColor::Red);
	}
}

