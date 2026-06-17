#include "VPCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "VPSource/Components/VPHealthComponent.h"
#include "Net/UnrealNetwork.h"

AVPCharacter::AVPCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Spring Arm
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.f;
	CameraBoom->SocketOffset = FVector(0.f, 60.f, 70.f);  // over right shoulder
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 10.f;  // smooth follow, not snappy

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	CameraBoom->bUsePawnControlRotation = true;   // arm rotates with controller
	bUseControllerRotationYaw = false;             // character doesn't snap-rotate with camera
	GetCharacterMovement()->bOrientRotationToMovement = true; // character faces movement direction

	HealthComponent = CreateDefaultSubobject<UVPHealthComponent>(TEXT("HealthComponent"));
}

void AVPCharacter::BeginPlay() 
{ 
	Super::BeginPlay();
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}
void AVPCharacter::Tick(float DeltaTime) 
{ 
	Super::Tick(DeltaTime); 
	float TargetLength = bIsAiming ? AimArmLength : DefaultArmLength;
	CameraBoom->TargetArmLength = FMath::FInterpTo(
		CameraBoom->TargetArmLength,
		TargetLength,
		DeltaTime,
		8.f  // speed — higher = snappier
	);
	UpdateCoverPeek(DeltaTime);
}

void AVPCharacter::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AVPCharacter, VPRole);
}

void AVPCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	AddMappingContexts(Cast<APlayerController>(NewController));
}

void AVPCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();
	AddMappingContexts(Cast<APlayerController>(GetController()));
}

void AVPCharacter::OnRep_Role()
{
	UE_LOG(LogTemp, Warning, TEXT("%s role set on client: %s"),
		*GetName(),
		*UEnum::GetValueAsString(VPRole));
}

void AVPCharacter::SetRole_Server(EVPRole NewRole)
{
	// Only server should call this
	if (!HasAuthority()) return;
	VPRole = NewRole;
	OnRep_Role(); // manually call on server since RepNotify only fires on clients
}

void AVPCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AVPCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AVPCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AVPCharacter::Look);

		//Aiming
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &AVPCharacter::AimStart);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AVPCharacter::AimEnd);
	}
}

void AVPCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AVPCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AVPCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AVPCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AVPCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AVPCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void AVPCharacter::AddMappingContexts(APlayerController* PC)
{
	if (!PC) return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());

	if (!Subsystem) return;

	if (DefaultMappingContext)
		Subsystem->AddMappingContext(DefaultMappingContext, 0);

	if (MouseLookMappingContext)
		Subsystem->AddMappingContext(MouseLookMappingContext, 1); // priority 1 — higher than default
}

void AVPCharacter::AimStart() { bIsAiming = true; }
void AVPCharacter::AimEnd() { bIsAiming = false; }

void AVPCharacter::UpdateCoverPeek(float DeltaTime)
{
	FVector Start = GetActorLocation();
	FVector Right = GetActorRightVector() * 80.f;

	FHitResult HitRight, HitLeft;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bWallRight = GetWorld()->LineTraceSingleByChannel(
		HitRight, Start, Start + Right, ECC_Visibility, Params);
	bool bWallLeft = GetWorld()->LineTraceSingleByChannel(
		HitLeft, Start, Start - Right, ECC_Visibility, Params);

	FVector TargetOffset = DefaultSocketOffset;
	if (bWallRight) TargetOffset = PeekLeftOffset;   // wall on right → peek left

	CameraBoom->SocketOffset = FMath::VInterpTo(
		CameraBoom->SocketOffset,
		TargetOffset,
		DeltaTime,
		6.f
	);
}

float AVPCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	HealthComponent->HandleTakeDamage(DamageAmount);
	return DamageAmount;
}