// Fill out your copyright notice in the Description page of Project Settings.


#include "BlackHole.h"
#include "Components/SphereComponent.h"

// Sets default values
ABlackHole::ABlackHole()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetRootComponent(MeshComp);
	//Внутрення сфера, отвечающая за уничтожение компонентов = поглащения предметов черной дырой
	InnerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InnerSphere"));
	InnerSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	InnerSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	//InnerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap);
	InnerSphere->AttachToComponent(MeshComp, FAttachmentTransformRules::KeepRelativeTransform);
	//Внешняя сфера, отвечающая за притягивание компонентов 
	OuterSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OuterSphere"));
	OuterSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	OuterSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	OuterSphere->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Ignore);
	OuterSphere->AttachToComponent(MeshComp, FAttachmentTransformRules::KeepRelativeTransform);
}

// Called when the game starts or when spawned
void ABlackHole::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ABlackHole::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TArray<UPrimitiveComponent*> OverlappingComponents;
	// Просмотр обьектов, которые пересекаются с внутренней сферой и должны быть уничтожены черной дырой
	InnerSphere->GetOverlappingComponents(OverlappingComponents);
	for (auto& OverlappedComponent : OverlappingComponents)
	{
		if (OverlappedComponent->GetOwner() != this)
		{
			OverlappedComponent->DestroyComponent(true);			
		}
	}
	// Просмотр обьектов, которые пересекаются с внешей сферой и должны притягиваться черной дырой
	OuterSphere->GetOverlappingComponents(OverlappingComponents);
	for (auto& OverlappedComponent : OverlappingComponents)
	{
		//
		if (OverlappedComponent->GetOwner() != this 
			&& OverlappedComponent->IsSimulatingPhysics())
		{
			OverlappedComponent->AddRadialForce(this->GetActorLocation(), OuterSphere->GetScaledSphereRadius(), -15000.0f, ERadialImpulseFalloff::RIF_Constant, true);
		}
	}
}

void ABlackHole::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

}


