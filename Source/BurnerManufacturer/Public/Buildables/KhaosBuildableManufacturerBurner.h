#pragma once

#include "CoreMinimal.h"
#include "Buildables/FGBuildableManufacturer.h"
#include "KhaosBuildableManufacturerBurner.generated.h"

/**
 * Base class for manufacturers that run on solid fuel.
 */
UCLASS()
class BURNERMANUFACTURER_API AKhaosBuildableManufacturerBurner : public AFGBuildableManufacturer
{
	GENERATED_BODY()

public:
	AKhaosBuildableManufacturerBurner();

	// Begin AActor interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	// End AACtor interface

	// Begin Factory_ interface
	virtual void Factory_Tick(float dt) override;
	// End Factory_ interface

	/**
	 * Check if a resource is valid as fuel for this manufacturer.
	 * @param Resource - Resource class to check.
	 * @return - true if resource valid as fuel; false if not valid.
	 */
	UFUNCTION(BlueprintPure, Category = "Power")
	bool IsValidFuel(TSubclassOf<UFGItemDescriptor> Resource) const;

	/**
	 * @return a valid pointer to the inventory if this machine runs on fuel. Can be nullptr on client.
	 */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE UFGInventoryComponent* GetFuelInventory() const { return mFuelInventory; };

	/**
	 * Check if this manufacturer has fuel.
	 * @return - true if this manufacturer has fuel; false if it has no fuel.
	 */
	UFUNCTION(BlueprintPure, Category = "Power")
	bool HasFuel() const;

	/** How much of the fuel have we burned? In range [0,1]. */
	UFUNCTION(BlueprintPure, Category = "Power")
	float GetFuelAmount() const;

	/** Returns the currently used fuel class */
	UFUNCTION(BlueprintPure, Category = "Power")
	FORCEINLINE TSubclassOf<UFGItemDescriptor> GetCurrentFuelClass() const { return mCurrentFuelClass; }

	/** Returns all fuel classes this manufacturer can run on. */
	UFUNCTION(BlueprintCallable, Category = "Power")
	FORCEINLINE TArray<TSoftClassPtr<UFGItemDescriptor>> GetDefaultFuelClasses() const { return mDefaultFuelClasses; }

	FORCEINLINE int32 GetFuelInventoryIndex() const { return mFuelInventoryIndex; }

protected:
	// Begin AFGBuildableFactory interface
	virtual bool CanProduce_Implementation() const override;
	virtual bool Factory_HasPower() const override;
	virtual EProductionStatus GetProductionIndicatorStatus() const override;
	virtual void Factory_TickProducing(float dt) override;
	// End AFGBuildableFactory interface

	/** Try to collect fuel from an fuel input. */
	void Factory_CollectFuel(float dt);

	/** Try load fuel into the burner. */
	virtual void LoadFuel();

	/** Can we load fuel in to the generator. Only call this on server. */
	virtual bool CanLoadFuel() const;

	/** Fuel classes this machine can run on. */
	UPROPERTY(EditDefaultsOnly, Category = "Power", meta = (MustImplement = "FGInventoryInterface"))
	TArray<TSoftClassPtr<UFGItemDescriptor>> mDefaultFuelClasses;

	/**
	 * The quantity of inventory to be loaded for use during production.
	 * Any quantity less than this will fail to load and halt production.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Power")
	int32 mFuelLoadAmount;

	/** Inventory where fuel is loaded into. */
	UPROPERTY(SaveGame)
	TObjectPtr<UFGInventoryComponent> mFuelInventory;

	/** Input inventory index to store the fuel */
	int32 mFuelInventoryIndex;

	/** Amount left of the currently burned piece of fuel. In megawatt seconds (MWs). */
	UPROPERTY(SaveGame, Replicated, Meta = (NoAutoJson = true))
	float mCurrentFuelAmount;

	/** Used so clients know how if they have available fuel or not. Could be removed later if the base game starts syncing the production indicator state */
	UPROPERTY(SaveGame, Replicated, Meta = (NoAutoJson = true))
	mutable bool mHasFuelCached;

	/** Type of the currently burned piece of fuel. */
	UPROPERTY(SaveGame, Replicated, Meta = (NoAutoJson = true))
	TSubclassOf<UFGItemDescriptor> mCurrentFuelClass;
};
