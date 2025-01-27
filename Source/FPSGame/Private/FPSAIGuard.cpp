// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSAIGuard.h"
#include "Perception/PawnSensingComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "FPSGameMode.h"
#include "Engine/World.h"
#include "Engine/TargetPoint.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AFPSAIGuard::AFPSAIGuard()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));
}

// Called when the game starts or when spawned
void AFPSAIGuard::BeginPlay()
{
	Super::BeginPlay();
	PawnSensingComp->OnSeePawn.AddDynamic(this, &AFPSAIGuard::OnSeenPawn);
	PawnSensingComp->OnHearNoise.AddDynamic(this, &AFPSAIGuard::OnHearNoise);
	OriginalLocation = GetActorRotation();	
	SetGuardState(EAIState::Idle);
}

void AFPSAIGuard::OnSeenPawn(APawn* SeenPawn)
{
	if (SeenPawn == nullptr) { return; }
	SetGuardState(EAIState::Alerted);
	DrawDebugSphere(GetWorld(), SeenPawn->GetActorLocation(), 32.0f, 12, FColor::Cyan, false, 10.0f);
	UE_LOG(LogTemp, Warning, TEXT("I have see: %s"), *SeenPawn->GetName());
	AFPSGameMode* GM = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		GM->CompleteMission(SeenPawn, false);
	}	
	
}

void AFPSAIGuard::OnHearNoise(APawn* Pawn, const FVector& Location, float Volume)
{
	if (GuardState == EAIState::Alerted) { return; }
	if (Pawn == nullptr) { return; }
	SetGuardState(EAIState::Suspicious);
	DrawDebugSphere(GetWorld(),Location, 32.0f, 12, FColor::Red, false, 10.0f);
	UE_LOG(LogTemp, Warning, TEXT("I have hear: %s"), *Pawn->GetName());
	FVector Direction = Location - GetActorLocation();
	Direction.Normalize();
	FRotator NewLookAt = FRotationMatrix::MakeFromX(Direction).Rotator();
	NewLookAt.Pitch = 0.0f;
	NewLookAt.Roll = 0.0f;
	SetActorRotation(NewLookAt);	
	GetWorldTimerManager().ClearTimer(TimerHandle_ResetOrientation);
	GetWorldTimerManager().SetTimer(TimerHandle_ResetOrientation, this, &AFPSAIGuard::ResetOrientation, 3.0f);	
}

void AFPSAIGuard::ResetOrientation()
{
	if (GuardState == EAIState::Alerted) { return; }
	SetActorRotation(OriginalLocation);
	SetGuardState(EAIState::Idle);
}

void AFPSAIGuard::SetGuardState(EAIState NewState)
{
	if (GuardState == NewState) { return; }
	GuardState = NewState;	
	GuadrStateChanged.Broadcast(GuardState);
	OnRep_GuardState();
}

// Called every frame
void AFPSAIGuard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

EAIState AFPSAIGuard::GetGuardState()
{
	return GuardState;
}

void AFPSAIGuard::OnRep_GuardState()
{
	OnStateChanged(GuardState);
}


void AFPSAIGuard::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPSAIGuard, GuardState);
}