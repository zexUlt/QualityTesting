// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include "WaveVRInput.h"
#include "Engine/Engine.h"
#include "Features/IModularFeatures.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

#include "WaveVREventCommon.h"
// Private includes
#include "Platforms/Editor/WaveVRDirectPreview.h"
#include "Platforms/WaveVRLogWrapper.h"
#include "PoseManagerImp.h"
#include "WaveVRUtils.h"

using namespace wvr::utils;

DEFINE_LOG_CATEGORY_STATIC(LogWaveVRInput, Log, All);

#pragma region
static bool IsPlayInEditor()
{
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.World()->IsPlayInEditor())
		{
			return true;
		}
	}
	return false;
}

float QuaternionLengthSquared(FQuat q)
{
	return
		q.W * q.W +
		q.X * q.X +
		q.Y * q.Y +
		q.Z * q.Z;
}

FVector QuaternionRotate(FQuat& q, const FVector v)
{
	const float w = q.W;
	const float x = q.X;
	const float y = q.Y;
	const float z = q.Z;

	const float kTwo = 2.0f;
	float vcoeff = kTwo * w * w - 1.0f;
	float ucoeff = kTwo * (x * v.X + y * v.Y + z * v.Z);
	float ccoeff = kTwo * w;

	float vx = vcoeff * v.X + ucoeff * x + ccoeff * (y * v.Z - z * v.Y);
	float vy = vcoeff * v.Y + ucoeff * y + ccoeff * (z * v.X - x * v.Z);
	float vz = vcoeff * v.Z + ucoeff * z + ccoeff * (x * v.Y - y * v.X);

	return FVector(vx, vy, vz);
}

float QuaternionDot(FQuat q1, FQuat q2)
{
	return q1.W * q2.W + q1.X * q2.X + q1.Y * q2.Y + q1.Z * q2.Z;
}

float QuaternionAngle(FQuat q1, FQuat q2)
{
	float f = QuaternionDot(q1, q2);
	return FMath::Acos(FMath::Min(FMath::Abs(f), 1.0f)) * 2.0f * 57.29578f;
}

FQuat FromToRotation(const FVector& from_direction, const FVector& to_direction)
{
	float dot = FVector::DotProduct(from_direction, to_direction);
	float squareFrom = FMath::Square(from_direction.X) + FMath::Square(from_direction.Y) + FMath::Square(from_direction.Z);
	float squareTo = FMath::Square(to_direction.X) + FMath::Square(to_direction.Y) + FMath::Square(to_direction.Z);
	float norm = sqrt(squareFrom * squareTo);
	float real = norm + dot;

	FVector w = FVector::ZeroVector;
	if (real < 1.e-6f * norm)
	{
		real = 0.0f;
		w = fabsf(from_direction.X) > fabsf(from_direction.Z) ?
			FVector(-from_direction.Y, from_direction.X, 0.0f) : FVector(0.0f, -from_direction.Z, from_direction.Y);
	}
	else
	{
		w = FVector(
			from_direction.Y * to_direction.Z - from_direction.Z * to_direction.Y,
			from_direction.Z * to_direction.X - from_direction.X * to_direction.Z,
			from_direction.X * to_direction.Y - from_direction.Y * to_direction.X);
	}

	FQuat result = FQuat(w.X, w.Y, w.Z, real);
	result.Normalize();

	return result;
}

FVector VectorSlerp(FVector start, FVector end, float filter) {
	// Make sure both start and end are normalized.
	start.Normalize();
	end.Normalize();
	float dot = FVector::DotProduct(start, end);
	dot = FMath::Clamp(dot, -1.0f, 1.0f);
	float theta = FMath::Acos(dot) * filter;
	FVector relative_vector = end - start * dot;
	relative_vector.Normalize();
	return ((start * FMath::Cos(theta)) + (relative_vector * FMath::Sin(theta)));
}

FQuat QuaternionSlerp(FQuat a, FQuat b, float t)
{
	t = FMath::Clamp<float>(t, 0, 1);

	if (QuaternionLengthSquared(a) == 0 && QuaternionLengthSquared(b) == 0)
		return FQuat::Identity;
	if (QuaternionLengthSquared(a) == 0)
		return b;
	if (QuaternionLengthSquared(b) == 0)
		return a;

	FVector va = FVector(a.X, a.Y, a.Z);
	FVector vb = FVector(b.X, b.Y, b.Z);
	float cosHalfAngle = a.W * b.W + FVector::DotProduct(va, vb);

	if (cosHalfAngle >= 1.0f || cosHalfAngle <= -1.0f)
	{
		// angle = 0.0f, so just return one input.
		return a;
	}
	else if (cosHalfAngle < 0.0f)
	{
		vb = -vb;
		b.W = -b.W;
		cosHalfAngle = -cosHalfAngle;
	}

	float blendA;
	float blendB;
	if (cosHalfAngle < 0.99f)
	{
		// do proper slerp for big angles
		float halfAngle = FMath::Acos(cosHalfAngle);
		float sinHalfAngle = FMath::Sin(halfAngle);
		float oneOverSinHalfAngle = 1.0f / sinHalfAngle;
		blendA = FMath::Sin(halfAngle * (1.0f - t)) * oneOverSinHalfAngle;
		blendB = FMath::Sin(halfAngle * t) * oneOverSinHalfAngle;
	}
	else
	{
		// do lerp if angle is really small.
		blendA = 1.0f - t;
		blendB = t;
	}

	FQuat result = FQuat(blendA * va + blendB * vb, blendA * a.W + blendB * b.W);
	if (QuaternionLengthSquared(result) > 0.0f)
	{
		result.Normalize();
		return result;
	}
	else
	{
		return FQuat::Identity;
	}
}

FQuat QuaternionLerp(FQuat a, FQuat b, float t)
{
	FQuat q = FQuat::Identity;
	t = FMath::Clamp<float>(t, 0, 1);

	float last = 1.0f - t;
	float dot = QuaternionDot(a, b);
	if (dot >= 0)
	{
		q.X = (last * a.X) + (t * b.X);
		q.Y = (last * a.Y) + (t * b.Y);
		q.Z = (last * a.Z) + (t * b.Z);
		q.W = (last * a.W) + (t * b.W);
	}
	else
	{
		q.X = (last * a.X) - (t * b.X);
		q.Y = (last * a.Y) - (t * b.Y);
		q.Z = (last * a.Z) - (t * b.Z);
		q.W = (last * a.W) - (t * b.W);
	}
	float squared = QuaternionLengthSquared(q);
	float deno = 1 / FMath::Sqrt(squared);
	q.X *= deno;
	q.Y *= deno;
	q.Z *= deno;
	q.W *= deno;
	return q;
}
#pragma endregion non-class function

void FWaveVRInput::AddKeys() {
	// HmdPressButton[0]: Back
	LOGD(LogWaveVRInput, "Add key: Wave (H) Back Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Hmd_Back, LOCTEXT("Hmd_Back", "Wave (H) Back Press"), FKeyDetails::GamepadKey, "Wave"));
	// HmdPressButton[1]: Enter
	LOGD(LogWaveVRInput, "Add key: Wave (H) Enter Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Hmd_Enter, LOCTEXT("Hmd_Enter", "Wave (H) Enter Press"), FKeyDetails::GamepadKey, "Wave"));

	// ----------------- Press Buttons --------------------
	// InputButton[0]: Menu
	LOGD(LogWaveVRInput, "Add key: Wave (L) Menu Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Menu, LOCTEXT("Left_Menu", "Wave (L) Menu Press"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) Menu Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Menu, LOCTEXT("Right_Menu", "Wave (R) Menu Press"), FKeyDetails::GamepadKey, "Wave"));
	// InputButton[1]: Grip
	LOGD(LogWaveVRInput, "Add key: Wave (L) Grip Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Grip, LOCTEXT("Left_Grip", "Wave (L) Grip Press"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) Grip Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Grip, LOCTEXT("Right_Grip", "Wave (R) Grip Press"), FKeyDetails::GamepadKey, "Wave"));
	// InputButton[2]: DPad_Left
	LOGD(LogWaveVRInput, "Add key: Wave (L) DPadLeft Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_DPadLeft, LOCTEXT("Left_DPadLeft", "Wave (L) DPadLeft Press"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) DPadLeft Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_DPadLeft, LOCTEXT("Right_DPadLeft", "Wave (R) DPadLeft Press"), FKeyDetails::GamepadKey, "Wave"));
	// InputButton[3]: DPad_Up
	LOGD(LogWaveVRInput, "Add key: Wave (L) DPadUp Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_DPadUp, LOCTEXT("Left_DPadUp", "Wave (L) DPadUp Press"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) DPadUp Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_DPadUp, LOCTEXT("Right_DPadUp", "Wave (R) DPadUp Press"), FKeyDetails::GamepadKey, "Wave"));
	// InputButton[4]: DPad_Right
	LOGD(LogWaveVRInput, "Add key: Wave (L) DPadRight Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_DPadRight, LOCTEXT("Left_DPadRight", "Wave (L) DPadRight Press"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) DPadRight Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_DPadRight, LOCTEXT("Right_DPadRight", "Wave (R) DPadRight Press"), FKeyDetails::GamepadKey, "Wave"));
	// InputButton[5]: DPad_Down
	LOGD(LogWaveVRInput, "Add key: Wave (L) DPadDown Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_DPadDown, LOCTEXT("Left_DPadDown", "Wave (L) DPadDown Press"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) DPadDown Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_DPadDown, LOCTEXT("Right_DPadDown", "Wave (R) DPadDown Press"), FKeyDetails::GamepadKey, "Wave"));
	// InputButton[6]: Volume_Up
	LOGD(LogWaveVRInput, "Add key: Wave (L) VolumeUp Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_VolumeUp, LOCTEXT("Left_VolumeUp", "Wave (L) VolumeUp Press"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) VolumeUp Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_VolumeUp, LOCTEXT("Right_VolumeUp", "Wave (R) VolumeUp Press"), FKeyDetails::GamepadKey, "Wave"));
	// InputButton[7]: Volume_Down
	LOGD(LogWaveVRInput, "Add key: Wave (L) VolumeDown Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_VolumeDown, LOCTEXT("Left_VolumeDown", "Wave (L) VolumeDown Press"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) VolumeDown Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_VolumeDown, LOCTEXT("Right_VolumeDown", "Wave (R) VolumeDown Press"), FKeyDetails::GamepadKey, "Wave"));
	// InputButton[8]: Bumper
	LOGD(LogWaveVRInput, "Add key: Wave (L) Bumper Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Bumper, LOCTEXT("Left_Bumper", "Wave (L) Bumper Press"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) Bumper Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Bumper, LOCTEXT("Right_Bumper", "Wave (R) Bumper Press"), FKeyDetails::GamepadKey, "Wave"));
	// InputButton[9]: A
	LOGD(LogWaveVRInput, "Add key: Wave (L) A Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_A, LOCTEXT("Left_A", "Wave (L) A Press"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) A Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_A, LOCTEXT("Right_A", "Wave (R) A Press"), FKeyDetails::GamepadKey, "Wave"));
	// InputButton[10]: B
	LOGD(LogWaveVRInput, "Add key: Wave (L) B Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_B, LOCTEXT("Left_B", "Wave (L) B Press"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) B Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_B, LOCTEXT("Right_B", "Wave (R) B Press"), FKeyDetails::GamepadKey, "Wave"));
	// InputButton[11]: X
	LOGD(LogWaveVRInput, "Add key: Wave (L) X Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_X, LOCTEXT("Left_X", "Wave (L) X Press"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) X Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_X, LOCTEXT("Right_X", "Wave (R) X Press"), FKeyDetails::GamepadKey, "Wave"));
	// InputButton[12]: Y
	LOGD(LogWaveVRInput, "Add key: Wave (L) Y Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Y, LOCTEXT("Left_Y", "Wave (L) Y Press"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) Y Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Y, LOCTEXT("Right_Y", "Wave (R) Y Press"), FKeyDetails::GamepadKey, "Wave"));
	// InputButton[13]: Back
	LOGD(LogWaveVRInput, "Add key: Wave (L) Back Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Back, LOCTEXT("Left_Back", "Wave (L) Back Press"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) Back Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Back, LOCTEXT("Right_Back", "Wave (R) Back Press"), FKeyDetails::GamepadKey, "Wave"));
	// InputButton[14]: Enter
	LOGD(LogWaveVRInput, "Add key: Wave (L) Enter Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Enter, LOCTEXT("Left_Enter", "Wave (L) Enter Press"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) Enter Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Enter, LOCTEXT("Right_Enter", "Wave (R) Enter Press"), FKeyDetails::GamepadKey, "Wave"));
	// InputButton[15]: Touchpad
	LOGD(LogWaveVRInput, "Add key: Wave (L) Touchpad Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Touchpad_Press, LOCTEXT("Left_Touchpad_Press", "Wave (L) Touchpad Press"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) Touchpad Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Touchpad_Press, LOCTEXT("Right_Touchpad_Press", "Wave (R) Touchpad Press"), FKeyDetails::GamepadKey, "Wave"));
	// InputButton[16]: Trigger
	LOGD(LogWaveVRInput, "Add key: Wave (L) Trigger Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Trigger_Press, LOCTEXT("Left_Trigger_Press", "Wave (L) Trigger Press"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) Trigger Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Trigger_Press, LOCTEXT("Right_Trigger_Press", "Wave (R) Trigger Press"), FKeyDetails::GamepadKey, "Wave"));
	// InputButton[17]: Thumbstick
	LOGD(LogWaveVRInput, "Add key: Wave (L) Thumbstick Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Thumbstick_Press, LOCTEXT("Left_Thumbstick_Press", "Wave (L) Thumbstick Press"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) Thumbstick Press");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Thumbstick_Press, LOCTEXT("Right_Thumbstick_Press", "Wave (R) Thumbstick Press"), FKeyDetails::GamepadKey, "Wave"));

	// -------------- Touch Buttons ----------------
	// TouchButton[0]: Touchpad
	LOGD(LogWaveVRInput, "Add key: Wave (L) Touchpad Touch");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Touchpad_Touch, LOCTEXT("Left_Touchpad_Touch", "Wave (L) Touchpad Touch"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (L) Touchpad X");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Touchpad_X, LOCTEXT("Left_Touchpad_X", "Wave (L) Touchpad X"), FKeyDetails::Axis1D, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (L) Touchpad Y");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Touchpad_Y, LOCTEXT("Left_Touchpad_Y", "Wave (L) Touchpad Y"), FKeyDetails::Axis1D, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) Touchpad Touch");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Touchpad_Touch, LOCTEXT("Right_Touchpad_Touch", "Wave (R) Touchpad Touch"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) Touchpad X");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Touchpad_X, LOCTEXT("Right_Touchpad_X", "Wave (R) Touchpad X"), FKeyDetails::Axis1D, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) Touchpad Y");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Touchpad_Y, LOCTEXT("Right_Touchpad_Y", "Wave (R) Touchpad Y"), FKeyDetails::Axis1D, "Wave"));

	// TouchButton[1]: Trigger
	LOGD(LogWaveVRInput, "Add key: Wave (L) Trigger Touch");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Trigger_Touch, LOCTEXT("Left_Trigger_Touch", "Wave (L) Trigger Touch"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (L) Trigger X");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Trigger_X, LOCTEXT("Left_Trigger_X", "Wave (L) Trigger Axis"), FKeyDetails::Axis1D, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) Trigger Touch");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Trigger_Touch, LOCTEXT("Right_Trigger_Touch", "Wave (R) Trigger Touch"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) Trigger X");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Trigger_X, LOCTEXT("Right_Trigger_X", "Wave (R) Trigger Axis"), FKeyDetails::Axis1D, "Wave"));

	// TouchButton[2]: Thumbstick
	LOGD(LogWaveVRInput, "Add key: Wave (L) Thumbstick Touch");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Thumbstick_Touch, LOCTEXT("Left_Thumbstick_Touch", "Wave (L) Thumbstick Touch"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (L) Thumbstick X");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Thumbstick_X, LOCTEXT("Left_Thumbstick_X", "Wave (L) Thumbstick X"), FKeyDetails::Axis1D, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (L) Thumbstick Y");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Thumbstick_Y, LOCTEXT("Left_Thumbstick_Y", "Wave (L) Thumbstick Y"), FKeyDetails::Axis1D, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) Thumbstick Touch");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Thumbstick_Touch, LOCTEXT("Right_Thumbstick_Touch", "Wave (R) Thumbstick Touch"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) Thumbstick X");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Thumbstick_X, LOCTEXT("Right_Thumbstick_X", "Wave (R) Thumbstick X"), FKeyDetails::Axis1D, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) Thumbstick Y");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Thumbstick_Y, LOCTEXT("Right_Thumbstick_Y", "Wave (R) Thumbstick Y"), FKeyDetails::Axis1D, "Wave"));

	// TouchButton[3]: Parking
	LOGD(LogWaveVRInput, "Add key: Wave (L) Parking Touch");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Parking_Touch, LOCTEXT("Left_Parking_Touch", "Wave (L) Parking Touch"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) Parking Touch");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Parking_Touch, LOCTEXT("Right_Parking_Touch", "Wave (R) Parking Touch"), FKeyDetails::GamepadKey, "Wave"));

	// TouchButton[4]: Grip
	LOGD(LogWaveVRInput, "Add key: Wave (L) Grip Touch");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Grip_Touch, LOCTEXT("Left_Grip_Touch", "Wave (L) Grip Touch"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (L) Grip X");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Grip_X, LOCTEXT("Left_Grip_X", "Wave (L) Grip Axis"), FKeyDetails::Axis1D, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) Grip Touch");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Grip_Touch, LOCTEXT("Right_Grip_Touch", "Wave (R) Grip Touch"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) Grip X");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Grip_X, LOCTEXT("Right_Grip_X", "Wave (R) Grip Axis"), FKeyDetails::Axis1D, "Wave"));

	// TouchButton[5]: A
	LOGD(LogWaveVRInput, "Add key: Wave (L) A Touch");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_A_Touch, LOCTEXT("Left_A_Touch", "Wave (L) A Touch"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) A Touch");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_A_Touch, LOCTEXT("Right_A_Touch", "Wave (R) A Touch"), FKeyDetails::GamepadKey, "Wave"));

	// TouchButton[6]: B
	LOGD(LogWaveVRInput, "Add key: Wave (L) B Touch");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_B_Touch, LOCTEXT("Left_B_Touch", "Wave (L) B Touch"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) B Touch");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_B_Touch, LOCTEXT("Right_B_Touch", "Wave (R) B Touch"), FKeyDetails::GamepadKey, "Wave"));

	// TouchButton[7]: X
	LOGD(LogWaveVRInput, "Add key: Wave (L) X Touch");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_X_Touch, LOCTEXT("Left_X_Touch", "Wave (L) X Touch"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) X Touch");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_X_Touch, LOCTEXT("Right_X_Touch", "Wave (R) X Touch"), FKeyDetails::GamepadKey, "Wave"));

	// TouchButton[8]: Y
	LOGD(LogWaveVRInput, "Add key: Wave (L) Y Touch");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Y_Touch, LOCTEXT("Left_Y_Touch", "Wave (L) Y Touch"), FKeyDetails::GamepadKey, "Wave"));
	LOGD(LogWaveVRInput, "Add key: Wave (R) Y Touch");
	EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Y_Touch, LOCTEXT("Right_Y_Touch", "Wave (R) Y Touch"), FKeyDetails::GamepadKey, "Wave"));
}

FName FWaveVRInput::DeviceTypeName(TEXT("WaveVRInput"));

FWaveVRInput::FWaveVRInput(WaveVRInputSimulator* simulator, const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler )
	: pSimulator(simulator)
	, bInputInitialized(false)
	, fFPS(0)
	, enumUseSimulationPose(SimulatePosition::WhenNoPosition)
	, framesOfFreeze(0)
	, FollowHead(false)
	, defaultHeadUEPosition(FVector::ZeroVector)
	, defaultHeadUnityPosition(FVector::ZeroVector)
	, bodyDirection(FVector::ZeroVector)
	, bodyRotation(FQuat::Identity)
	, simulateUnityPosition(FVector::ZeroVector)
	, simulateUEPosition(FVector::ZeroVector)
	, simulateUnityQuaternion(FQuat::Identity)
	, simulateUERotation(FRotator::ZeroRotator)
	, v3ChangeArmYAxis(FVector(1, 1, 1))
	, bIsLeftHanded(false)
	, MessageHandler(InMessageHandler)
{
	IModularFeatures::Get().RegisterModularFeature( GetModularFeatureName(), this );

	for (int i = 0; i < ControllerCount; i++)
	{
		for (int j = 0; j < InputButtonCount; j++)
			PrevPressState[i][j] = false;
		for (int j = 0; j < TouchButtonCount; j++)
			PrevTouchState[i][j] = false;
	}

	int32 right_hand = (int32)EControllerHand::Right;
	int32 left_hand = (int32)EControllerHand::Left;

	EKeys::AddMenuCategoryDisplayInfo("Wave", LOCTEXT("WaveSubCategory", "Wave"), TEXT("GraphEditor.PadEvent_16x"));

	// HmdPressButton[0]: Back
	//LOGD(LogWaveVRInput, "Add key: Wave (H) Back Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Hmd_Back, LOCTEXT("Hmd_Back", "Wave (H) Back Press"), FKeyDetails::GamepadKey, "Wave"));
	HmdPressButtons[0] = WaveVRControllerKeyNames::Hmd_Back.GetFName();

	// HmdPressButton[1]: Enter
	//LOGD(LogWaveVRInput, "Add key: Wave (H) Enter Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Hmd_Enter, LOCTEXT("Hmd_Enter", "Wave (H) Enter Press"), FKeyDetails::GamepadKey, "Wave"));
	HmdPressButtons[1] = WaveVRControllerKeyNames::Hmd_Enter.GetFName();


	// ----------------- Press Buttons --------------------
	// InputButton[0]: Menu
	uint8 press_index = 0;
	//LOGD(LogWaveVRInput, "Add key: Wave (L) Menu Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Menu, LOCTEXT("Left_Menu", "Wave (L) Menu Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[left_hand][press_index] = WaveVRControllerKeyNames::Left_Menu.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (R) Menu Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Menu, LOCTEXT("Right_Menu", "Wave (R) Menu Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[right_hand][press_index] = WaveVRControllerKeyNames::Right_Menu.GetFName();

	// InputButton[1]: Grip
	press_index++;
	//LOGD(LogWaveVRInput, "Add key: Wave (L) Grip Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Grip, LOCTEXT("Left_Grip", "Wave (L) Grip Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[left_hand][press_index] = WaveVRControllerKeyNames::Left_Grip.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (R) Grip Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Grip, LOCTEXT("Right_Grip", "Wave (R) Grip Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[right_hand][press_index] = WaveVRControllerKeyNames::Right_Grip.GetFName();

	// InputButton[2]: DPad_Left
	press_index++;
	//LOGD(LogWaveVRInput, "Add key: Wave (L) DPadLeft Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_DPadLeft, LOCTEXT("Left_DPadLeft", "Wave (L) DPadLeft Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[left_hand][press_index] = WaveVRControllerKeyNames::Left_DPadLeft.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (R) DPadLeft Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_DPadLeft, LOCTEXT("Right_DPadLeft", "Wave (R) DPadLeft Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[right_hand][press_index] = WaveVRControllerKeyNames::Right_DPadLeft.GetFName();

	// InputButton[3]: DPad_Up
	press_index++;
	//LOGD(LogWaveVRInput, "Add key: Wave (L) DPadUp Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_DPadUp, LOCTEXT("Left_DPadUp", "Wave (L) DPadUp Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[left_hand][press_index] = WaveVRControllerKeyNames::Left_DPadUp.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (R) DPadUp Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_DPadUp, LOCTEXT("Right_DPadUp", "Wave (R) DPadUp Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[right_hand][press_index] = WaveVRControllerKeyNames::Right_DPadUp.GetFName();

	// InputButton[4]: DPad_Right
	press_index++;
	//LOGD(LogWaveVRInput, "Add key: Wave (L) DPadRight Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_DPadRight, LOCTEXT("Left_DPadRight", "Wave (L) DPadRight Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[left_hand][press_index] = WaveVRControllerKeyNames::Left_DPadRight.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (R) DPadRight Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_DPadRight, LOCTEXT("Right_DPadRight", "Wave (R) DPadRight Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[right_hand][press_index] = WaveVRControllerKeyNames::Right_DPadRight.GetFName();

	// InputButton[5]: DPad_Down
	press_index++;
	//LOGD(LogWaveVRInput, "Add key: Wave (L) DPadDown Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_DPadDown, LOCTEXT("Left_DPadDown", "Wave (L) DPadDown Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[left_hand][press_index] = WaveVRControllerKeyNames::Left_DPadDown.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (R) DPadDown Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_DPadDown, LOCTEXT("Right_DPadDown", "Wave (R) DPadDown Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[right_hand][press_index] = WaveVRControllerKeyNames::Right_DPadDown.GetFName();

	// InputButton[6]: Volume_Up
	press_index++;
	//LOGD(LogWaveVRInput, "Add key: Wave (L) VolumeUp Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_VolumeUp, LOCTEXT("Left_VolumeUp", "Wave (L) VolumeUp Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[left_hand][press_index] = WaveVRControllerKeyNames::Left_VolumeUp.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (R) VolumeUp Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_VolumeUp, LOCTEXT("Right_VolumeUp", "Wave (R) VolumeUp Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[right_hand][press_index] = WaveVRControllerKeyNames::Right_VolumeUp.GetFName();

	// InputButton[7]: Volume_Down
	press_index++;
	//LOGD(LogWaveVRInput, "Add key: Wave (L) VolumeDown Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_VolumeDown, LOCTEXT("Left_VolumeDown", "Wave (L) VolumeDown Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[left_hand][press_index] = WaveVRControllerKeyNames::Left_VolumeDown.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (R) VolumeDown Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_VolumeDown, LOCTEXT("Right_VolumeDown", "Wave (R) VolumeDown Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[right_hand][press_index] = WaveVRControllerKeyNames::Right_VolumeDown.GetFName();

	// InputButton[8]: Bumper
	press_index++;
	//LOGD(LogWaveVRInput, "Add key: Wave (L) Bumper Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Bumper, LOCTEXT("Left_Bumper", "Wave (L) Bumper Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[left_hand][press_index] = WaveVRControllerKeyNames::Left_Bumper.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (R) Bumper Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Bumper, LOCTEXT("Right_Bumper", "Wave (R) Bumper Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[right_hand][press_index] = WaveVRControllerKeyNames::Right_Bumper.GetFName();

	// InputButton[9]: A
	press_index++;
	//LOGD(LogWaveVRInput, "Add key: Wave (L) A Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_A, LOCTEXT("Left_A", "Wave (L) A Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[left_hand][press_index] = WaveVRControllerKeyNames::Left_A.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (R) A Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_A, LOCTEXT("Right_A", "Wave (R) A Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[right_hand][press_index] = WaveVRControllerKeyNames::Right_A.GetFName();

	// InputButton[10]: B
	press_index++;
	//LOGD(LogWaveVRInput, "Add key: Wave (L) B Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_B, LOCTEXT("Left_B", "Wave (L) B Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[left_hand][press_index] = WaveVRControllerKeyNames::Left_B.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (R) B Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_B, LOCTEXT("Right_B", "Wave (R) B Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[right_hand][press_index] = WaveVRControllerKeyNames::Right_B.GetFName();

	// InputButton[11]: X
	press_index++;
	//LOGD(LogWaveVRInput, "Add key: Wave (L) X Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_X, LOCTEXT("Left_X", "Wave (L) X Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[left_hand][press_index] = WaveVRControllerKeyNames::Left_X.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (R) X Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_X, LOCTEXT("Right_X", "Wave (R) X Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[right_hand][press_index] = WaveVRControllerKeyNames::Right_X.GetFName();

	// InputButton[12]: Y
	press_index++;
	//LOGD(LogWaveVRInput, "Add key: Wave (L) Y Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Y, LOCTEXT("Left_Y", "Wave (L) Y Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[left_hand][press_index] = WaveVRControllerKeyNames::Left_Y.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (R) Y Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Y, LOCTEXT("Right_Y", "Wave (R) Y Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[right_hand][press_index] = WaveVRControllerKeyNames::Right_Y.GetFName();

	// InputButton[13]: Back
	press_index++;
	//LOGD(LogWaveVRInput, "Add key: Wave (L) Back Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Back, LOCTEXT("Left_Back", "Wave (L) Back Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[left_hand][press_index] = WaveVRControllerKeyNames::Left_Back.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (R) Back Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Back, LOCTEXT("Right_Back", "Wave (R) Back Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[right_hand][press_index] = WaveVRControllerKeyNames::Right_Back.GetFName();

	// InputButton[14]: Enter
	press_index++;
	//LOGD(LogWaveVRInput, "Add key: Wave (L) Enter Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Enter, LOCTEXT("Left_Enter", "Wave (L) Enter Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[left_hand][press_index] = WaveVRControllerKeyNames::Left_Enter.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (R) Enter Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Enter, LOCTEXT("Right_Enter", "Wave (R) Enter Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[right_hand][press_index] = WaveVRControllerKeyNames::Right_Enter.GetFName();

	// InputButton[15]: Touchpad
	press_index++;
	//LOGD(LogWaveVRInput, "Add key: Wave (L) Touchpad Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Touchpad_Press, LOCTEXT("Left_Touchpad_Press", "Wave (L) Touchpad Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[left_hand][press_index] = WaveVRControllerKeyNames::Left_Touchpad_Press.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (R) Touchpad Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Touchpad_Press, LOCTEXT("Right_Touchpad_Press", "Wave (R) Touchpad Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[right_hand][press_index] = WaveVRControllerKeyNames::Right_Touchpad_Press.GetFName();

	// InputButton[16]: Trigger
	press_index++;
	//LOGD(LogWaveVRInput, "Add key: Wave (L) Trigger Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Trigger_Press, LOCTEXT("Left_Trigger_Press", "Wave (L) Trigger Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[left_hand][press_index] = WaveVRControllerKeyNames::Left_Trigger_Press.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (R) Trigger Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Trigger_Press, LOCTEXT("Right_Trigger_Press", "Wave (R) Trigger Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[right_hand][press_index] = WaveVRControllerKeyNames::Right_Trigger_Press.GetFName();

	// InputButton[17]: Thumbstick
	press_index++;
	//LOGD(LogWaveVRInput, "Add key: Wave (L) Thumbstick Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Thumbstick_Press, LOCTEXT("Left_Thumbstick_Press", "Wave (L) Thumbstick Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[left_hand][press_index] = WaveVRControllerKeyNames::Left_Thumbstick_Press.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (R) Thumbstick Press");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Thumbstick_Press, LOCTEXT("Right_Thumbstick_Press", "Wave (R) Thumbstick Press"), FKeyDetails::GamepadKey, "Wave"));
	ControllerPressButtons[right_hand][press_index] = WaveVRControllerKeyNames::Right_Thumbstick_Press.GetFName();


	// -------------- Touch Buttons ----------------
	// TouchButton[0]: Touchpad
	uint8 touch_index = 0;
	//LOGD(LogWaveVRInput, "Add key: Wave (L) Touchpad Touch");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Touchpad_Touch, LOCTEXT("Left_Touchpad_Touch", "Wave (L) Touchpad Touch"), FKeyDetails::GamepadKey, "Wave"));
	ControllerTouchButtons[left_hand][touch_index] = WaveVRControllerKeyNames::Left_Touchpad_Touch.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (L) Touchpad X");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Touchpad_X, LOCTEXT("Left_Touchpad_X", "Wave (L) Touchpad X"), FKeyDetails::Axis1D, "Wave"));

	//LOGD(LogWaveVRInput, "Add key: Wave (L) Touchpad Y");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Touchpad_Y, LOCTEXT("Left_Touchpad_Y", "Wave (L) Touchpad Y"), FKeyDetails::Axis1D, "Wave"));

	//LOGD(LogWaveVRInput, "Add key: Wave (R) Touchpad Touch");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Touchpad_Touch, LOCTEXT("Right_Touchpad_Touch", "Wave (R) Touchpad Touch"), FKeyDetails::GamepadKey, "Wave"));
	ControllerTouchButtons[right_hand][touch_index] = WaveVRControllerKeyNames::Right_Touchpad_Touch.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (R) Touchpad X");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Touchpad_X, LOCTEXT("Right_Touchpad_X", "Wave (R) Touchpad X"), FKeyDetails::Axis1D, "Wave"));

	//LOGD(LogWaveVRInput, "Add key: Wave (R) Touchpad Y");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Touchpad_Y, LOCTEXT("Right_Touchpad_Y", "Wave (R) Touchpad Y"), FKeyDetails::Axis1D, "Wave"));

	// TouchButton[1]: Trigger
	touch_index++;
	//LOGD(LogWaveVRInput, "Add key: Wave (L) Trigger Touch");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Trigger_Touch, LOCTEXT("Left_Trigger_Touch", "Wave (L) Trigger Touch"), FKeyDetails::GamepadKey, "Wave"));
	ControllerTouchButtons[left_hand][touch_index] = WaveVRControllerKeyNames::Left_Trigger_Touch.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (L) Trigger X");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Trigger_X, LOCTEXT("Left_Trigger_X", "Wave (L) Trigger Axis"), FKeyDetails::Axis1D, "Wave"));

	//LOGD(LogWaveVRInput, "Add key: Wave (R) Trigger Touch");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Trigger_Touch, LOCTEXT("Right_Trigger_Touch", "Wave (R) Trigger Touch"), FKeyDetails::GamepadKey, "Wave"));
	ControllerTouchButtons[right_hand][touch_index] = WaveVRControllerKeyNames::Right_Trigger_Touch.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (R) Trigger X");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Trigger_X, LOCTEXT("Right_Trigger_X", "Wave (R) Trigger Axis"), FKeyDetails::Axis1D, "Wave"));

	// TouchButton[2]: Thumbstick
	touch_index++;
	//LOGD(LogWaveVRInput, "Add key: Wave (L) Thumbstick Touch");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Thumbstick_Touch, LOCTEXT("Left_Thumbstick_Touch", "Wave (L) Thumbstick Touch"), FKeyDetails::GamepadKey, "Wave"));
	ControllerTouchButtons[left_hand][touch_index] = WaveVRControllerKeyNames::Left_Thumbstick_Touch.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (L) Thumbstick X");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Thumbstick_X, LOCTEXT("Left_Thumbstick_X", "Wave (L) Thumbstick X"), FKeyDetails::Axis1D, "Wave"));

	//LOGD(LogWaveVRInput, "Add key: Wave (L) Thumbstick Y");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Thumbstick_Y, LOCTEXT("Left_Thumbstick_Y", "Wave (L) Thumbstick Y"), FKeyDetails::Axis1D, "Wave"));

	//LOGD(LogWaveVRInput, "Add key: Wave (R) Thumbstick Touch");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Thumbstick_Touch, LOCTEXT("Right_Thumbstick_Touch", "Wave (R) Thumbstick Touch"), FKeyDetails::GamepadKey, "Wave"));
	ControllerTouchButtons[right_hand][touch_index] = WaveVRControllerKeyNames::Right_Thumbstick_Touch.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (R) Thumbstick X");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Thumbstick_X, LOCTEXT("Right_Thumbstick_X", "Wave (R) Thumbstick X"), FKeyDetails::Axis1D, "Wave"));

	//LOGD(LogWaveVRInput, "Add key: Wave (R) Thumbstick Y");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Thumbstick_Y, LOCTEXT("Right_Thumbstick_Y", "Wave (R) Thumbstick Y"), FKeyDetails::Axis1D, "Wave"));

	// TouchButton[3]: Parking
	touch_index++;
	//LOGD(LogWaveVRInput, "Add key: Wave (L) Parking Touch");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Parking_Touch, LOCTEXT("Left_Parking_Touch", "Wave (L) Parking Touch"), FKeyDetails::GamepadKey, "Wave"));
	ControllerTouchButtons[left_hand][touch_index] = WaveVRControllerKeyNames::Left_Parking_Touch.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (R) Parking Touch");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Parking_Touch, LOCTEXT("Right_Parking_Touch", "Wave (R) Parking Touch"), FKeyDetails::GamepadKey, "Wave"));
	ControllerTouchButtons[right_hand][touch_index] = WaveVRControllerKeyNames::Right_Parking_Touch.GetFName();

	// TouchButton[4]: Grip
	touch_index++;
	//LOGD(LogWaveVRInput, "Add key: Wave (L) Grip Touch");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Grip_Touch, LOCTEXT("Left_Grip_Touch", "Wave (L) Grip Touch"), FKeyDetails::GamepadKey, "Wave"));
	ControllerTouchButtons[left_hand][touch_index] = WaveVRControllerKeyNames::Left_Grip_Touch.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (L) Grip X");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Grip_X, LOCTEXT("Left_Grip_X", "Wave (L) Grip Axis"), FKeyDetails::Axis1D, "Wave"));

	//LOGD(LogWaveVRInput, "Add key: Wave (R) Grip Touch");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Grip_Touch, LOCTEXT("Right_Grip_Touch", "Wave (R) Grip Touch"), FKeyDetails::GamepadKey, "Wave"));
	ControllerTouchButtons[right_hand][touch_index] = WaveVRControllerKeyNames::Right_Grip_Touch.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (R) Grip X");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Grip_X, LOCTEXT("Right_Grip_X", "Wave (R) Grip Axis"), FKeyDetails::Axis1D, "Wave"));

	// TouchButton[5]: A
	touch_index++;
	//LOGD(LogWaveVRInput, "Add key: Wave (L) A Touch");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_A_Touch, LOCTEXT("Left_A_Touch", "Wave (L) A Touch"), FKeyDetails::GamepadKey, "Wave"));
	ControllerTouchButtons[left_hand][touch_index] = WaveVRControllerKeyNames::Left_A_Touch.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (R) A Touch");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_A_Touch, LOCTEXT("Right_A_Touch", "Wave (R) A Touch"), FKeyDetails::GamepadKey, "Wave"));
	ControllerTouchButtons[right_hand][touch_index] = WaveVRControllerKeyNames::Right_A_Touch.GetFName();

	// TouchButton[6]: B
	touch_index++;
	//LOGD(LogWaveVRInput, "Add key: Wave (L) B Touch");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_B_Touch, LOCTEXT("Left_B_Touch", "Wave (L) B Touch"), FKeyDetails::GamepadKey, "Wave"));
	ControllerTouchButtons[left_hand][touch_index] = WaveVRControllerKeyNames::Left_B_Touch.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (R) B Touch");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_B_Touch, LOCTEXT("Right_B_Touch", "Wave (R) B Touch"), FKeyDetails::GamepadKey, "Wave"));
	ControllerTouchButtons[right_hand][touch_index] = WaveVRControllerKeyNames::Right_B_Touch.GetFName();

	// TouchButton[7]: X
	touch_index++;
	//LOGD(LogWaveVRInput, "Add key: Wave (L) X Touch");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_X_Touch, LOCTEXT("Left_X_Touch", "Wave (L) X Touch"), FKeyDetails::GamepadKey, "Wave"));
	ControllerTouchButtons[left_hand][touch_index] = WaveVRControllerKeyNames::Left_X_Touch.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (R) X Touch");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_X_Touch, LOCTEXT("Right_X_Touch", "Wave (R) X Touch"), FKeyDetails::GamepadKey, "Wave"));
	ControllerTouchButtons[right_hand][touch_index] = WaveVRControllerKeyNames::Right_X_Touch.GetFName();

	// TouchButton[8]: Y
	touch_index++;
	//LOGD(LogWaveVRInput, "Add key: Wave (L) Y Touch");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Left_Y_Touch, LOCTEXT("Left_Y_Touch", "Wave (L) Y Touch"), FKeyDetails::GamepadKey, "Wave"));
	ControllerTouchButtons[left_hand][touch_index] = WaveVRControllerKeyNames::Left_Y_Touch.GetFName();

	//LOGD(LogWaveVRInput, "Add key: Wave (R) Y Touch");
	//EKeys::AddKey(FKeyDetails(WaveVRControllerKeyNames::Right_Y_Touch, LOCTEXT("Right_Y_Touch", "Wave (R) Y Touch"), FKeyDetails::GamepadKey, "Wave"));
	ControllerTouchButtons[right_hand][touch_index] = WaveVRControllerKeyNames::Right_Y_Touch.GetFName();

	for (unsigned int i = 0; i < EWVR_DeviceType_Count; i++)
	{
		uePose[i].localRotation = FRotator::ZeroRotator;
		uePose[i].localPosition = FVector::ZeroVector;
		uePose_pev[i].localRotation = FRotator::ZeroRotator;
		uePose_pev[i].localPosition = FVector::ZeroVector;
		unityPose[i].pos = FVector::ZeroVector;
		unityPose[i].rot = FQuat::Identity;
		unityPose_prev[i].pos = FVector::ZeroVector;
		unityPose_prev[i].rot = FQuat::Identity;
		bPoseIsValid[i] = false;
		CurrentDoF[i] = EWVR_DOF::DOF_3;
	}

	bIsLeftHanded = UWaveVRBlueprintFunctionLibrary::IsLeftHandedMode();

	LOGD(LogWaveVRInput, "WaveVRInput is initialized");
	bInputInitialized = true;
}


FWaveVRInput::~FWaveVRInput()
{
	IModularFeatures::Get().UnregisterModularFeature( GetModularFeatureName(), this );
}

bool FWaveVRInput::IsInputInitialized()
{
	return bInputInitialized;
}

void FWaveVRInput::FireButtonPressEvent(EControllerHand hand, EWVR_InputId id, bool down)
{
	LOGD(LogWaveVRInput, "FireButtonPressEvent() hand %d button %d is pressed %s.", (uint8)hand, (uint8)id, (down ? "down" : "up"));
	switch (hand)
	{
	case EControllerHand::Right:
		UWaveVREventCommon::OnAllButtonPressNative_Right.Broadcast((uint8)id, down);
		break;
	case EControllerHand::Left:
		UWaveVREventCommon::OnAllButtonPressNative_Left.Broadcast((uint8)id, down);
		break;
	default:
		break;
	}
}

void FWaveVRInput::FireButtonTouchEvent(EControllerHand hand, EWVR_TouchId id, bool down)
{
	LOGD(LogWaveVRInput, "FireButtonTouchEvent() hand %d button %d is touched %s.", (uint8)hand, (uint8)id, (down ? "down" : "up"));
	switch (hand)
	{
	case EControllerHand::Right:
		UWaveVREventCommon::OnAllButtonTouchNative_Right.Broadcast((uint8)id, down);
		break;
	case EControllerHand::Left:
		UWaveVREventCommon::OnAllButtonTouchNative_Left.Broadcast((uint8)id, down);
		break;
	default:
		break;
	}
}

void FWaveVRInput::UpdateHmdPressStates()
{
	bool s_HmdPressState[HmdPressCount] = { false };
	for (int i = 0; i < HmdPressCount; i++)
	{
		s_HmdPressState[i] = UWaveVRBlueprintFunctionLibrary::IsButtonPressed(EWVR_DeviceType::DeviceType_HMD, HmdPressButton[i]);
	}

	for (int i = 0; i < HmdPressCount; i++)
	{
		FName button_name = HmdPressButtons[i];

		if (s_HmdPressState[i] != s_HmdPressStateEx[i])
		{
			if (s_HmdPressState[i])
			{
				LOGD(LogWaveVRInput, "UpdateHmdPressStates() HMD button %s is pressed.", TCHAR_TO_ANSI(*button_name.ToString()));
				MessageHandler->OnControllerButtonPressed(button_name, 0, false);
			}
			else
			{
				LOGD(LogWaveVRInput, "UpdateHmdPressStates() HMD button %s is released.", TCHAR_TO_ANSI(*button_name.ToString()));
				MessageHandler->OnControllerButtonReleased(button_name, 0, false);
			}
		}

		s_HmdPressStateEx[i] = s_HmdPressState[i];
	}
}
void FWaveVRInput::UpdateButtonPressStates(EControllerHand hand)
{
	EWVR_DeviceType adaptive_device = GetLeftHandedDevice(hand);

	int32 hand_index = (int32)hand;
	bool curr_press_state[InputButtonCount] = { false };

	// Get current buttons' states.
	for (int i = 0; i < InputButtonCount; i++)
	{
		curr_press_state[i] = UWaveVRBlueprintFunctionLibrary::IsButtonPressed(adaptive_device, InputButton[i]);
	}

	// Compare with previous buttons' states.
	for (int i = 0; i < InputButtonCount; i++)
	{
		FName button_name = ControllerPressButtons[hand_index][i];

		if (curr_press_state[i] != PrevPressState[hand_index][i])
		{
			if (curr_press_state[i])
			{
				LOGD(LogWaveVRInput, "UpdateButtonPressStates() hand %d device %d button %s is pressed.", hand_index, (int32)adaptive_device, TCHAR_TO_ANSI(*button_name.ToString()));
				FireButtonPressEvent(hand, InputButton[i], true);
				MessageHandler->OnControllerButtonPressed(button_name, 0, false);
			}
			else
			{
				LOGD(LogWaveVRInput, "UpdateButtonPressStates() hand %d device %d button %s is released.", hand_index, (int32)adaptive_device, TCHAR_TO_ANSI(*button_name.ToString()));
				FireButtonPressEvent(hand, InputButton[i], false);
				MessageHandler->OnControllerButtonReleased(button_name, 0, false);
			}
		}

		PrevPressState[hand_index][i] = curr_press_state[i];
	}
}
void FWaveVRInput::UpdateButtonTouchStates(EControllerHand hand)
{
	EWVR_DeviceType adaptive_device = GetLeftHandedDevice(hand);

	int32 hand_index = (int32)hand;
	bool curr_touch_state[TouchButtonCount] = { false };

	for (int i = 0; i < TouchButtonCount; i++)
	{
		if (!IsPlayInEditor() || WaveVRDirectPreview::IsDirectPreview())
		{
			curr_touch_state[i] = UWaveVRBlueprintFunctionLibrary::IsButtonTouched(adaptive_device, TouchButton[i]);
		}

		if (curr_touch_state[i])
		{
			FVector2D axis = UWaveVRController::GetControllerAxis(adaptive_device, TouchButton[i]);
			//LOGD(LogWaveVRInput, "UpdateButtonTouchStates() hand %d device %d axis (%f, %f)", hand_index, (int32)adaptive_device, axis.X, axis.Y);
			switch (TouchButton[i])
			{
			case EWVR_TouchId::Touchpad:
				if (hand == EControllerHand::Right)
				{
					MessageHandler->OnControllerAnalog(WaveVRControllerKeyNames::Right_Touchpad_X.GetFName(), 0, axis.X);
					MessageHandler->OnControllerAnalog(WaveVRControllerKeyNames::Right_Touchpad_Y.GetFName(), 0, axis.Y);
				}
				if (hand == EControllerHand::Left)
				{
					MessageHandler->OnControllerAnalog(WaveVRControllerKeyNames::Left_Touchpad_X.GetFName(), 0, axis.X);
					MessageHandler->OnControllerAnalog(WaveVRControllerKeyNames::Left_Touchpad_Y.GetFName(), 0, axis.Y);
				}
				break;
			case EWVR_TouchId::Thumbstick:
				if (hand == EControllerHand::Right)
				{
					MessageHandler->OnControllerAnalog(WaveVRControllerKeyNames::Right_Thumbstick_X.GetFName(), 0, axis.X);
					MessageHandler->OnControllerAnalog(WaveVRControllerKeyNames::Right_Thumbstick_Y.GetFName(), 0, axis.Y);
				}
				if (hand == EControllerHand::Left)
				{
					MessageHandler->OnControllerAnalog(WaveVRControllerKeyNames::Left_Thumbstick_X.GetFName(), 0, axis.X);
					MessageHandler->OnControllerAnalog(WaveVRControllerKeyNames::Left_Thumbstick_Y.GetFName(), 0, axis.Y);
				}
				break;
			case EWVR_TouchId::Trigger:
			{
				float trigger_axis = axis.X < 0 ? -(axis.X) : axis.X;
				if (hand == EControllerHand::Right)
				{
					MessageHandler->OnControllerAnalog(WaveVRControllerKeyNames::Right_Trigger_X.GetFName(), 0, trigger_axis);
				}
				if (hand == EControllerHand::Left)
				{
					MessageHandler->OnControllerAnalog(WaveVRControllerKeyNames::Left_Trigger_X.GetFName(), 0, trigger_axis);
				}
				break;
			}
			case EWVR_TouchId::Grip:
			{
				float grip_axis = axis.X < 0 ? -(axis.X) : axis.X;
				if (hand == EControllerHand::Right)
				{
					MessageHandler->OnControllerAnalog(WaveVRControllerKeyNames::Right_Grip_X.GetFName(), 0, grip_axis);
				}
				if (hand == EControllerHand::Left)
				{
					MessageHandler->OnControllerAnalog(WaveVRControllerKeyNames::Left_Grip_X.GetFName(), 0, grip_axis);
				}
				break;
			}
				// case EWVR_TouchId::Parking: // Parking does NOT have axis.
			default:
				break;
			}
		}
		else
		{
			MessageHandler->OnControllerAnalog(WaveVRControllerKeyNames::Right_Touchpad_X.GetFName(), 0, 0);
			MessageHandler->OnControllerAnalog(WaveVRControllerKeyNames::Right_Touchpad_Y.GetFName(), 0, 0);
			MessageHandler->OnControllerAnalog(WaveVRControllerKeyNames::Left_Touchpad_X.GetFName(), 0, 0);
			MessageHandler->OnControllerAnalog(WaveVRControllerKeyNames::Left_Touchpad_Y.GetFName(), 0, 0);
			MessageHandler->OnControllerAnalog(WaveVRControllerKeyNames::Right_Trigger_X.GetFName(), 0, 0);
			MessageHandler->OnControllerAnalog(WaveVRControllerKeyNames::Left_Trigger_X.GetFName(), 0, 0);
			MessageHandler->OnControllerAnalog(WaveVRControllerKeyNames::Right_Thumbstick_X.GetFName(), 0, 0);
			MessageHandler->OnControllerAnalog(WaveVRControllerKeyNames::Right_Thumbstick_Y.GetFName(), 0, 0);
			MessageHandler->OnControllerAnalog(WaveVRControllerKeyNames::Left_Thumbstick_X.GetFName(), 0, 0);
			MessageHandler->OnControllerAnalog(WaveVRControllerKeyNames::Left_Thumbstick_Y.GetFName(), 0, 0);
			MessageHandler->OnControllerAnalog(WaveVRControllerKeyNames::Right_Grip_X.GetFName(), 0, 0);
			MessageHandler->OnControllerAnalog(WaveVRControllerKeyNames::Left_Grip_X.GetFName(), 0, 0);
		}
	}

	for (int i = 0; i < TouchButtonCount; i++)
	{
		FName button_name = ControllerTouchButtons[hand_index][i];

		if (curr_touch_state[i] != PrevTouchState[hand_index][i])
		{
			if (curr_touch_state[i])
			{
				LOGD(LogWaveVRInput, "UpdateButtonTouchStates() hand %d device %d button %s is touched.", hand_index, (int32)adaptive_device, TCHAR_TO_ANSI(*button_name.ToString()));
				FireButtonTouchEvent(hand, TouchButton[i], true);
				MessageHandler->OnControllerButtonPressed(button_name, 0, false);
			}
			else
			{
				LOGD(LogWaveVRInput, "UpdateButtonTouchStates() hand %d device %d button %s is untouched.", hand_index, (int32)adaptive_device, TCHAR_TO_ANSI(*button_name.ToString()));
				FireButtonTouchEvent(hand, TouchButton[i], false);
				MessageHandler->OnControllerButtonReleased(button_name, 0, false);
			}
		}

		PrevTouchState[hand_index][i] = curr_touch_state[i];
	}
}

void FWaveVRInput::EnableInputSimulator(UObject * WorldContextObject)
{
	LOGD(LogWaveVRInput, "EnableInputSimulator");
	pSimulator->EnableSimulator(WorldContextObject);
}

#pragma region
void FWaveVRInput::SendControllerEvents()
{
	//LOGD(LogWaveVRInput, "SendControllerEvents()");

	UpdateHmdPressStates();
	UpdateButtonPressStates(EControllerHand::Right);
	UpdateButtonPressStates(EControllerHand::Left);
	UpdateButtonTouchStates(EControllerHand::Right);
	UpdateButtonTouchStates(EControllerHand::Left);
}
#pragma endregion IInputDevice overrides

#pragma region
bool FWaveVRInput::GetControllerOrientationAndPosition( const int32 ControllerIndex, const EControllerHand DeviceHand, FRotator& OutOrientation, FVector& OutPosition, float WorldToMetersScale) const
{
	bool bIsValid = false;
	unsigned int type_index = 0;

	if (DeviceHand == EControllerHand::Right)
		type_index = (unsigned int)EWVR_DeviceType::DeviceType_Controller_Right;
	else if (DeviceHand == EControllerHand::Left)
		type_index = (unsigned int)EWVR_DeviceType::DeviceType_Controller_Left;
	else if (DeviceHand == EControllerHand::ExternalCamera)
		type_index = (unsigned int)EWVR_DeviceType::DeviceType_HMD;
	else
		return false;

	bIsValid = bPoseIsValid[type_index];
	OutPosition = uePose[type_index].localPosition;
	OutOrientation = uePose[type_index].localRotation;

	return bIsValid;
}

ETrackingStatus FWaveVRInput::GetControllerTrackingStatus(const int32 ControllerIndex, const EControllerHand DeviceHand) const
{
	ETrackingStatus status = ETrackingStatus::NotTracked;

	if (DeviceHand == EControllerHand::Right)
	{
		if (bPoseIsValid[(unsigned int)EWVR_DeviceType::DeviceType_Controller_Right])
			status = ETrackingStatus::Tracked;
	}
	else if (DeviceHand == EControllerHand::Left)
	{
		if (bPoseIsValid[(unsigned int)EWVR_DeviceType::DeviceType_Controller_Left])
			status = ETrackingStatus::Tracked;
	}
	else if (DeviceHand == EControllerHand::ExternalCamera)
	{
		if (bPoseIsValid[(unsigned int)EWVR_DeviceType::DeviceType_HMD])
			status = ETrackingStatus::Tracked;
	}
	else if (DeviceHand == EControllerHand::AnyHand)
	{
		if (UWaveVRBlueprintFunctionLibrary::IsDeviceConnected(EWVR_DeviceType::DeviceType_Controller_Right) ||
			UWaveVRBlueprintFunctionLibrary::IsDeviceConnected(EWVR_DeviceType::DeviceType_Controller_Left))
		{
			status = ETrackingStatus::InertialOnly;
		}
	}
	else
	{
		return ETrackingStatus::NotTracked;
	}

	return status;
}
#pragma endregion IMotionController overrides

void FWaveVRInput::SetHapticFeedbackValues(int32 ControllerId, int32 Hand, const FHapticFeedbackValues& Values)
{
}

void FWaveVRInput::GetHapticFrequencyRange(float& MinFrequency, float& MaxFrequency) const
{
	MinFrequency = 0.0f;
	MaxFrequency = 1.0f;
}

float FWaveVRInput::GetHapticAmplitudeScale() const
{
	return 1.0f;
}

void FWaveVRInput::Tick(float DeltaTime)
{
	fFPS = 1 / DeltaTime;
	//LOGD(LogWaveVRInput, "Tick() fps is %f, simulation: %d", fFPS, enumUseSimulationPose);
	bIsLeftHanded = UWaveVRBlueprintFunctionLibrary::IsLeftHandedMode();
	UpdatePose();
}

#pragma region
void FWaveVRInput::UpdatePose()
{
	// Get pose and transform to Unity coordinate.
	for (unsigned int i = 1; i < EWVR_DeviceType_Count; i++)    // 0 is DeviceType_Invalid
	{
		uePose_pev[i].localPosition = uePose[i].localPosition;
		uePose_pev[i].localRotation = uePose[i].localRotation;
		unityPose_prev[i].pos = unityPose[i].pos;
		unityPose_prev[i].rot = unityPose[i].rot;
		EWVR_DeviceType dev_type = (EWVR_DeviceType)i;
		CurrentDoF[i] = UWaveVRBlueprintFunctionLibrary::GetSupportedNumOfDoF(dev_type);

		bPoseIsValid[i] = UWaveVRBlueprintFunctionLibrary::GetDevicePose(uePose[i].localPosition, uePose[i].localRotation, dev_type);
		if (bPoseIsValid[i])
		{
			unityPose[i].pos = CoordinateUtil::ConvertToUnityVector(uePose[i].localPosition, 1 / UWaveVRBlueprintFunctionLibrary::GetWorldToMetersScale());
			unityPose[i].rot = CoordinateUtil::ConvertToUnityQuaternion(uePose[i].localRotation);
		}
	}

	// Update final pose in Unreal coordinate.
	for (unsigned int i = 1; i < EWVR_DeviceType_Count; i++)    // 0 is DeviceType_Invalid
	{
		if (bPoseIsValid[i])
		{
			EWVR_DeviceType dev_type = (EWVR_DeviceType)i;
			if (dev_type == EWVR_DeviceType::DeviceType_HMD)
			{
				uePose[i].localPosition = CoordinateUtil::ConvertToUnrealVector(unityPose[i].pos, UWaveVRBlueprintFunctionLibrary::GetWorldToMetersScale());
				uePose[i].localRotation = CoordinateUtil::ConvertToUnrealRotator(unityPose[i].rot);
			}
			else
			{
				if (IsPlayInEditor())
				{
					bool _simulate_pose = false;

					if (enumUseSimulationPose == SimulatePosition::ForceSimulation ||
						(enumUseSimulationPose == SimulatePosition::WhenNoPosition &&
							CurrentDoF[i] == EWVR_DOF::DOF_3 || CurrentDoF[i] == EWVR_DOF::DOF_SYSTEM))
					{
						_simulate_pose = true;
					}

					if (_simulate_pose)
					{
						switch (dev_type)
						{
						case EWVR_DeviceType::DeviceType_Controller_Right:
							v3ChangeArmYAxis.X = 1;
							break;
						case EWVR_DeviceType::DeviceType_Controller_Left:
							v3ChangeArmYAxis.X = -1;
							break;
						default:
							break;
						}

						UpdateControllerPose(dev_type);

						simulateUEPosition = CoordinateUtil::ConvertToUnrealVector(simulateUnityPosition, UWaveVRBlueprintFunctionLibrary::GetWorldToMetersScale());
						simulateUERotation = CoordinateUtil::ConvertToUnrealRotator(simulateUnityQuaternion);

						uePose[i].localPosition = simulateUEPosition;
						uePose[i].localRotation = simulateUERotation;
					}
				}
			}
		}
		else
		{
			uePose[i].localPosition = uePose_pev[i].localPosition;
			uePose[i].localRotation = uePose_pev[i].localRotation;
		}
	}
}

void FWaveVRInput::UpdateControllerPose(EWVR_DeviceType hand)
{
	if (hand != EWVR_DeviceType::DeviceType_Controller_Left && hand != EWVR_DeviceType::DeviceType_Controller_Right)
		return;
	bodyRotation = FQuat::Identity;
	UpdateHeadAndBodyPose(hand);
	ComputeControllerUnityPose(unityPose[(unsigned int)hand]);
}

void FWaveVRInput::UpdateHeadAndBodyPose(EWVR_DeviceType hand)
{
	FVector gazeDirection = GetHeadUnityForward();
	gazeDirection.Y = 0.0f;
	gazeDirection.Normalize();

	float _bodyLerpFilter = BodyRotationFilter(unityPose_prev[(unsigned int)hand], unityPose[(unsigned int)hand]);
	if (_bodyLerpFilter > 0 && !FollowHead)
	{
		defaultHeadUnityPosition = unityPose[(unsigned int)EWVR_DeviceType::DeviceType_HMD].pos;
	}

	//bodyDirection = FMath::Lerp(bodyDirection, gazeDirection, _bodyLerpFilter);
	//bodyDirection = bodyDirection * (1.0f - _bodyLerpFilter) + gazeDirection * _bodyLerpFilter;
	//bodyDirection = UKismetMathLibrary::VLerp(bodyDirection, gazeDirection, _bodyLerpFilter);
	bodyDirection = VectorSlerp(bodyDirection, gazeDirection, _bodyLerpFilter);
	//bodyRotation = UKismetMathLibrary::FindLookAtRotation(UNITY_FORWARD, bodyDirection).Quaternion();
	bodyRotation = FromToRotation(UNITY_FORWARD, bodyDirection);
	if (WAVEVR_DEBUG)
	{
		LOGD(LogWaveVRInput, "UpdateHeadAndBodyPose() gazeDirection (%f, %f, %f)", gazeDirection.X, gazeDirection.Y, gazeDirection.Z);
		LOGD(LogWaveVRInput, "UpdateHeadAndBodyPose() bodyRotation (%f, %f, %f, %f)", bodyRotation.W, bodyRotation.X, bodyRotation.Y, bodyRotation.Z);
	}
}

float FWaveVRInput::BodyRotationFilter(RigidTransform pre_pose, RigidTransform pose)
{
	float _bodyLerpFilter = 0;

	FQuat _rot_old = pre_pose.rot;
	FQuat _rot_new = pose.rot;
	float _rot_XY_angle_old = 0, _rot_XY_angle_new = 0;

	FVector _rot_forward = FVector::ZeroVector;
	FQuat _rot_XY_rotation = FQuat::Identity;

	_rot_forward = _rot_old * UNITY_FORWARD;
	//_rot_XY_rotation = UKismetMathLibrary::FindLookAtRotation(UNITY_FORWARD, _rot_forward).Quaternion();
	_rot_XY_rotation = FromToRotation(UNITY_FORWARD, _rot_forward);
	_rot_XY_angle_old = QuaternionAngle(_rot_XY_rotation, FQuat::Identity);

	_rot_forward = _rot_new * UNITY_FORWARD;
	//_rot_XY_rotation = UKismetMathLibrary::FindLookAtRotation(UNITY_FORWARD, _rot_forward).Quaternion();
	_rot_XY_rotation = FromToRotation(UNITY_FORWARD, _rot_forward);
	_rot_XY_angle_new = QuaternionAngle(_rot_XY_rotation, FQuat::Identity);

	float _diff_angle = _rot_XY_angle_new - _rot_XY_angle_old;
	_diff_angle = _diff_angle > 0 ? _diff_angle : -_diff_angle;

	_bodyLerpFilter = FMath::Clamp<float>((_diff_angle - BodyAngleBound) / BodyAngleLimitation, 0, 1.0f);
	framesOfFreeze = _bodyLerpFilter < 1.0f ? framesOfFreeze + 1 : 0;
	if (WAVEVR_DEBUG)
	{
		LOGD(LogWaveVRInput, "BodyRotationFilter() _bodyLerpFilter %f, framesOfFreeze %d", _bodyLerpFilter, framesOfFreeze);
	}

	if (framesOfFreeze <= fFPS)
		return _bodyLerpFilter;
	else
		return 0;
}

/// <summary>
/// Get the simulated position of controller.
///
/// Consider the parts construct controller position:
/// Parts contain elbow, wrist and controller and each part has default offset from head.
/// 1. simulated elbow offset = default elbow offset apply body rotation = body rotation (Quaternion) * elbow offset (Vector3)
/// 2. simulated wrist offset = default wrist offset apply elbow rotation = elbow rotation (Quaternion) * wrist offset (Vector3)
/// 3. simulated controller offset = default controller offset apply wrist rotation = wrist rotation (Quat) * controller offset (V3)
/// head + 1 + 2 + 3 = controller position.
/// </summary>
/// <param name="rtPose">RigidTransform</param>
void FWaveVRInput::ComputeControllerUnityPose(RigidTransform rtPose)
{
	// if bodyRotation angle is θ, _inverseBodyRation is -θ
	// the operator * of Quaternion in Unity means concatenation, not multipler.
	// If quaternion qA has angle θ, quaternion qB has angle ε,
	// qA * qB will plus θ and ε which means rotating angle θ then rotating angle ε.
	// (_inverseBodyRotation * rotation of controller in world space) means angle ε subtracts angle θ.
	FQuat _controllerRotation = bodyRotation.Inverse() * rtPose.rot;
	FVector _headPosition = GetHeadUnityPosition();

	/// 1. simulated elbow offset = default elbow offset apply body rotation = body rotation (Quaternion) * elbow offset (Vector3)
	// Default left / right elbow offset.
	FVector _elbowOffset = UNITY_HEADTOELBOW_OFFSET * v3ChangeArmYAxis;
	// Default left / right elbow pitch offset.
	FVector _elbowPitchOffset = UNITY_ELBOW_PITCH_OFFSET * v3ChangeArmYAxis;

	// Use controller pitch to simulate elbow pitch.
	// Range from UNITY_ELBOW_PITCH_ANGLE_MIN ~ UNITY_ELBOW_PITCH_ANGLE_MAX.
	// The percent of pitch angle will be used to calculate the position offset.
	FVector _controllerForward = _controllerRotation * UNITY_FORWARD;
	float _controllerPitch = 90.0f - FMath::RadiansToDegrees(acosf(FVector::DotProduct(_controllerForward, UNITY_UP)));
	float _controllerPitchRadio = (_controllerPitch - UNITY_ELBOW_PITCH_ANGLE_MIN) / (UNITY_ELBOW_PITCH_ANGLE_MAX - UNITY_ELBOW_PITCH_ANGLE_MIN);
	_controllerPitchRadio = FMath::Clamp<float>(_controllerPitchRadio, 0.0f, 1.0f);

	// According to pitch angle percent, plus offset to elbow position.
	_elbowOffset += _elbowPitchOffset * _controllerPitchRadio;
	// Apply body rotation and head position to calculate final elbow position.
	_elbowOffset = _headPosition + bodyRotation * _elbowOffset;

	FVector _controllerRight = _controllerRotation * UNITY_RIGHT;
	/// 2. simulated wrist offset = default wrist offset apply elbow rotation = elbow rotation (Quaternion) * wrist offset (Vector3)
	// Rotation from Z-axis to XY-plane used to simulated elbow & wrist rotation.
	//FQuat _controllerXYRotation = UKismetMathLibrary::FindLookAtRotation(UNITY_FORWARD, _controllerForward).Quaternion();
	FQuat _controllerXYRotation = FromToRotation(UNITY_FORWARD, _controllerForward);
	float _xy_angle = QuaternionAngle(_controllerXYRotation, FQuat::Identity);
	float _controllerXYRotationRadio = _xy_angle / 180;
	// Simulate the elbow raising curve.
	float _elbowCurveLerpValue = ELBOW_TO_XYPLANE_LERP_MIN + (_controllerXYRotationRadio * (ELBOW_TO_XYPLANE_LERP_MAX - ELBOW_TO_XYPLANE_LERP_MIN));
	//FQuat _controllerXYLerpRotation = FQuat::Identity * (1.0f - _elbowCurveLerpValue) + _controllerXYRotation * _elbowCurveLerpValue;
	FQuat _controllerXYLerpRotation = QuaternionLerp(FQuat::Identity, _controllerXYRotation, _elbowCurveLerpValue);

	// Default left / right wrist offset
	FVector _wristOffset = UNITY_ELBOWTOWRIST_OFFSET * v3ChangeArmYAxis;
	// elbow rotation + curve = wrist rotation
	// wrist rotation = controller XY rotation
	// => elbow rotation + curve = controller XY rotation
	// => elbow rotation = controller XY rotation - curve
	FQuat _elbowRotation = bodyRotation * _controllerXYLerpRotation.Inverse() * _controllerXYRotation;
	// Apply elbow offset and elbow rotation to calculate final wrist position.
	_wristOffset = _elbowOffset + _elbowRotation * _wristOffset;


	/// 3. simulated controller offset = default controller offset apply wrist rotation = wrist rotation (Quat) * controller offset (V3)
	// Default left / right controller offset.
	FVector _controllerOffset = UNITY_WRISTTOCONTROLLER_OFFSET * v3ChangeArmYAxis;
	FQuat _wristRotation = _controllerXYRotation;
	// Apply wrist offset and wrist rotation to calculate final controller position.
	_controllerOffset = _wristOffset + _wristRotation * _controllerOffset;

	if (WAVEVR_DEBUG)
	{
		LOGD(LogWaveVRInput, "ComputeControllerPose() _elbowPitchOffset (%f, %f, %f)", _elbowPitchOffset.X, _elbowPitchOffset.Y, _elbowPitchOffset.Z);
		LOGD(LogWaveVRInput, "ComputeControllerPose() _controllerPitch: %f, _controllerPitchRadio: %f", _controllerPitch, _controllerPitchRadio);
		LOGD(LogWaveVRInput, "ComputeControllerPose() _elbowOffset (%f, %f, %f)", _elbowOffset.X, _elbowOffset.Y, _elbowOffset.Z);
		LOGD(LogWaveVRInput, "ComputeControllerPose() _wristOffset (%f, %f, %f)", _wristOffset.X, _wristOffset.Y, _wristOffset.Z);
		LOGD(LogWaveVRInput, "ComputeControllerPose() _controllerOffset (%f, %f, %f)", _controllerOffset.X, _controllerOffset.Y, _controllerOffset.Z);
	}

	simulateUnityPosition = /*bodyRotation */ _controllerOffset;
	simulateUnityQuaternion = bodyRotation * _controllerRotation;
}

FVector FWaveVRInput::GetHeadUEPosition()
{
	FVector _headpos = uePose[(unsigned int)EWVR_DeviceType::DeviceType_HMD].localPosition;
	return FollowHead ? _headpos : defaultHeadUEPosition;
}

FVector FWaveVRInput::GetHeadUnityPosition()
{
	FVector _headpos = unityPose[(unsigned int)EWVR_DeviceType::DeviceType_HMD].pos;
	return FollowHead ? _headpos : defaultHeadUnityPosition;
}

FQuat FWaveVRInput::GetHeadUnityRotation()
{
	FQuat _headrot = FQuat::Identity;
	if (bPoseIsValid[(unsigned int)EWVR_DeviceType::DeviceType_HMD])
	{
		_headrot = unityPose[(unsigned int)EWVR_DeviceType::DeviceType_HMD].rot;
	}
	return _headrot;
}

FVector FWaveVRInput::GetHeadUnityForward()
{
	FQuat _head_rot = GetHeadUnityRotation();
	FVector _forward = _head_rot * UNITY_FORWARD;
	return _forward;
}

bool FWaveVRInput::IsLeftHandedMode()
{
	return bIsLeftHanded;
}

EWVR_DeviceType FWaveVRInput::GetLeftHandedDevice(EControllerHand hand)
{
	EWVR_DeviceType adaptive_device = EWVR_DeviceType::DeviceType_HMD;
	switch (hand)
	{
	case EControllerHand::Right:
		adaptive_device = bIsLeftHanded ? EWVR_DeviceType::DeviceType_Controller_Left : EWVR_DeviceType::DeviceType_Controller_Right;
		break;
	case EControllerHand::Left:
		adaptive_device = bIsLeftHanded ? EWVR_DeviceType::DeviceType_Controller_Right : EWVR_DeviceType::DeviceType_Controller_Left;
		break;
	default:
		break;
	}

	return adaptive_device;
}

void FWaveVRInput::UseSimulationPose(SimulatePosition simulation)
{
	LOGD(LogWaveVRInput, "UseSimulationPose() simulation: %d.", (uint8)simulation);
	enumUseSimulationPose = simulation;
	UWaveVRBlueprintFunctionLibrary::SetPoseSimulationOption(enumUseSimulationPose);
}

void FWaveVRInput::FollowHeadPosition(bool follow)
{
	FollowHead = follow;
	UWaveVRBlueprintFunctionLibrary::SetFollowHead(follow);
}

void FWaveVRInput::UpdateUnitySimulationSettingsFromJson(
	FVector HEADTOELBOW_OFFSET,
	FVector ELBOWTOWRIST_OFFSET,
	FVector WRISTTOCONTROLLER_OFFSET,
	FVector ELBOW_PITCH_OFFSET,
	float ELBOW_PITCH_ANGLE_MIN,
	float ELBOW_PITCH_ANGLE_MAX)
{
	LOGD(LogWaveVRInput, "UpdateUnitySimulationSettingsFromJson() Before update:");
	LOGD(LogWaveVRInput, "UNITY_HEADTOELBOW_OFFSET (%f, %f, %f)", UNITY_HEADTOELBOW_OFFSET.X, UNITY_HEADTOELBOW_OFFSET.Y, UNITY_HEADTOELBOW_OFFSET.Z);
	LOGD(LogWaveVRInput, "UNITY_ELBOWTOWRIST_OFFSET (%f, %f, %f)", UNITY_ELBOWTOWRIST_OFFSET.X, UNITY_ELBOWTOWRIST_OFFSET.Y, UNITY_ELBOWTOWRIST_OFFSET.Z);
	LOGD(LogWaveVRInput, "WRISTTOCONTROLLER_OFFSET (%f, %f, %f)", WRISTTOCONTROLLER_OFFSET.X, WRISTTOCONTROLLER_OFFSET.Y, WRISTTOCONTROLLER_OFFSET.Z);
	LOGD(LogWaveVRInput, "ELBOW_PITCH_OFFSET (%f, %f, %f)", ELBOW_PITCH_OFFSET.X, ELBOW_PITCH_OFFSET.Y, ELBOW_PITCH_OFFSET.Z);
	LOGD(LogWaveVRInput, "ELBOW_PITCH_ANGLE_MIN (%f), ELBOW_PITCH_ANGLE_MAX (%f)", ELBOW_PITCH_ANGLE_MIN, ELBOW_PITCH_ANGLE_MAX);

	UNITY_HEADTOELBOW_OFFSET = HEADTOELBOW_OFFSET;
	UNITY_ELBOWTOWRIST_OFFSET = ELBOWTOWRIST_OFFSET;
	UNITY_WRISTTOCONTROLLER_OFFSET = WRISTTOCONTROLLER_OFFSET;
	UNITY_ELBOW_PITCH_OFFSET = ELBOW_PITCH_OFFSET;
	UNITY_ELBOW_PITCH_ANGLE_MIN = ELBOW_PITCH_ANGLE_MIN;
	UNITY_ELBOW_PITCH_ANGLE_MAX = ELBOW_PITCH_ANGLE_MAX;

	LOGD(LogWaveVRInput, "UpdateUnitySimulationSettingsFromJson() After update:");
	LOGD(LogWaveVRInput, "UNITY_HEADTOELBOW_OFFSET (%f, %f, %f)", UNITY_HEADTOELBOW_OFFSET.X, UNITY_HEADTOELBOW_OFFSET.Y, UNITY_HEADTOELBOW_OFFSET.Z);
	LOGD(LogWaveVRInput, "UNITY_ELBOWTOWRIST_OFFSET (%f, %f, %f)", UNITY_ELBOWTOWRIST_OFFSET.X, UNITY_ELBOWTOWRIST_OFFSET.Y, UNITY_ELBOWTOWRIST_OFFSET.Z);
	LOGD(LogWaveVRInput, "WRISTTOCONTROLLER_OFFSET (%f, %f, %f)", WRISTTOCONTROLLER_OFFSET.X, WRISTTOCONTROLLER_OFFSET.Y, WRISTTOCONTROLLER_OFFSET.Z);
	LOGD(LogWaveVRInput, "ELBOW_PITCH_OFFSET (%f, %f, %f)", ELBOW_PITCH_OFFSET.X, ELBOW_PITCH_OFFSET.Y, ELBOW_PITCH_OFFSET.Z);
	LOGD(LogWaveVRInput, "ELBOW_PITCH_ANGLE_MIN (%f), ELBOW_PITCH_ANGLE_MAX (%f)", ELBOW_PITCH_ANGLE_MIN, ELBOW_PITCH_ANGLE_MAX);
}
#pragma endregion Controller Position Simulation
