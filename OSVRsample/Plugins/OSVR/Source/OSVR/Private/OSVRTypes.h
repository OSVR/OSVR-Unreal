//
// Copyright 2014, 2015 Razer Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//


#pragma once

#if OSVR_ENABLED

FORCEINLINE FVector OSVR2FVector(const OSVR_Vec3& Vec3)
{
	// OSVR: The coordinate system is right-handed, with X to the right, Y up, and Z near.
	return FVector(-float(Vec3.data[2]), float(Vec3.data[0]), float(Vec3.data[1]));
}

FORCEINLINE FQuat OSVR2FQuat(const OSVR_Quaternion& Quat)
{
	//UE_LOG(LogActor, Warning, TEXT("X=%3.3f Y=%3.3f Z=%3.3f W=%3.3f"), osvrQuatGetX(&Quat), osvrQuatGetY(&Quat), osvrQuatGetZ(&Quat), osvrQuatGetW(&Quat));
	FQuat q = FQuat(-osvrQuatGetZ(&Quat), osvrQuatGetX(&Quat), osvrQuatGetY(&Quat), -osvrQuatGetW(&Quat));
	// Hydra is 15 degrees twisted.
	//q = q * FQuat(FVector(0, 1, 0), -15 * PI / 180);
	return q;
}

#endif // OSVR_ENABLED
