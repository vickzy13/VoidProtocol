#include "VPSource/Components/VPInfiltratorComponent.h"
#include "VPSource/Characters/VPGuard.h"
#include "VPSource/AI/VPGuardController.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/OverlapResult.h"

UVPInfiltratorComponent::UVPInfiltratorComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UVPInfiltratorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Scan for takedown targets every 0.2s
    GetOwner()->GetWorldTimerManager().SetTimer(TakedownScanTimer,
        this, &UVPInfiltratorComponent::ScanForTakedown, 0.2f, true);
}

void UVPInfiltratorComponent::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UVPInfiltratorComponent, CarriedGuard);
}

void UVPInfiltratorComponent::ScanForTakedown()
{
    APawn* Owner = Cast<APawn>(GetOwner());
    if (!Owner) return;
    if (bIsCarrying) return;

    FVector Start = Owner->GetActorLocation();
    FVector Forward = Owner->GetActorForwardVector();

    TakedownTarget = nullptr;

    // Find all guards in range
    TArray<FOverlapResult> Overlaps;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(TakedownRange);
    FCollisionObjectQueryParams ObjectParams;
    ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    GetWorld()->OverlapMultiByObjectType(Overlaps, Start,
        FQuat::Identity, ObjectParams, Sphere, Params);

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AVPGuard* Guard = Cast<AVPGuard>(Overlap.GetActor());
        if (!Guard || Guard->IsUnconscious())
        {
            continue;
        }

        AActor* OwnerActor = GetOwner();
        if (!OwnerActor)
        {
            continue;
        }

        // Direction from Guard -> Player
        const FVector GuardToPlayer =
            (OwnerActor->GetActorLocation() - Guard->GetActorLocation()).GetSafeNormal();

        // Direction Guard is facing
        const FVector GuardForward = Guard->GetActorForwardVector();

        // +1 = Player in front of guard
        //  0 = Player at side
        // -1 = Player directly behind guard
        const float Dot = FVector::DotProduct(GuardForward, GuardToPlayer);

        // Behind cone angle
        const float HalfAngle = TakedownAngle * 0.5f;
        const float Threshold = -FMath::Cos(FMath::DegreesToRadians(HalfAngle));

        UE_LOG(LogTemp, Warning,
            TEXT("Guard=%s Dot=%f Threshold=%f"),
            *Guard->GetName(),
            Dot,
            Threshold);

        if (Dot <= Threshold)
        {
            TakedownTarget = Guard;

            UE_LOG(LogTemp, Warning,
                TEXT("Takedown target: %s"),
                *Guard->GetName());

            break;
        }
    }
}

void UVPInfiltratorComponent::TryTakedown()
{
    if (bIsCarrying)
    {
        DropBody();
        return;
    }

    // Scan immediately before checking target
    ScanForTakedown();

    if (!TakedownTarget)
    {
        UE_LOG(LogTemp, Warning, TEXT("TryTakedown: No target in range"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("TryTakedown: Taking down %s"),
        *TakedownTarget->GetName());

    ServerTakedown(TakedownTarget);
}

void UVPInfiltratorComponent::ServerTakedown_Implementation(AVPGuard* Guard)
{
    if (!Guard || Guard->IsUnconscious()) return;

    // Make guard unconscious
    Guard->SetUnconscious(true);

    // Play takedown FX on all clients
    MulticastTakedownFX(Guard);

    UE_LOG(LogTemp, Warning, TEXT("ServerTakedown: %s is now unconscious"),
        *Guard->GetName());
}

void UVPInfiltratorComponent::MulticastTakedownFX_Implementation(AVPGuard* Guard)
{
    if (!Guard) return;

    APawn* Owner = Cast<APawn>(GetOwner());
    if (!Owner) return;

    // Attach guard to infiltrator
    FAttachmentTransformRules Rules(EAttachmentRule::SnapToTarget, true);
    Guard->AttachToActor(Owner, Rules);

    // Offset guard in front of player
    Guard->SetActorRelativeLocation(FVector(CarryOffset, 0.f, -50.f));
    Guard->SetActorRelativeRotation(FRotator::ZeroRotator);

    bIsCarrying = true;
    CarriedGuard = Guard;

    UE_LOG(LogTemp, Warning, TEXT("MulticastTakedownFX: Carrying %s"), *Guard->GetName());
}

void UVPInfiltratorComponent::DropBody()
{
    if (!bIsCarrying || !CarriedGuard) return;
}

void UVPInfiltratorComponent::ServerHideBody_Implementation(AActor* HidingSpot)
{
    UE_LOG(LogTemp, Warning, TEXT("ServerHideBody called | CarriedGuard: %s | HidingSpot: %s"),
        CarriedGuard ? *CarriedGuard->GetName() : TEXT("NULL"),
        HidingSpot ? *HidingSpot->GetName() : TEXT("NULL"));

    if (!CarriedGuard)
    {
        UE_LOG(LogTemp, Error, TEXT("ServerHideBody: CarriedGuard is NULL"));
        return;
    }

    if (!HidingSpot)
    {
        UE_LOG(LogTemp, Error, TEXT("ServerHideBody: HidingSpot is NULL"));
        return;
    }

    AVPGuard* GuardToHide = CarriedGuard;

    FDetachmentTransformRules Rules(EDetachmentRule::KeepWorld, true);
    GuardToHide->DetachFromActor(Rules);
    UE_LOG(LogTemp, Warning, TEXT("Guard detached"));

    GuardToHide->SetActorLocation(HidingSpot->GetActorLocation());
    UE_LOG(LogTemp, Warning, TEXT("Guard moved to hiding spot"));

    GuardToHide->SetActorHiddenInGame(true);
    UE_LOG(LogTemp, Warning, TEXT("Guard hidden: %s"),
        GuardToHide->IsHidden() ? TEXT("YES") : TEXT("NO"));

    GuardToHide->SetActorEnableCollision(false);

    bIsCarrying = false;
    CarriedGuard = nullptr;

    UE_LOG(LogTemp, Warning, TEXT("ServerHideBody complete"));
}