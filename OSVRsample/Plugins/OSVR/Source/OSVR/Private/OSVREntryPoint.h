
#pragma once

#include "OSVRInterfaceCollection.h"

class OSVREntryPoint : public FTickableGameObject
{
public:

	OSVREntryPoint();
	virtual ~OSVREntryPoint();

	virtual void Tick(float DeltaTime) override;

	virtual bool IsTickable() const override { return (!GWorld->HasBegunPlay() && GIsEditor) ? false : true; }
	virtual bool IsTickableWhenPaused() const override { return true; }
	virtual bool IsTickableInEditor() const override { return false; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(OSVREntryPoint, STATGROUP_Tickables); }

	OSVRInterfaceCollection* GetInterfaceCollection();

private:

	TSharedPtr<OSVRInterfaceCollection> InterfaceCollection;
};
