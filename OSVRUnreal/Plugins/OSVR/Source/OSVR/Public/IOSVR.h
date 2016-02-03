// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModuleManager.h"
#include "IHeadMountedDisplayModule.h"

//#if OSVR_INPUTDEVICE_ENABLED
//#include "IInputDeviceModule.h"
//
///**
// * The public interface to this module.  In most cases, this interface is only public to sibling modules 
// * within this plugin.
// */
//class IOSVR : public IInputDeviceModule
//{
//
//public:
//	/**
//	 * Singleton-like access to this module's interface.  This is just for convenience!
//	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
//	 *
//	 * @return Returns singleton instance, loading the module on demand if needed
//	 */
//	static inline IOSVR& Get()
//	{
//		return FModuleManager::LoadModuleChecked< IOSVR >("OSVR");
//	}
//
//	/**
//	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
//	 *
//	 * @return True if the module is loaded and ready to use
//	 */
//	static inline bool IsAvailable()
//	{
//		return FModuleManager::Get().IsModuleLoaded("OSVR");
//	}
//};
//#endif // OSVR_INPUTDEVICE_ENABLED

class OSVREntryPoint;
class FOSVRHMD;

/**
* The public interface to this module.  In most cases, this interface is only public to sibling modules
* within this plugin.
*/
OSVR_API class IOSVR : public IHeadMountedDisplayModule
{
public:
    /** Returns the key into the HMDPluginPriority section of the config file for this module */
    virtual FString GetModulePriorityKeyName() const
    {
        return FString(TEXT("OSVR"));
    }

    /**
    * Singleton-like access to this module's interface.  This is just for convenience!
    * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
    *
    * @return Returns singleton instance, loading the module on demand if needed
    */
    static inline IOSVR& Get()
    {
        return FModuleManager::LoadModuleChecked< IOSVR >("OSVR");
    }

    /**
    * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
    *
    * @return True if the module is loaded and ready to use
    */
    static inline bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded("OSVR");
    }

    virtual void LoadOSVRClientKitModule() = 0;
    virtual OSVREntryPoint* GetEntryPoint() = 0;
    virtual TSharedPtr<FOSVRHMD, ESPMode::ThreadSafe> GetHMD() = 0;
};

DECLARE_LOG_CATEGORY_EXTERN(OSVRLog, Log, All);