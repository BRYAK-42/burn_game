
#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_SubmachineGun UMETA(DisplayName = "SubmachineGun"),
	EWT_AssautRifle UMETA(DisplayName = "AssautRifle"),
	EWT_Max			UMETA(DisplayName = "DefaultMax")
};

UCLASS()
class SHADERS_API AWeapon : public AItem
{
	GENERATED_BODY()
public:
	AWeapon();
	virtual void Tick(float DeltaTime) override;
protected:
	void StopFalling();
private:
	FTimerHandle ThrowWeaponTimer;
	float ThrowWeaponTime;
	bool bIsFalling;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties", meta = (AllowPrivateAccess = "true"))
	int Ammo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties", meta = (AllowPrivateAccess = "true"))
	int MagazineCapacity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties", meta = (AllowPrivateAccess = "true"))
	EWeaponType WeaponType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties", meta = (AllowPrivateAccess = "true"))
	EAmmoTypes AmmoType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties", meta = (AllowPrivateAccess = "true"))
	FName ReloadMontageSection;

	bool bMovingClip;

public:
	void TrowWeapon();
	FORCEINLINE int GetAmmo() { return Ammo; };
	void DecrementAmmo();
	FORCEINLINE EWeaponType GetWeaponType()const { return WeaponType; };
	FORCEINLINE EAmmoTypes GetAmmoType()const { return AmmoType; };
	FORCEINLINE int GetMagazineCapacity()const { return MagazineCapacity; };
	FORCEINLINE FName GetReloadMontageSection()const { return ReloadMontageSection; };

	void ReloadAmmo(int Amount);
};