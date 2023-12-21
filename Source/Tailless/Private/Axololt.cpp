// Fill out your copyright notice in the Description page of Project Settings.


#include "Axololt.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"
//#include "Blueprint/WidgetLayoutLibrary.h"


// Sets default values
AAxololt::AAxololt()
{
	RotatorProjectile = FRotator(0, 0, 0);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, -30.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // We want to pull camera in when it collides with level

	// Create a camera...
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

}

// Called when the game starts or when spawned
void AAxololt::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = Cast<APlayerController>(Controller);

	if (PlayerController) {
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())) {
			Subsystem->AddMappingContext(DefaultMappingContent, 0);
		}
	}

}

// Called every frame
void AAxololt::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AAxololt::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Triggered, this, &AAxololt::Dashing);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAxololt::Move);

		//EnhancedInputComponent->BindAction(LAttackAction, ETriggerEvent::Triggered, this, &AAxololt::LightAttack);
	}

}

void AAxololt::Move(const FInputActionValue& _value) {
	FVector2D MovementVector = _value.Get<FVector2D>();

	if (Controller != nullptr) {
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);

		//Calcula la direccion del movimiento
		FVector MovementDirection = ForwardDirection * MovementVector.Y + RightDirection * MovementVector.X;

		if (!MovementDirection.IsNearlyZero()) {
			//Normaliza la direccion del movimiento
			MovementDirection.Normalize();

			//Calcula la rotacion hacia la que el personaje deberia mirara
			FRotator NewLookAt = MovementDirection.Rotation();

			//Obtiene la rotacion actual del personaje 
			FRotator CurrentRotation = GetActorRotation();

			//Interpola suavemente hacia la nueva rotacion
			FRotator SmoothRotation = FMath::RInterpTo(CurrentRotation, NewLookAt, GetWorld()->GetDeltaSeconds(), 2.0f);

			//Actualiza la rotacion del perosnaje para que mire en la direccion del movimiento
			SetActorRotation(SmoothRotation);
			UE_LOG(LogTemp, Display, TEXT("SmoothRotation: %s"), *SmoothRotation.ToString());
		}
	}
}

void AAxololt::Dashing() 
{	
	if (!DashDisable)
	{
		const FVector ForwardDir = this->GetActorForwardVector();
		LaunchCharacter(ForwardDir * DashDistance, true, false);
	}
}

void AAxololt::LightAttack()
{
	UE_LOG(LogTemp, Display, TEXT("Se hace"));

	FVector Start = GetActorLocation();
	FVector End = Start + (GetActorForwardVector() * AttackRange);

	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	TArray<AActor*> CharacterChildren;
	GetAllChildActors(CharacterChildren);
	CollisionParams.AddIgnoredActors(CharacterChildren);
	CollisionParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, CollisionParams);
	UE_LOG(LogTemp, Display, TEXT("Se entra? %b"), bHit);


	if (bHit) {
		AActor* HitActor = HitResult.GetActor();
		if (HitActor != nullptr) {

			FDamageEvent DamageEvent(PlayerDamageType);


			AController* InstitigatedBy = GetWorld()->GetFirstPlayerController();

			AActor* DamageCauser = this;

			HitActor->TakeDamage(WeponDamage, DamageEvent, InstitigatedBy, DamageCauser);
			UE_LOG(LogTemp, Display, TEXT("Pegado"));
		}
	}
}

bool AAxololt::IsAttacking()
{
	return false;
}

void AAxololt::AddHealth(float _healthAmount) {
	Health += _healthAmount;
}

float AAxololt::InternalTakePointDamage(float Damage, FPointDamageEvent const& RadialDamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Health -= Damage;
	return Health;
}

float AAxololt::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = DamageAmount;

	UDamageType const* const DamageTypeCDO = DamageEvent.DamageTypeClass ? DamageEvent.DamageTypeClass->GetDefaultObject<UDamageType>() : GetDefault<UDamageType>();
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		// point damage event, pass off to helper function
		FPointDamageEvent* const PointDamageEvent = (FPointDamageEvent*)&DamageEvent;
		ActualDamage = InternalTakePointDamage(ActualDamage, *PointDamageEvent, EventInstigator, DamageCauser);

		// K2 notification for this actor
		if (ActualDamage != 0.f)
		{
			ReceivePointDamage(ActualDamage, DamageTypeCDO, PointDamageEvent->HitInfo.ImpactPoint, PointDamageEvent->HitInfo.ImpactNormal, PointDamageEvent->HitInfo.Component.Get(), PointDamageEvent->HitInfo.BoneName, PointDamageEvent->ShotDirection, EventInstigator, DamageCauser, PointDamageEvent->HitInfo);
			OnTakePointDamage.Broadcast(this, ActualDamage, EventInstigator, PointDamageEvent->HitInfo.ImpactPoint, PointDamageEvent->HitInfo.Component.Get(), PointDamageEvent->HitInfo.BoneName, PointDamageEvent->ShotDirection, DamageTypeCDO, DamageCauser);

			// Notify the component
			UPrimitiveComponent* const PrimComp = PointDamageEvent->HitInfo.Component.Get();
			if (PrimComp)
			{
				PrimComp->ReceiveComponentDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
			}
		}
	}
	else if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
	{
		// radial damage event, pass off to helper function
		FRadialDamageEvent* const RadialDamageEvent = (FRadialDamageEvent*)&DamageEvent;
		ActualDamage = InternalTakeRadialDamage(ActualDamage, *RadialDamageEvent, EventInstigator, DamageCauser);

		// K2 notification for this actor
		if (ActualDamage != 0.f)
		{
			FHitResult const& Hit = (RadialDamageEvent->ComponentHits.Num() > 0) ? RadialDamageEvent->ComponentHits[0] : FHitResult();
			ReceiveRadialDamage(ActualDamage, DamageTypeCDO, RadialDamageEvent->Origin, Hit, EventInstigator, DamageCauser);
			OnTakeRadialDamage.Broadcast(this, ActualDamage, DamageTypeCDO, RadialDamageEvent->Origin, Hit, EventInstigator, DamageCauser);

			// add any desired physics impulses to our components
			for (int HitIdx = 0; HitIdx < RadialDamageEvent->ComponentHits.Num(); ++HitIdx)
			{
				FHitResult const& CompHit = RadialDamageEvent->ComponentHits[HitIdx];
				UPrimitiveComponent* const PrimComp = CompHit.Component.Get();
				if (PrimComp && PrimComp->GetOwner() == this)
				{
					PrimComp->ReceiveComponentDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
				}
			}
		}
	}

	// generic damage notifications sent for any damage
	// note we will broadcast these for negative damage as well
	if (ActualDamage != 0.f)
	{
		ReceiveAnyDamage(ActualDamage, DamageTypeCDO, EventInstigator, DamageCauser);
		OnTakeAnyDamage.Broadcast(this, ActualDamage, DamageTypeCDO, EventInstigator, DamageCauser);
		if (EventInstigator != nullptr)
		{
			EventInstigator->InstigatedAnyDamage(ActualDamage, DamageTypeCDO, this, DamageCauser);
		}
	}

	return ActualDamage;
}

void AAxololt::RangedAttack() {

	FHitResult Hit;
	bool bHitSuccessful = false;

	FVector Start = GetActorLocation();

	bHitSuccessful = PlayerController->GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);

	if (bHitSuccessful) {
		ProjectileDirection = Hit.Location - Start;
	}

	float ArrowSize = 10.f;
	float LifeTime = 0.1f;
	uint8 DepthPriority = 0;
	float Thickness = 2.0f;
	FColor Color = FColor::Red;

	ProjectileDirection.Z = Start.Z;

	//DrawDebugDirectionalArrow(GetWorld(), Start, ProjectileDirection + Start, ArrowSize, Color, false, LifeTime, DepthPriority, Thickness);

	FVector NormalizedStart = Start.GetSafeNormal();
	FVector NormalizedDirection = ProjectileDirection.GetSafeNormal();

	FRotator RotationProjectile = NormalizedDirection.Rotation();
	FRotator RotationStart = NormalizedStart.Rotation();

	FRotator NewLookAt = RotationProjectile;

	//float DotProduct = FVector::DotProduct(NormalizedStart, NormalizedDirection);
	//float AngleRad = FMath::Acos(DotProduct);

	//AngleProjectile = FMath::RadiansToDegrees(AngleRad);

	RotatorProjectile = NewLookAt;

	//UE_LOG(LogTemp, Display, TEXT("Draw Arrow"));

}
