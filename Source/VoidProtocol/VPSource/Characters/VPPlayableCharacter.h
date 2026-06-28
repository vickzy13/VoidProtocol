#pragma once
#include "CoreMinimal.h"
#include "VPSource/Characters/VPCharacter.h"
#include "VPSource/Components/VPHealthComponent.h"
#include "VPPlayableCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UVPStealthComponent;
class UUserWidget;
struct FInputActionValue;

UCLASS()
class VOIDPROTOCOL_API AVPPlayableCharacter : public AVPCharacter
{
    GENERATED_BODY()

    //=========================================================
    // PRIVATE — camera, input actions, widgets, stealth
    // nothing here needs to be on guards
    //=========================================================
private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera",
        meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera",
        meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    UVPStealthComponent* StealthComponent;

    UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputMappingContext* MouseLookMappingContext;

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
    UInputAction* CrouchAction;

    // Detection widget
    UPROPERTY(EditDefaultsOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UUserWidget> DetectionWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UUserWidget> AlertWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UUserWidget> MissionWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UUserWidget> DownedWidgetClass;

    UPROPERTY()
    UUserWidget* MissionWidget = nullptr;

    UPROPERTY()
    UUserWidget* AlertWidget = nullptr;

    UPROPERTY()
    UUserWidget* DetectionWidget = nullptr;

    UPROPERTY()
    UUserWidget* DownedWidget = nullptr;

    // Camera state
    bool bIsAiming = false;
    float DefaultArmLength = 300.f;
    float AimArmLength = 150.f;
    FVector DefaultSocketOffset = FVector(0.f, 60.f, 70.f);
    FVector PeekLeftOffset = FVector(0.f, -60.f, 70.f);

    // Input helpers
    void AddMappingContexts(APlayerController* PC);
    void SetupLocalPlayer(APlayerController* PC);
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void AimStart();
    void AimEnd();
    void StartCrouch();
    void EndCrouch();
    void UpdateCoverPeek(float DeltaTime);
    void DebugTakeDamage();
    // Footstep timer
    FTimerHandle FootstepTimer;
    
protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual void SetupAbilityInputBindings(UEnhancedInputComponent* EIC) {}
    virtual void PossessedBy(AController* NewController) override;
    virtual void OnRep_Controller() override;
    virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
    virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
    virtual void OnRep_bIsDowned() override;
    
public:
    AVPPlayableCharacter();

    virtual void Tick(float DeltaTime) override;

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
    FORCEINLINE UVPStealthComponent* GetStealthComponent() const { return StealthComponent; }
};