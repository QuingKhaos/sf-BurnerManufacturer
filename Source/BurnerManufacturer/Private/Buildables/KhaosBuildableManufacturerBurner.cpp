#include "Buildables/KhaosBuildableManufacturerBurner.h"
#include "BPFL/KhaosNameBPFL.h"
#include "Net/UnrealNetwork.h"
#include "BurnerManufacturerLogChannels.h"
#include "FGCheatManager.h"
#include "FGFactoryConnectionComponent.h"
#include "FGGameState.h"

AKhaosBuildableManufacturerBurner::AKhaosBuildableManufacturerBurner()
	:Super(), mFuelLoadAmount(1), mFuelInventoryIndex(0)
{
	mCanChangePotential = true;
	mRunsOnPowerOverride = true;

	mFuelInventory = CreateDefaultSubobject<UFGInventoryComponent>(TEXT("FuelInventory"));
}

void AKhaosBuildableManufacturerBurner::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AKhaosBuildableManufacturerBurner, mCurrentFuelAmount);
	DOREPLIFETIME(AKhaosBuildableManufacturerBurner, mHasFuelCached);
	DOREPLIFETIME(AKhaosBuildableManufacturerBurner, mCurrentFuelClass);
}

void AKhaosBuildableManufacturerBurner::BeginPlay()
{
	mFuelInventory->SetDefaultSize(mFuelInventoryIndex + 1);
	if (mFuelInventory->IsValidIndex(mFuelInventoryIndex))
	{
		mFuelInventory->SetReplicationRelevancyOwner(this);
	}

	Super::BeginPlay();
}

void AKhaosBuildableManufacturerBurner::Factory_Tick(float dt)
{
	if (!HasAuthority())
	{
		return;
	}

	Factory_CollectFuel(dt);

	Super::Factory_Tick(dt);
}

bool AKhaosBuildableManufacturerBurner::IsValidFuel(TSubclassOf<UFGItemDescriptor> Resource) const
{
	for (const TSoftClassPtr<UFGItemDescriptor> SoftFuelClass : mDefaultFuelClasses)
	{
		if (SoftFuelClass.IsValid())
		{
			if (TSubclassOf<UFGItemDescriptor> FuelClass = SoftFuelClass.Get())
			{
				if (Resource == FuelClass)
				{
					return UFGItemDescriptor::GetEnergyValue(FuelClass) > 0.f;
				}
			}
		}
	}

	return false;
}

bool AKhaosBuildableManufacturerBurner::HasFuel() const
{
	if (!HasAuthority())
	{
		return mHasFuelCached;
	}

	if (mFuelInventoryIndex < 0)
	{
		return false;
	}

	if (mCurrentFuelAmount > 0.f)
	{
		mHasFuelCached = true;
		return true;
	}

	if (CanLoadFuel())
	{
		FInventoryStack OutStack;
		mFuelInventory->GetStackFromIndex(mFuelInventoryIndex, OutStack);
		if (OutStack.NumItems > 0)
		{
			if (TSubclassOf<UFGItemDescriptor> FuelItemClass = OutStack.Item.GetItemClass())
			{
				if (OutStack.NumItems >= mFuelLoadAmount)
				{
					mHasFuelCached = true;
					return true;
				}
			}
		}
	}

	return false;
}

float AKhaosBuildableManufacturerBurner::GetFuelAmount() const
{
	if (mCurrentFuelClass)
	{
		float EnergyValue = UFGItemDescriptor::GetEnergyValue(mCurrentFuelClass);
		float MaxFuelAmount = EnergyValue * mFuelLoadAmount;
		return mCurrentFuelAmount / MaxFuelAmount;
	}

	return 0.f;
}

bool AKhaosBuildableManufacturerBurner::CanProduce_Implementation() const
{
	bool CanProduce = Super::CanProduce_Implementation() && (HasPower());

	return CanProduce;
}

bool AKhaosBuildableManufacturerBurner::Factory_HasPower() const
{
	if (GetWorld()->GetGameState<AFGGameState>()->GetCheatNoPower())
	{
		return true;
	}

	bool HasPower = HasFuel();

	return HasPower;
}

EProductionStatus AKhaosBuildableManufacturerBurner::GetProductionIndicatorStatus() const
{
	if (!Factory_IsProducing())
	{
		return HasPower() ? EProductionStatus::IS_STANDBY : EProductionStatus::IS_ERROR;
	}

	if (UFGInventoryComponent* PotentialInventory = GetPotentialInventory())
	{
		if (PotentialInventory->GetNumItems(nullptr) > 0)
		{
			return EProductionStatus::IS_PRODUCING_WITH_CRYSTAL;
		}
	}

	return EProductionStatus::IS_PRODUCING;
}

void AKhaosBuildableManufacturerBurner::Factory_TickProducing(float dt)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!mFuelInventory)
	{
		return;
	}

	if (GetWorld()->GetGameState<AFGGameState>()->GetCheatNoPower())
	{
		Super::Factory_TickProducing(dt);
		return;
	}

	float ProcessedFuelAmount = 0.f;
	float FuelConsumption = GetProducingPowerConsumption() * dt;
	float RemainingFuelConsumption = FuelConsumption;

	do
	{
		if (mCurrentFuelAmount > RemainingFuelConsumption)
		{
			mCurrentFuelAmount -= RemainingFuelConsumption;
			ProcessedFuelAmount += RemainingFuelConsumption;
			RemainingFuelConsumption = 0.f;
			break;
		}

		if (mCurrentFuelAmount > 0.f)
		{
			RemainingFuelConsumption -= mCurrentFuelAmount;
			ProcessedFuelAmount += mCurrentFuelAmount;
			mCurrentFuelAmount = 0.f;
		}

		if (mCurrentFuelAmount <= 0.f)
		{
			LoadFuel();

			if (mCurrentFuelAmount <= 0.f)
			{
				break;
			}
		}
	}
	while (RemainingFuelConsumption > 0.f);

	if (ProcessedFuelAmount > 0.f)
	{
		// Reduce dt by the relation of processed fuel to total fuel consumption, so that production is restricted by the amount of fuel we processed.
		float FuelConsumptionRatio = ProcessedFuelAmount / FuelConsumption;
		dt *= FuelConsumptionRatio;

		Super::Factory_TickProducing(dt);
	}
}

void AKhaosBuildableManufacturerBurner::Factory_CollectFuel(float dt)
{
	if (!HasAuthority() || mDefaultFuelClasses.Num() <= 0 || mFuelInventoryIndex < 0)
	{
		return;
	}

	for (UFGFactoryConnectionComponent* InputConnection : mFactoryInputConnections)
	{
		if (UKhaosNameBPFL::StartsWith(InputConnection->GetFName(), FName("FuelInput"), ESearchCase::CaseSensitive))
		{
			TArray<FInventoryItem> OutItems;
			InputConnection->Factory_PeekOutput(OutItems);

			for (const FInventoryItem& Item : OutItems)
			{
				if (TSubclassOf<UFGItemDescriptor> FuelClass = Item.GetItemClass())
				{
					if (!IsValidFuel(FuelClass))
					{
						break;
					}

					if (!mFuelInventory)
					{
						return;
					}

					FInventoryStack OutStack;
					mFuelInventory->GetStackFromIndex(mFuelInventoryIndex, OutStack);

					if (TSubclassOf<UFGItemDescriptor> CurrentFuelClass = OutStack.Item.GetItemClass())
					{
						int32 FuelStackSize = 0;

						if (CurrentFuelClass == FuelClass)
						{
							FuelStackSize = UFGItemDescriptor::GetStackSize(FuelClass);
						}

						if (CurrentFuelClass != FuelClass || OutStack.NumItems >= FuelStackSize)
						{
							break;
						}
					}

					FInventoryItem OutItem;
					float OutOffsetBeyond;
					InputConnection->Factory_GrabOutput(OutItem, OutOffsetBeyond);
					TSubclassOf<UFGItemDescriptor> OutItemClass = OutItem.GetItemClass();

					FInventoryStack InStack(1, OutItemClass);
					int32 NumAdded = mFuelInventory->AddStackToIndex(mFuelInventoryIndex, InStack);

					if (NumAdded == 1)
					{
						return;
					}
				}
			}
		}
	}
}

void AKhaosBuildableManufacturerBurner::LoadFuel()
{
	if (!CanLoadFuel())
	{
		return;
	}

	FInventoryStack OutStack;
	mFuelInventory->GetStackFromIndex(mFuelInventoryIndex, OutStack);

	if (OutStack.NumItems > 0)
	{
		if (TSubclassOf<UFGItemDescriptor> FuelItemClass = OutStack.Item.GetItemClass())
		{
			if (OutStack.NumItems >= mFuelLoadAmount)
			{
				mFuelInventory->RemoveFromIndex(mFuelInventoryIndex, mFuelLoadAmount);
				mCurrentFuelClass = FuelItemClass;

				float EnergyValue = UFGItemDescriptor::GetEnergyValue(FuelItemClass);
				mCurrentFuelAmount = EnergyValue * mFuelLoadAmount;
			}
		}
	}
}

bool AKhaosBuildableManufacturerBurner::CanLoadFuel() const
{
	return HasAuthority() && mFuelInventory && mFuelInventory->IsValidIndex(mFuelInventoryIndex);
}
