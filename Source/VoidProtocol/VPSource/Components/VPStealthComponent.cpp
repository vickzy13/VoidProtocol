// VPStealthComponent.cpp
#include "VPSource/Components/VPStealthComponent.h"
#include "Perception/AISense_Hearing.h"

UVPStealthComponent::UVPStealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UVPStealthComponent::OnFootstep(bool bIsCrouching, bool bIsSprinting)
{
    // Only server reports noise — never trust the client for this
    if (!GetOwner()->HasAuthority()) return;

    float NoiseRadius = WalkNoiseRadius;
    if (bIsCrouching)      NoiseRadius = CrouchNoiseRadius;
    else if (bIsSprinting) NoiseRadius = SprintNoiseRadius;

    UAISense_Hearing::ReportNoiseEvent(
        GetWorld(),
        GetOwner()->GetActorLocation(),
        1.0f,
        GetOwner(),
        NoiseRadius,
        FName("Footstep")
    );

    UE_LOG(LogTemp, Warning, TEXT("%s footstep: radius %.0f (crouch=%d sprint=%d)"),
        *GetOwner()->GetName(), NoiseRadius, bIsCrouching, bIsSprinting);
}