// Fill out your copyright notice in the Description page of Project Settings.

#include "Item.h"
#include "ShooterChar.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
AItem::AItem()
{
	
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Item Mesh");
	SetRootComponent(ItemMesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>("Collision Box");
	CollisionBox->SetupAttachment(ItemMesh);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	PickUpWidget = CreateDefaultSubobject<UWidgetComponent>("Pick Up Widget");
	PickUpWidget->SetupAttachment(GetRootComponent());

	MyAreaSphere = CreateDefaultSubobject<USphereComponent>("My Area Sphere");
	MyAreaSphere->SetupAttachment(RootComponent);

	ItemName = "Default";
	ItemCount = 0;
	ItemRarity = EItemRarity::EIR_Common;
	ItemState = EItemState::EIS_PickUp;
	
	ZCurveTime = 0.7;
	ItemInterpStarsLoc = FVector(0);
	CemeraTargetLoc = FVector(0);
	bIsInterping = false;
	ItemInterpX = ItemInterpY = 0.f;

}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();
	SetActiveStars();
	if (PickUpWidget)
	{
		PickUpWidget->SetVisibility(false);
	}
	//Setup Overlap
	MyAreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereBeginOverlap);
	MyAreaSphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);

	SetItemProperties(ItemState);
}


// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ItemInterp(DeltaTime);
}


void AItem::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
	if (OtherActor)
	{
		AShooterChar* ShooterCharacter = Cast<AShooterChar>(OtherActor);
		if (ShooterCharacter)
		{
			ShooterCharacter->IncrementOverlappedItemCount(1);
		}
	}
}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OthereComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AShooterChar* ShooterCharacter = Cast<AShooterChar>(OtherActor);
		if (ShooterCharacter)
		{
			ShooterCharacter->IncrementOverlappedItemCount(-1);
		}
	}
}

void AItem::SetActiveStars()
{
	for (int i = 0; i <= 5; i++)
	{
		ActiveStars.Add(false);
	}
	switch (ItemRarity)
	{
	case EItemRarity::EIR_Legendary:
		ActiveStars[5] = true;
	case EItemRarity::EIR_Rare:
		ActiveStars[4] = true;
	case EItemRarity::EIR_Uncommon:
		ActiveStars[3] = true;
	case EItemRarity::EIR_Common:
		ActiveStars[2] = true;
	case EItemRarity::EIR_Damaged:
		ActiveStars[1] = true;
		break;
	case EItemRarity::EIR_Max:

		break;
	default:
		break;
	}

}

void AItem::SetItemProperties(EItemState State)
{
	switch (State)
	{
	case EItemState::EIS_PickUp:

		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		MyAreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		MyAreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility,ECR_Block);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		break;
	case EItemState::EIS_EquipInterping:

		PickUpWidget->SetVisibility(false);

		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);

		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		MyAreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		MyAreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);


		break;
	case EItemState::EIS_PickedUp:

		break;
	case EItemState::EIS_Equiped:

		PickUpWidget->SetVisibility(false);

		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);

		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		MyAreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		MyAreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		

		break;
	case EItemState::EIS_Falling:

		ItemMesh->SetEnableGravity(true);
		ItemMesh->SetSimulatePhysics(true);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionResponseToChannel(
			ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);

		MyAreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		MyAreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);



		break;
	case EItemState::EIS_Max:

		break;
	default:
		break;
	}
}

void AItem::FinishInterping()
{
	bIsInterping = false;
	if (ShooterCharRef)
	{
		ShooterCharRef->GetPickUpItem(this);
	}
}

void AItem::ItemInterp(float DeltaTime)
{
	if (!bIsInterping) return;
	if (ShooterCharRef && ItemZCurve)
	{
		const float ElapsedTime = GetWorldTimerManager().GetTimerElapsed(ItemInterpTimer);
		const float CurveValue = ItemZCurve->GetFloatValue(ElapsedTime);
		FVector ItemLocation = ItemInterpStarsLoc;
		const FVector CameraInterpLocation = ShooterCharRef->GetCameraInterpLocation();
	
		const FVector ItemToCamera = FVector(0, 0, (CameraInterpLocation - ItemLocation).Z);
		const float DeltaZ = ItemToCamera.Size();
		const FVector CurrentLocation = GetActorLocation();


		const float InterpX = FMath::FInterpTo(CurrentLocation.X, CameraInterpLocation.X, DeltaTime, 30.f);
		const float InterpY = FMath::FInterpTo(CurrentLocation.Y, CameraInterpLocation.Y, DeltaTime, 30.f);

		ItemLocation.Z += CurveValue * DeltaZ;
		ItemLocation.X = InterpX;
		ItemLocation.Y = InterpY;

		
		SetActorLocation(ItemLocation, true, nullptr, ETeleportType::TeleportPhysics);


	}
}

void AItem::SetItemState(EItemState State)
{
	ItemState = State;
	SetItemProperties(State);
}

void AItem::StartItemCurve(AShooterChar* Char)
{
	ShooterCharRef = Char;
	ItemInterpStarsLoc = GetActorLocation();
	bIsInterping = true;
	SetItemState(EItemState::EIS_EquipInterping);
	GetWorldTimerManager().SetTimer(ItemInterpTimer, this, &AItem::FinishInterping, ZCurveTime);
}
