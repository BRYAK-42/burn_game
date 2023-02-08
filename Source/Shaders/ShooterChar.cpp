
#include "ShooterChar.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Item.h"

#include "Particles/ParticleSystemComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"

// Sets default values
AShooterChar::AShooterChar()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraArm = CreateDefaultSubobject<USpringArmComponent>("CameraArm");
	CameraArm-> SetupAttachment(RootComponent);
	CameraArm-> bUsePawnControlRotation = true;
	CameraArm-> SocketOffset = FVector(0, 50.f, 80.f);
	CameraArm->TargetArmLength = 250.f;
	bIsAiming = false;
	//Camera
	CameraDefaultFOV = 0.f;
	CameraZoomedFOV = 40.f;
	FOVInterpolationSpeed = 20.f;
	CurrentCameraFOV = 0.f;

	CharCamera = CreateDefaultSubobject<UCameraComponent>("Character Camera");
	CharCamera-> SetupAttachment(CameraArm, USpringArmComponent::SocketName);
	CharCamera-> bUsePawnControlRotation = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 400.f;
	GetCharacterMovement()->AirControl = 0.2;



	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	HipTurnRate = 70.f;
	HipLookUpRate = 70.f;
	AimLookUpRate = 10.f;
	AimLookUpRate = 10.f;

	MouseHipTurnRate = 1;
	MouseHipLookUpRate = 1;
	MouseAimLookUpRate = 0.6;
	MouseAimLookUpRate = 0.6;

	CR_HAimFactor = 0.f;
	CR_HVelocityFactor = 0.f;
	CR_HInAirFactor = 0.f;
	CR_HAimFactor = 0.f;
	CR_HShootingFactor = 0.f;
	//Bullet Timer
	bIsFiring = false;
	ShootTimeDur = 0.05;

	//Automatic Fire
	bCanFire = true;
	bFireButtonPressed = false;
	AutoFireRate = 0.08;

	bShouldTrace = false;
	CameraInterpDistance = 250.f;
	CameraInterpElevation = 75.f;

	StartingAmmout9mm = 80;
	StartingAmmoutAR = 120;

	CombatState = ECombatState::ECS_Unoccupied;
	
}

// Called when the game starts or when spawned
void AShooterChar::BeginPlay()
{
	Super::BeginPlay();
	if (CharCamera)
	{
		CameraDefaultFOV = CharCamera->FieldOfView;
		CurrentCameraFOV = CameraDefaultFOV;
	}
	EquipWeapon(SpawnDefaultWeapon());
	InitializeAmmoMap();
}


// Called every frame
void AShooterChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SmoothZoom(DeltaTime);
	SetViewRates();
	CalculateCRHSpread(DeltaTime);
	TraceForItems();
	
}

// Called to bind functionality to input
void AShooterChar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);
	//Movement Functionality
	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterChar::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterChar::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterChar::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterChar::LookUpAtRate);

	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterChar::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterChar::FireButtonReleased);

	PlayerInputComponent->BindAxis("MouseTurn", this, &AShooterChar::Turn);
	PlayerInputComponent->BindAxis("MouseUp", this, &AShooterChar::LookUp);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Aiming", IE_Pressed, this, &AShooterChar::AimButtonPressed);
	PlayerInputComponent->BindAction("Aiming", IE_Released, this, &AShooterChar::AimButtonRelesed);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AShooterChar::InteractButtonPressed);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &AShooterChar::InteractButtonRelesed);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AShooterChar::ReloadButtonPressed);

}

void AShooterChar::MoveForward(float w)
{
	if (Controller != nullptr && w != 0.0f)
	{
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw,0 };
		const FVector Dir{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) };
		AddMovementInput(Dir, w);
	}
}

void AShooterChar::MoveRight(float d)
{
	if (Controller != nullptr && d != 0.0f)
	{
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw,0 };
		const FVector Dir{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) };
		AddMovementInput(Dir, d);
	}
}

void AShooterChar::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AShooterChar::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AShooterChar::FireWeapon()
{
	if (EquippedWeapon == nullptr) return;
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	if (bWeaponHasAmmo())
	{
		//Play Fire Sound
		PlayFireSound();
		//Send Bullet
		SendBullet();
		PlayGunFireMontage();
		//Start Bullet Timer CRH
		//?StartCRHBulletFire();
		EquippedWeapon->DecrementAmmo();
		StartFireTimer();
	}

}
void AShooterChar::FireButtonPressed()
{
	bFireButtonPressed = true;
	FireWeapon();
}
void AShooterChar::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void AShooterChar::AutoFireReset()
{
	CombatState = ECombatState::ECS_Unoccupied;

	if (bWeaponHasAmmo())
	{
		if (bFireButtonPressed)
		{
			FireWeapon();
		}
	}
	else
	{
		ReloadWeapon();
	}
}
void AShooterChar::StartFireTimer()
{
	CombatState = ECombatState::ECS_FireTemerInProgress;
	GetWorldTimerManager().SetTimer(AutoFireRateTime, this, &AShooterChar::AutoFireReset, AutoFireRate);
}

bool AShooterChar::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
		//Get screen space location of cr-h
	FHitResult CRHitResult;
	
	bool bCRHit = bTraceUnderCRH(CRHitResult,OutBeamLocation );
	if (bCRHit)
	{
		OutBeamLocation = CRHitResult.Location;
	}
	FHitResult WeaponTraceHit;
	const FVector StartToEnd = OutBeamLocation - MuzzleSocketLocation;
	const FVector WeaponTraceStart = MuzzleSocketLocation;
	const FVector WeaponTraceEnd = MuzzleSocketLocation + StartToEnd * 1.25;
	GetWorld()->LineTraceSingleByChannel(
		WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);

	if (WeaponTraceHit.bBlockingHit)
	{
		OutBeamLocation = WeaponTraceHit.Location;
		return true;
	}
	return false;
}

void AShooterChar::AimButtonPressed()
{
	bIsAiming = true;
}

void AShooterChar::AimButtonRelesed()
{
	bIsAiming = false;
}

void AShooterChar::SmoothZoom(float DeltaTime)
{

	if (bIsAiming)
	{
		CurrentCameraFOV = FMath::FInterpTo(CurrentCameraFOV, CameraZoomedFOV, DeltaTime, FOVInterpolationSpeed);
	}
	else
	{
		CurrentCameraFOV = FMath::FInterpTo(CurrentCameraFOV, CameraDefaultFOV, DeltaTime, FOVInterpolationSpeed);
	}
	CharCamera->SetFieldOfView(CurrentCameraFOV);
}

void AShooterChar::SetViewRates()
{
	if (bIsAiming)
	{
		BaseTurnRate = AimTurnRate;
		BaseLookUpRate = AimLookUpRate;
	}
	else
	{
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}
}

void AShooterChar::Turn(float val)
{
	float TurnScaleFactor;
	if (bIsAiming)
	{
		TurnScaleFactor = MouseAimTurnRate;
	}
	else
	{
		TurnScaleFactor = MouseHipTurnRate;
	}

	AddControllerYawInput(val * TurnScaleFactor);
}

void AShooterChar::LookUp(float val)
{
	float LookUpScaleFactor;
	if (bIsAiming)
	{
		LookUpScaleFactor = MouseAimLookUpRate;
	}
	else
	{
		LookUpScaleFactor = MouseHipLookUpRate;
	}

	AddControllerPitchInput(val * LookUpScaleFactor);
}

void AShooterChar::CalculateCRHSpread(float DeltaTime)
{
	FVector2D WalkSpeedRange{0.f,600.f};
	FVector2D VelMultRange{0.f,1.f};
	FVector Velocity = FVector(GetVelocity().X, GetVelocity().Y, 0.f);

	CR_HVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange,VelMultRange, Velocity.Size());

	//In Air effect
	if (GetCharacterMovement()->IsFalling())
	{
		CR_HInAirFactor = FMath::FInterpTo(CR_HInAirFactor, 3.f, DeltaTime, 2.25);
	}
	else
	{
		CR_HInAirFactor = FMath::FInterpTo(CR_HInAirFactor, 0.f, DeltaTime, 30.f);
	}

	//Aim effect
	if (bIsAiming)
	{
		CR_HAimFactor = FMath::FInterpTo(CR_HAimFactor, -0.35f, DeltaTime, 10.f);
	}
	else
	{
		CR_HAimFactor = FMath::FInterpTo(CR_HAimFactor, 0, DeltaTime, 10.f);
	}
	if (bIsFiring)
	{
		CR_HShootingFactor = FMath::FInterpTo(CR_HShootingFactor,0.3 , DeltaTime, 60.f);
	}
	else
	{
		CR_HShootingFactor = FMath::FInterpTo(CR_HShootingFactor,0.f, DeltaTime, 60.f);
	}

	CR_HSpreadMult = 0.5 + CR_HVelocityFactor + CR_HInAirFactor + CR_HAimFactor + CR_HShootingFactor;
}

void AShooterChar::StartCRHBulletFire()
{
	bIsFiring = true;
	GetWorldTimerManager().SetTimer(CRHShootTimer, this, &AShooterChar::FinishCRHBulletFire, ShootTimeDur);
}
void AShooterChar::FinishCRHBulletFire()
{
	bIsFiring = false;
}


float AShooterChar::GetCR_HSpreadMult() const
{
	return CR_HSpreadMult;
}

bool AShooterChar::bTraceUnderCRH(FHitResult& OutHit,FVector& OutHitLocation)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	FVector2D CrosshairLocation(ViewportSize.X / 2., ViewportSize.Y / 2.);
	FVector CrosshairWPos;
	FVector CrosshairWDir;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::
		GetPlayerController(this, 0), CrosshairLocation, CrosshairWPos, CrosshairWDir);
	if (bScreenToWorld)
	{
		FVector Start = CrosshairWPos;
		FVector End = Start + CrosshairWDir * 50000.f;
		OutHitLocation = End;
		GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECollisionChannel::ECC_Visibility);
		
		if (OutHit.bBlockingHit)
		{
			OutHitLocation = OutHit.Location;
			return true;
		}

	}
		return false;
}

void AShooterChar::TraceForItems()
{
	if (bShouldTrace)
	{

		bTraceUnderCRH(ItemTraseResult, OutBeamLoc);
		if (ItemTraseResult.bBlockingHit)
		{
			TraceHitItem = Cast<AItem>(ItemTraseResult.Actor);

			if (TraceHitItem && TraceHitItem->GetPickUpWidget())
			{
				TraceHitItem->GetPickUpWidget()->SetVisibleFlag(true);
			}

			if (TraceHitItemLastFrame)
			{
				if (TraceHitItem !=TraceHitItemLastFrame)
				{
					TraceHitItemLastFrame->GetPickUpWidget()->SetVisibility(false);
				
				}
			}
				TraceHitItemLastFrame = TraceHitItem;

		}
	}
	else if (TraceHitItemLastFrame)
	{
		TraceHitItemLastFrame->GetPickUpWidget()->SetVisibility(false);
	}
}

void AShooterChar::IncrementOverlappedItemCount(int Amount)
{
	if (OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTrace = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bShouldTrace = true;
	}
}

void AShooterChar::GetPickUpItem(AItem* Item)
{
	auto Weapon = Cast<AWeapon>(Item);
	if (Weapon)
	{
		SwapWeapon(Weapon);
	}
}

FVector AShooterChar::GetCameraInterpLocation()
{
	const FVector CameraWorldLocation(CharCamera->GetComponentLocation());
	const FVector CameraForward = CharCamera->GetForwardVector();
	const FVector CameraUp = FVector(0,0,CameraInterpElevation);

	return CameraWorldLocation + CameraForward * CameraInterpDistance + CameraUp;
}

AWeapon* AShooterChar::SpawnDefaultWeapon()
{
	if (DefaultWeaponClass)
	{
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
	}
	return nullptr;
}

void AShooterChar::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip)
	{

		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			HandSocket->AttachActor(WeaponToEquip, GetMesh());

		}
		EquippedWeapon = WeaponToEquip;
		EquippedWeapon->SetItemState(EItemState::EIS_Equiped);
	}
}

void AShooterChar::DropWeapon()
{
	if (EquippedWeapon)
	{	
		FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld,true);
		EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);
		EquippedWeapon->TrowWeapon();
	}
}

void AShooterChar::InteractButtonPressed()
{
	if (TraceHitItem)
	{
		TraceHitItem->StartItemCurve(this);
	}
}

void AShooterChar::InteractButtonRelesed()
{
}

void AShooterChar::SwapWeapon(AWeapon* WeaponToSwap)
{
	DropWeapon();
	EquipWeapon(WeaponToSwap);
	TraceHitItemLastFrame = TraceHitItem = nullptr;
	
}

bool AShooterChar::bWeaponHasAmmo()
{
	if (EquippedWeapon == nullptr) return false;
	return EquippedWeapon->GetAmmo() > 0;
}

void AShooterChar::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoTypes::EAT_9mm, StartingAmmout9mm);
	AmmoMap.Add(EAmmoTypes::EAT_AR, StartingAmmoutAR);
}

void AShooterChar::PlayFireSound()
{
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
}

void AShooterChar::SendBullet()
{
	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());

		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		FVector BeamEnd;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd);
		if (bBeamEnd)
		{


			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(), ImpactParticles, BeamEnd);
			}
			if (BeamParticles)
			{
				UParticleSystemComponent* Beam =
					UGameplayStatics::
					SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform.GetLocation());
				if (Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamEnd);
				}
			}
		}
	}
}
void AShooterChar::PlayGunFireMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}

void AShooterChar::ReloadButtonPressed()
{
	
	ReloadWeapon();
}

void AShooterChar::ReloadWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && ReloadMontage)
	{
		
		CombatState = ECombatState::ECS_Reloading;
		AnimInstance->Montage_Play(ReloadMontage);
		AnimInstance->Montage_JumpToSection(FName("ReloadSMG"), ReloadMontage);
	}
}

void AShooterChar::FinishReloading()
{
	CombatState = ECombatState::ECS_Unoccupied;
}
