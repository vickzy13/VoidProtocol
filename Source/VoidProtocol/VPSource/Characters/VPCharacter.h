// VPCharacter.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VPCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UVPHealthComponent;
struct FInputActionValue;
class UInputMappingContext;

UENUM(BlueprintType)
enum class EVPRole : uint8
{
    None        UMETA(DisplayName = "None"),
    Infiltrator UMETA(DisplayName = "Infiltrator"),
    Hacker      UMETA(DisplayName = "Hacker")
};

UCLASS()
class VOIDPROTOCOL_API AVPCharacter : public ACharacter
{
    GENERATED_BODY()

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UVPHealthComponent* HealthComponent;

    bool bIsAiming = false;
    float DefaultArmLength = 300.f;
    float AimArmLength = 150.f;
    FVector DefaultSocketOffset = FVector(0.f, 60.f, 70.f);
    FVector PeekLeftOffset = FVector(0.f, -60.f, 70.f);

    void AddMappingContexts(APlayerController* PC);
    void AimStart();
    void AimEnd();
    void UpdateCoverPeek(float DeltaTime);



    UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputAction* JumpAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputAction* MoveAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputAction* LookAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputAction* MouseLookAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputAction* AimAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputMappingContext* MouseLookMappingContext;

protected:
    UPROPERTY(ReplicatedUsing = OnRep_Role, BlueprintReadOnly, Category = "Role")
    EVPRole VPRole = EVPRole::None;

    UFUNCTION()
    void OnRep_Role();

    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void PossessedBy(AController* NewController) override;
    virtual void OnRep_Controller() override;

    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);

public:
    AVPCharacter();

    virtual void Tick(float DeltaTime) override;

    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

    // Role — GameMode calls SetRole_Server, anyone can read GetRole
    void SetRole_Server(EVPRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Role")
    EVPRole GetRole() const { return VPRole; }

    // Input interface — callable from Blueprint or UI
    UFUNCTION(BlueprintCallable, Category = "Input")
    virtual void DoMove(float Right, float Forward);

    UFUNCTION(BlueprintCallable, Category = "Input")
    virtual void DoLook(float Yaw, float Pitch);

    UFUNCTION(BlueprintCallable, Category = "Input")
    virtual void DoJumpStart();

    UFUNCTION(BlueprintCallable, Category = "Input")
    virtual void DoJumpEnd();

    FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
    FORCEINLINE UVPHealthComponent* GetHealthComponent() const { return HealthComponent; }
};