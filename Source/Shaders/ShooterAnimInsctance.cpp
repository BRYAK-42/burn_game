// Fill out your copyright notice in the Description page of Project Settings.
#include "ShooterAnimInsctance.h"
#include"ShooterChar.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

UShooterAnimInsctance::UShooterAnimInsctance() 
{
	Speed = 0;
	bIsInAir = false;
	bIsAccelerating = false;
	bAiming = false;
	MovementOffsetYaw = 0;
	LastMovementOffsetYaw = 0; 
	CharacterYaw = 0;
	CharacterYawLastFrame = 0;
	RootYawOffset = 0;
}

void UShooterAnimInsctance::UpdateAnimationProperties(float DelatTime)
{
	if (ShooterCharacter != nullptr)
	{
		ShooterCharacter = Cast<AShooterChar>(TryGetPawnOwner());
	}
	if (ShooterCharacter)
	{
		FVector Velocity = ShooterCharacter->GetVelocity();
		Velocity.Z = 0;
		Speed = Velocity.Size();
		bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();
		if (ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0)
		{
			bIsAccelerating = true;
		}
		else {bIsAccelerating = false;}

		FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());

		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
		if (ShooterCharacter->GetVelocity().Size() > 0.f)
		{
			LastMovementOffsetYaw = MovementOffsetYaw;
		}
		bAiming = ShooterCharacter->GetAimState();
	}
	TurnInPlace();
}

void UShooterAnimInsctance::NativeInitializeAnimation()
{
	ShooterCharacter = Cast<AShooterChar>(TryGetPawnOwner());
}

void UShooterAnimInsctance::TurnInPlace()
{
	if (ShooterCharacter == nullptr) return;
	if (Speed < 0)
	{
		CharacterYawLastFrame = CharacterYaw;
		CharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		const float YawDelta = CharacterYaw - CharacterYawLastFrame;

		RootYawOffset -= YawDelta;
	}

}
