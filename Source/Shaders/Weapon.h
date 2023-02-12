
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponProperties", meta = (AllowPrivateAccess = "true"))
	bool bMovingClip;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FName ClipBoneName;

public:
	void TrowWeapon();
	FORCEINLINE int GetAmmo() { return Ammo; };
	void DecrementAmmo();
	FORCEINLINE EWeaponType GetWeaponType()const { return WeaponType; };
	FORCEINLINE EAmmoTypes GetAmmoType()const { return AmmoType; };
	FORCEINLINE int GetMagazineCapacity()const { return MagazineCapacity; };
	FORCEINLINE FName GetReloadMontageSection()const { return ReloadMontageSection; };
	FORCEINLINE FName GetClipBoneName() const { return ClipBoneName; };
	FORCEINLINE void SetMovingClip(bool Move) { bMovingClip = Move; };
	void ReloadAmmo(int Amount);
};