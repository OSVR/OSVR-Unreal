// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

//#include "IOSVR.h"

// You should place include statements to your module's private header files here.  You only need to
// add includes for headers that are used in most of your module's source files though.

#include "Core.h"
#include "CoreUObject.h"

#include "Engine.h"

#define OSVR_INPUTDEVICE_ENABLED 1

// The original OSVR Blueprint API should be considered deprecated,
// but if you're still using it, copy the code from /Archive back
// into the project, and set this to 1. Keep in mind the Blueprint
// API is scheduled for a redesign soon.
#define OSVR_DEPRECATED_BLUEPRINT_API_ENABLED 0

#include <osvr/ClientKit/ContextC.h>
#include <osvr/ClientKit/InterfaceC.h>
#include <osvr/ClientKit/InterfaceCallbackC.h>
#include <osvr/ClientKit/InterfaceStateC.h>
#include <osvr/ClientKit/ParametersC.h>
