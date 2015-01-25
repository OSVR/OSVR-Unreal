/** @file
    @brief Header

    Must be c-safe!

    @date 2014

    @author
    Ryan Pavlik
    <ryan@sensics.com>
    <http://sensics.com>
*/

/*
// Copyright 2014 Sensics, Inc.
//
// All rights reserved.
//
// (Final version intended to be licensed under
// the Apache License, Version 2.0)
*/

#ifndef INCLUDED_ClientCallbackTypesC_h_GUID_4D43A675_C8A4_4BBF_516F_59E6C785E4EF
#define INCLUDED_ClientCallbackTypesC_h_GUID_4D43A675_C8A4_4BBF_516F_59E6C785E4EF

/* Internal Includes */
#include <osvr/Util/ClientReportTypesC.h>
#include <osvr/Util/ReturnCodesC.h>
#include <osvr/Util/TimeValueC.h>

/* Library/third-party includes */
/* none */

/* Standard includes */
/* none */

OSVR_EXTERN_C_BEGIN

/** @addtogroup ClientKit
    @{
*/

/** @name Report callback types
    @{
*/

/* generated file - do not edit! */
/** @brief C function type for a Position callback */
typedef void (*OSVR_PositionCallback)(void *userdata,
                                     const struct OSVR_TimeValue *timestamp,
                                     const struct OSVR_PositionReport *report);
/** @brief C function type for a Orientation callback */
typedef void (*OSVR_OrientationCallback)(void *userdata,
                                     const struct OSVR_TimeValue *timestamp,
                                     const struct OSVR_OrientationReport *report);
/** @brief C function type for a Pose callback */
typedef void (*OSVR_PoseCallback)(void *userdata,
                                     const struct OSVR_TimeValue *timestamp,
                                     const struct OSVR_PoseReport *report);
/** @brief C function type for a Button callback */
typedef void (*OSVR_ButtonCallback)(void *userdata,
                                     const struct OSVR_TimeValue *timestamp,
                                     const struct OSVR_ButtonReport *report);
/** @brief C function type for a Analog callback */
typedef void (*OSVR_AnalogCallback)(void *userdata,
                                     const struct OSVR_TimeValue *timestamp,
                                     const struct OSVR_AnalogReport *report);

/** @} */

/** @} */

OSVR_EXTERN_C_END

#endif
