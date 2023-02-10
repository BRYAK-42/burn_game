// Fill out your copyright notice in the Description page of Project Settings.
#include "Weapon.h"
AWeapon::AWeapon()
{
	ThrowWeaponTime = 1.5f;
	bIsFalling = false;
	PrimaryActorTick.bCanEverTick = true;
	Ammo = 30;
	MagazineCapacity = 30;
	WeaponType = EWeaponType::EWT_SubmachineGun;
	AmmoType = EAmmoTypes::EAT_9mm;
	ReloadMontageSection = FName(TEXT("ReloadSMG"));
	bMovingClip = false;
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetItemState()==EItemState::EIS_Falling && bIsFalling)
	{
		const FRotator MeshRotation = FRotator(0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f);
		GetItemMesh()->SetWorldRotation(MeshRotation,false,nullptr,ETeleportType::TeleportPhysics);
	}

}

void AWeapon::TrowWeapon()
{
	SetItemState(EItemState::EIS_Falling);
	FRotator MeshRotation = FRotator(0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f);
	GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
	FVector MeshForward = GetItemMesh()->GetForwardVector();
	FVector const MeshRight = GetItemMesh()->GetRightVector();

	FVector MeshImpulse = MeshRight.RotateAngleAxis(-20, MeshForward);
	MeshImpulse = MeshImpulse.RotateAngleAxis(30.f, FVector(0,0,1));
	MeshImpulse *= 2000;
	GetItemMesh()->AddImpulse(MeshImpulse);

	GetWorldTimerManager().SetTimer(ThrowWeaponTimer, this, &AWeapon::StopFalling, ThrowWeaponTime);
}
void AWeapon::DecrementAmmo()
{
	if (Ammo - 1<=0){Ammo = 0;}
	else{--Ammo;}
}
void AWeapon::ReloadAmmo(int Amount)
{
	checkf(Ammo += Amount <= MagazineCapacity, 
		TEXT("Attemptet to reload more than magazine capacity"));
	Ammo += Amount;
}
void AWeapon::StopFalling()
{
	bIsFalling = false;
	SetItemState(EItemState::EIS_PickUp);
}