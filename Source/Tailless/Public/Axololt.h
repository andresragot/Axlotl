// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Engine/DamageEvents.h"
#include "DrawDebugHelpers.h"
#include "Axololt.generated.h"

UCLASS()
class TAILLESS_API AAxololt : public ACharacter
{
	GENERATED_BODY()

private:
	//Spring Arm Component para que la cámara persiga detras al player
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true")) class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true")) class UCameraComponent* FollowCamera;

public:
	UPROPERTY(EditAnyWhere, Category = "PlayerCharacter Properties", BlueprintReadWrite) float DashDistance = 2000;
	UPROPERTY(EditAnyWhere, Category = "PlayerCharacter Properties", BlueprintReadWrite) float WeponDamage = 5;
	UPROPERTY(EditAnyWhere, Category = "PlayerCharacter Properties", BlueprintReadWrite) float AttackRange = 1000;
	UPROPERTY(EditAnyWhere, Category = "PlayerCharacter Properties", BlueprintReadWrite) TSubclassOf<UDamageType> PlayerDamageType;
	UPROPERTY(EditAnyWhere, Category = "PlayerCharacter Properties", BlueprintReadWrite) float Health = 100;
	UPROPERTY(EditAnyWhere, Category = "Attacks", BlueprintReadWrite) FVector ProjectileDirection;
	UPROPERTY(EditAnyWhere, Category = "Attacks", BlueprintReadWrite) float AngleProjectile = 0;
	UPROPERTY(EditAnyWhere, Category = "Attacks", BlueprintReadWrite) FRotator RotatorProjectile;
	UPROPERTY(EditAnyWhere, Category = "Dash", BlueprintReadWrite) bool DashDisable = false;


private:
	UFUNCTION(BlueprintCallable, Category = "Movement")
	//Llamado para adelante/atras inputs
	void Move(const FInputActionValue& _value);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void Dashing();

	UFUNCTION(BlueprintCallable, Category = "Attacks")
	void LightAttack();

	UFUNCTION(BlueprintCallable, Category = "Attacks")
	bool IsAttacking();

	UFUNCTION(BlueprintCallable, Category = "Health")
	void AddHealth(float HealthAmount);

	UFUNCTION(BlueprintCallable, Category = "Ranged")
	void RangedAttack();

protected:
	float InternalTakePointDamage(float Damage, struct FPointDamageEvent const& RadialDamageEvent, class AController* EventInstigator, AActor* DamageCauser);

	float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser);

public:
	// Sets default values for this character's properties
	AAxololt();

	//APawn interface
protected:
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;




public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

public:
	UPROPERTY(EditAnyWhere, Category = "PlayerCharacter Properties", BlueprintReadWrite)
	float NormalSpeedRun;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputMappingContext* DefaultMappingContent;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* DashAction;

	APlayerController* PlayerController = NULL;
};
