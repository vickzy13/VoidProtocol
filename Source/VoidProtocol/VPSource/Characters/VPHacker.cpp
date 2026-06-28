#include "VPSource/Characters/VPHacker.h"
#include "VPSource/Components/VPHackerComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/OverlapResult.h"
#include "VPSource/Components/VPHealthComponent.h"
#include "VPSource/VPGameMode.h"
#include "EnhancedInputComponent.h"

AVPHacker::AVPHacker()
{
    GetCharacterMovement()->MaxWalkSpeed = 450.f;
    VPRole = EVPRole::Hacker;

    HackerComponent = CreateDefaultSubobject<UVPHackerComponent>(TEXT("HackerComponent"));
}

void AVPHacker::SetupAbilityInputBindings(UEnhancedInputComponent* EIC)
{
    if (HackAction)
    {
        EIC->BindAction(HackAction, ETriggerEvent::Started,
            this, &AVPHacker::OnHackPressed);
        EIC->BindAction(HackAction, ETriggerEvent::Completed,
            this, &AVPHacker::OnHackReleased);
    }
    if (JumpCameraAction)
        EIC->BindAction(JumpCameraAction, ETriggerEvent::Started,
            this, &AVPHacker::OnJumpCamera);
    if (ExitCameraAction)
        EIC->BindAction(ExitCameraAction, ETriggerEvent::Started,
            this, &AVPHacker::OnExitCamera);
}

void AVPHacker::OnHackPressed() { HackerComponent->StartHack(); }
void AVPHacker::OnHackReleased() { HackerComponent->StopHack(); }
void AVPHacker::OnJumpCamera()
{
    UE_LOG(LogTemp, Warning, TEXT("OnJumpCamera pressed"));

    // Revive takes priority
    if (TryRevivePartner()) return;

    // No downed partner — jump camera
    HackerComponent->JumpToNextNode();
}
void AVPHacker::OnExitCamera() { HackerComponent->ExitCamera(); }

bool AVPHacker::TryRevivePartner()
{
    FVector Start = GetActorLocation();
    TArray<FOverlapResult> Overlaps;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(200.f);
    FCollisionObjectQueryParams ObjectParams;
    ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    GetWorld()->OverlapMultiByObjectType(Overlaps, Start,
        FQuat::Identity, ObjectParams, Sphere, Params);

    for (auto& Overlap : Overlaps)
    {
        AVPCharacter* VPChar = Cast<AVPCharacter>(Overlap.GetActor());
        if (VPChar && VPChar->IsDowned())
        {
            UE_LOG(LogTemp, Warning, TEXT("Requesting revive for: %s"), *VPChar->GetName());
            ServerRequestRevive(VPChar); // ← RPC to server
            return true;
        }
    }
    // After the overlap loop, before return false:
    UE_LOG(LogTemp, Warning, TEXT("TryRevivePartner: No downed player in range — are you close enough?"));
    return false;
}

void AVPHacker::ServerRequestRevive_Implementation(AVPCharacter* PlayerToRevive)
{
    if (!PlayerToRevive || !PlayerToRevive->IsDowned()) return;

    PlayerToRevive->SetDowned(false);

    if (UVPHealthComponent* HC = PlayerToRevive->GetHealthComponent())
        HC->Revive(50.f);

    UE_LOG(LogTemp, Warning, TEXT("Hacker revived: %s"), *PlayerToRevive->GetName());
}