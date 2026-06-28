#include "VPSource/Characters/VPPlayableCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Blueprint/UserWidget.h"
#include "VPSource/Components/VPStealthComponent.h"

AVPPlayableCharacter::AVPPlayableCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Camera
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 300.f;
    CameraBoom->SocketOffset = FVector(0.f, 60.f, 70.f);
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 10.f;
    CameraBoom->bUsePawnControlRotation = true;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // Stealth
    StealthComponent = CreateDefaultSubobject<UVPStealthComponent>(TEXT("StealthComponent"));

    // Movement
    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);
    GetCharacterMovement()->MaxWalkSpeed = 500.f;
    GetCharacterMovement()->JumpZVelocity = 500.f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
}

void AVPPlayableCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        GetWorldTimerManager().SetTimer(FootstepTimer, [this]()
            {
                if (!StealthComponent) return;
                if (GetVelocity().Size() < 10.f) return;
                bool bCrouching = GetCharacterMovement()->IsCrouching();
                bool bSprinting = GetVelocity().Size() > 500.f;
                StealthComponent->OnFootstep(bCrouching, bSprinting);
            }, 0.4f, true);
    }
}

void AVPPlayableCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    float TargetLength = bIsAiming ? AimArmLength : DefaultArmLength;
    CameraBoom->TargetArmLength = FMath::FInterpTo(
        CameraBoom->TargetArmLength, TargetLength, DeltaTime, 8.f);

    UpdateCoverPeek(DeltaTime);
}

void AVPPlayableCharacter::AddMappingContexts(APlayerController* PC)
{
    if (!PC) return;
    UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
    if (!Subsystem) return;
    if (DefaultMappingContext)
        Subsystem->AddMappingContext(DefaultMappingContext, 0);
    if (MouseLookMappingContext)
        Subsystem->AddMappingContext(MouseLookMappingContext, 1);
}

void AVPPlayableCharacter::SetupLocalPlayer(APlayerController* PC)
{
    if (!PC || !PC->IsLocalController()) return;
    AddMappingContexts(PC);
    PC->SetShowMouseCursor(false);
    PC->SetInputMode(FInputModeGameOnly());

    if (DetectionWidgetClass && !DetectionWidget)
    {
        DetectionWidget = CreateWidget<UUserWidget>(PC, DetectionWidgetClass);
        if (DetectionWidget)
            DetectionWidget->AddToViewport();
    }
    if (AlertWidgetClass && !AlertWidget)
    {
        AlertWidget = CreateWidget<UUserWidget>(PC, AlertWidgetClass);
        if (AlertWidget)
            AlertWidget->AddToViewport();
    }
    if (MissionWidgetClass && !MissionWidget)
    {
        MissionWidget = CreateWidget<UUserWidget>(PC, MissionWidgetClass);
        if (MissionWidget)
            MissionWidget->AddToViewport();
    }
}

void AVPPlayableCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    SetupLocalPlayer(Cast<APlayerController>(NewController));
}

void AVPPlayableCharacter::OnRep_Controller()
{
    Super::OnRep_Controller();
    SetupLocalPlayer(Cast<APlayerController>(GetController()));
}

void AVPPlayableCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (JumpAction)
        {
            EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
            EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        }
        if (MoveAction)
            EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AVPPlayableCharacter::Move);
        if (LookAction)
            EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AVPPlayableCharacter::Look);
        if (MouseLookAction)
            EIC->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AVPPlayableCharacter::Look);
        if (AimAction)
        {
            EIC->BindAction(AimAction, ETriggerEvent::Started, this, &AVPPlayableCharacter::AimStart);
            EIC->BindAction(AimAction, ETriggerEvent::Completed, this, &AVPPlayableCharacter::AimEnd);
        }
        if (CrouchAction)
        {
            EIC->BindAction(CrouchAction, ETriggerEvent::Started, this, &AVPPlayableCharacter::StartCrouch);
            EIC->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AVPPlayableCharacter::EndCrouch);
        }
        SetupAbilityInputBindings(EIC);
    }
}

void AVPPlayableCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    DoMove(MovementVector.X, MovementVector.Y);
}

void AVPPlayableCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookVector = Value.Get<FVector2D>();
    DoLook(LookVector.X, LookVector.Y);
}

void AVPPlayableCharacter::DoMove(float Right, float Forward)
{
    if (!GetController()) return;
    const FRotator Rotation = GetController()->GetControlRotation();
    const FRotator YawRotation(0, Rotation.Yaw, 0);
    AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Forward);
    AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Right);
}

void AVPPlayableCharacter::DoLook(float Yaw, float Pitch)
{
    if (!GetController()) return;
    AddControllerYawInput(Yaw);
    AddControllerPitchInput(Pitch);
}

void AVPPlayableCharacter::DoJumpStart() { Jump(); }
void AVPPlayableCharacter::DoJumpEnd() { StopJumping(); }
void AVPPlayableCharacter::AimStart() { bIsAiming = true; }
void AVPPlayableCharacter::AimEnd() { bIsAiming = false; }
void AVPPlayableCharacter::StartCrouch()
{
    Crouch();
    UE_LOG(LogTemp, Warning, TEXT("%s crouching"), *GetName());
}
void AVPPlayableCharacter::EndCrouch()
{
    UnCrouch();
    UE_LOG(LogTemp, Warning, TEXT("%s uncrouching"), *GetName());
}

void AVPPlayableCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
    UE_LOG(LogTemp, Warning, TEXT("%s IS NOW CROUCHING"), *GetName());
}

void AVPPlayableCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
    UE_LOG(LogTemp, Warning, TEXT("%s STOPPED CROUCHING"), *GetName());
}

void AVPPlayableCharacter::OnRep_bIsDowned()
{
    Super::OnRep_bIsDowned();

    // Only show downed widget for locally controlled player
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC || !PC->IsLocalController()) return;

    if (bIsDowned)
    {
        if (DownedWidgetClass && !DownedWidget)
        {
            DownedWidget = CreateWidget<UUserWidget>(PC, DownedWidgetClass);
            if (DownedWidget)
                DownedWidget->AddToViewport(10); // high Z-order
        }
    }
    else
    {
        if (DownedWidget)
        {
            DownedWidget->RemoveFromParent();
            DownedWidget = nullptr;
        }
    }
}

void AVPPlayableCharacter::UpdateCoverPeek(float DeltaTime)
{
    FVector Start = GetActorLocation();
    FVector Right = GetActorRightVector() * 80.f;
    FHitResult HitRight, HitLeft;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bWallRight = GetWorld()->LineTraceSingleByChannel(
        HitRight, Start, Start + Right, ECC_Visibility, Params);

    FVector TargetOffset = DefaultSocketOffset;
    if (bWallRight) TargetOffset = PeekLeftOffset;

    CameraBoom->SocketOffset = FMath::VInterpTo(
        CameraBoom->SocketOffset, TargetOffset, DeltaTime, 6.f);
}