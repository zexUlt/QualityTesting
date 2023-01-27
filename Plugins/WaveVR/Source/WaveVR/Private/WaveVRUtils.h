// "WaveVR SDK
// © 2019 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#pragma once

#include "CoreMinimal.h"
#include "wvr.h"
#include "Kismet/KismetMathLibrary.h"

namespace wvr {
	namespace utils {
		// No coordination system transfrom
		static FORCEINLINE FMatrix ToFMatrix(const WVR_Matrix4f_t& tm)
		{
			// The FMatrix's column is mathematical matrix's row, and the FMatrix's row is mathematical matrix's column.
			// FPlane is as the column in mathmatic, but is as the row in FMatrix.
			return FMatrix(
				FPlane(tm.m[0][0], tm.m[0][1], tm.m[0][2], tm.m[0][3]),
				FPlane(tm.m[1][0], tm.m[1][1], tm.m[1][2], tm.m[1][3]),
				FPlane(tm.m[2][0], tm.m[2][1], tm.m[2][2], tm.m[2][3]),
				FPlane(tm.m[3][0], tm.m[3][1], tm.m[3][2], tm.m[3][3]));
		}

		// No coordination system transfrom
		static FORCEINLINE WVR_Matrix4f_t ToWVRMatrix(const FMatrix& mat)
		{
			WVR_Matrix4f_t m {};
			m.m[0][0] = mat.M[0][0];
			m.m[0][1] = mat.M[0][1];
			m.m[0][2] = mat.M[0][2];
			m.m[0][3] = mat.M[0][3];

			m.m[1][0] = mat.M[1][0];
			m.m[1][1] = mat.M[1][1];
			m.m[1][2] = mat.M[1][2];
			m.m[1][3] = mat.M[1][3];

			m.m[2][0] = mat.M[2][0];
			m.m[2][1] = mat.M[2][1];
			m.m[2][2] = mat.M[2][2];
			m.m[2][3] = mat.M[2][3];

			m.m[3][0] = mat.M[3][0];
			m.m[3][1] = mat.M[3][1];
			m.m[3][2] = mat.M[3][2];
			m.m[3][3] = mat.M[3][3];

			return m;
		}

		// If the input f16 array is column major, set columnMajor true.
		static FORCEINLINE FMatrix FromGLToUnrealMatrix(const float * f16, bool columnMajor, float meterToWorldUnit)
		{
			// The FMatrix's column is mathematical matrix's row, and the FMatrix's row is mathematical matrix's column.
			if (columnMajor)
				return FMatrix(
					FPlane( f16[10], -f16[ 8], -f16[ 9], -f16[11]),
					FPlane(-f16[ 2],  f16[ 0],  f16[ 1],  f16[ 3]),
					FPlane(-f16[ 6],  f16[ 4],  f16[ 5],  f16[ 7]),
					FPlane(-f16[14] * meterToWorldUnit,  f16[12] * meterToWorldUnit,  f16[13] * meterToWorldUnit,  f16[15]));
			else
				return FMatrix(
					FPlane( f16[10], -f16[2], -f16[ 6], -f16[14]),
					FPlane(-f16[ 8],  f16[0],  f16[ 4],  f16[12]),
					FPlane(-f16[ 9],  f16[1],  f16[ 5],  f16[13]),
					FPlane(-f16[11] * meterToWorldUnit,  f16[3] * meterToWorldUnit,  f16[ 7] * meterToWorldUnit,  f16[15]));
		}

		// coordination system transfrom
		static FORCEINLINE FMatrix FromGLToUnreal(const WVR_Matrix4f_t& tm, float meterToWorldUnit)
		{
			// WVR_Matrix4f_t is row major.
			return FromGLToUnrealMatrix(&(tm.m[0][0]), false, meterToWorldUnit);
		}

		static FORCEINLINE FMatrix FromGLToUnreal(const FMatrix& m, float meterToWorldUnit)
		{
			// FMatrix is row major.
			return FromGLToUnrealMatrix(&(m.M[0][0]), false, meterToWorldUnit);
		}

		static FORCEINLINE FVector FromGLToUnrealVector(const float * f3, float meterToWorldUnit)
		{
			return FVector(-f3[2] * meterToWorldUnit, f3[0] * meterToWorldUnit, f3[1] * meterToWorldUnit);
		}

		static FORCEINLINE FVector FromGLToUnreal(const WVR_Vector3f_t& v, float meterToWorldUnit)
		{
			return FromGLToUnrealVector(v.v, meterToWorldUnit);
		}

		static FORCEINLINE FVector FromGLToUnreal(const FVector& v, float meterToWorldUnit)
		{
			return FVector(-v.Z * meterToWorldUnit, v.X * meterToWorldUnit, v.Y * meterToWorldUnit);
		}

		static FORCEINLINE FRotator FromGLToUnrealEuler(const float * f3)
		{
			// FRotator is right hand "Roll-forward, Yaw-down, Pitch-right" coordinate system, and applies right hand rules.
			// FRotator's header have description about Yaw-Pitch-Roll, but I don't think it's correct.
			// You can try it in the UE4 editor.  When you rotate an object in xyz, it follows my definition.
			// https://www.mathworks.com/help/uav/ug/coordinate-systems-for-unreal-engine-simulation-in-uav-toolbox.html#mw_9fd3915e-88af-4953-b7f0-1cc028c6503a
			return FRotator(f3[0], -f3[1], -f3[2]);
		}

		static FORCEINLINE FRotator FromGLToUnrealEuler(const WVR_Vector3f_t& v)
		{
			return FromGLToUnrealEuler(v.v);
		}

		class WaveVRUtils {
		public:
			static void MatrixToQuat(WVR_Quatf *q, const WVR_Matrix4f_t &mx);
			static void MatrixToQuat(const WVR_Matrix4f_t &mx, WVR_Quatf& q);
			static WVR_Quatf MatrixToQuat(const WVR_Matrix4f_t &mx);

			static void ConvertWVRPosePairToUnrealPose(const WVR_DevicePosePair_t& InPose, float meterToWorldUnit, FQuat& OutOrientation, FVector& OutPosition);
			static void ConvertWVRMatrixToUnrealPose(const WVR_Matrix4f_t& InPoseMatrix, float meterToWorldUnit, FQuat& OutOrientation, FVector& OutPosition);

			static void CoordinatTransform(const WVR_Matrix4f_t& InPoseMatrix, FQuat& OutOrientation, FVector& OutPosition);
			static void CoordinateTransform(const WVR_Matrix4f_t& InPoseMatrix, float meterToWorldUnit, FQuat& OutOrientation, FVector& OutPosition);

			static bool VerifyMatrixQuality(const WVR_Matrix4f_t& InPoseMatrix);
		};

		class NativeModelParameter {
		public:
			static void SetSpawnFlag(bool flag);
			static bool GetSpawnFlag();
			static void SetSpawnDeviceType(WVR_DeviceType deviceType);
			static WVR_DeviceType GetSpawnDeviceType();
			static void SetAlwaysShow(bool show);
			static bool GetAlwaysShow();
			static void SetHideWhenIdle(bool hide) { HideWhenIdle = hide; }
			static bool GetHideWhenIdle() { return HideWhenIdle; }

		private:
			static bool mSpawnControllerFlag;
			static WVR_DeviceType mDeviceType;
			static bool mAlwaysShow;
			static bool HideWhenIdle;
		};

		// Game thread coordinate APIs.
		class CoordinateUtil {
		public:

			static FVector2D GetVector2(const FVector2D& wvr_axis, float WorldToMetersScale) {
				FVector2D vec2D = FVector2D::ZeroVector;

				vec2D.X = -wvr_axis.X * WorldToMetersScale;
				vec2D.Y = wvr_axis.Y * WorldToMetersScale;
				return vec2D;
			}

			static FVector2D GetVector2(const WVR_Vector2f_t& wvr_axis, float WorldToMetersScale) {
				FVector2D vec2D = FVector2D::ZeroVector;
				vec2D.X = -wvr_axis.v[0] * WorldToMetersScale;
				vec2D.Y = wvr_axis.v[1] * WorldToMetersScale;
				return vec2D;
			}

			static FVector GetVector3(const FVector& wvr_position, float WorldToMetersScale) {
				FVector vec = FVector::ZeroVector;

				vec.X = -wvr_position.Z * WorldToMetersScale;
				vec.Y = wvr_position.X * WorldToMetersScale;
				vec.Z = wvr_position.Y * WorldToMetersScale;
				return vec;
			}

			static FVector GetVector3(const WVR_Vector3f_t& wvr_position, float WorldToMetersScale)
			{
				FVector vec = FVector::ZeroVector;
				vec.X = -wvr_position.v[2] * WorldToMetersScale;
				vec.Y = wvr_position.v[0] * WorldToMetersScale;
				vec.Z = wvr_position.v[1] * WorldToMetersScale;
				return vec;
			}

			static void Vector3NormalizeX(FVector& vec)
			{
				if (vec.X == 0) { return; }
				float normal = vec.X < 0 ? -vec.X : vec.X;
				vec.X /= normal;
				vec.Y /= normal;
				vec.Z /= normal;
			}

			static FQuat GetQuaternion(const WVR_Quatf& wvr_rotation)
			{
				FQuat outQuat = outQuat.Identity;
				outQuat.X = -wvr_rotation.z;
				outQuat.Y = wvr_rotation.x;
				outQuat.Z = wvr_rotation.y;
				outQuat.W = -wvr_rotation.w;
				return outQuat;
			}

			// From GL to Unreal
			static FRotator GetEulerRotator(const WVR_Vector3f_t& eulerAngles)
			{
				return FRotator(eulerAngles.v[0], eulerAngles.v[1], -eulerAngles.v[2]);
			}

			static WVR_Quatf MatrixToQuat(const WVR_Matrix4f_t &mx) {
				WVR_Quatf q;
				float trace = mx.m[0][0] + mx.m[1][1] + mx.m[2][2];
				if (trace > 0) {
					float s = 0.5f / sqrtf(trace + 1.0f);
					q.w = 0.25f / s;
					q.x = (mx.m[2][1] - mx.m[1][2]) * s;
					q.y = (mx.m[0][2] - mx.m[2][0]) * s;
					q.z = (mx.m[1][0] - mx.m[0][1]) * s;
				}
				else {
					if (mx.m[0][0] > mx.m[1][1] && mx.m[0][0] > mx.m[2][2]) {
						float s = 2.0f * sqrtf(1.0f + mx.m[0][0] - mx.m[1][1] - mx.m[2][2]);
						if (s > 0) {
							q.w = (mx.m[2][1] - mx.m[1][2]) / s;
							q.x = 0.25f * s;
							q.y = (mx.m[0][1] + mx.m[1][0]) / s;
							q.z = (mx.m[0][2] + mx.m[2][0]) / s;
						}
					}
					else if (mx.m[1][1] > mx.m[2][2]) {
						float s = 2.0f * sqrtf(1.0f + mx.m[1][1] - mx.m[0][0] - mx.m[2][2]);
						if (s > 0) {
							q.w = (mx.m[0][2] - mx.m[2][0]) / s;
							q.x = (mx.m[0][1] + mx.m[1][0]) / s;
							q.y = 0.25f * s;
							q.z = (mx.m[1][2] + mx.m[2][1]) / s;
						}
					}
					else {
						float s = 2.0f * sqrtf(1.0f + mx.m[2][2] - mx.m[0][0] - mx.m[1][1]);
						if (s > 0) {
							q.w = (mx.m[1][0] - mx.m[0][1]) / s;
							q.x = (mx.m[0][2] + mx.m[2][0]) / s;
							q.y = (mx.m[1][2] + mx.m[2][1]) / s;
							q.z = 0.25f * s;
						}
					}
				}
				return q;
			}

			static void MatrixToPose(const WVR_Matrix4f_t& InPoseMatrix, FQuat& OutOrientation, FVector& OutPosition, float WorldToMetersScale) {
				WVR_Quatf Quat = MatrixToQuat(InPoseMatrix);

				// GL space to Unreal space
				OutOrientation.X = -Quat.z;
				OutOrientation.Y = Quat.x;
				OutOrientation.Z = Quat.y;
				OutOrientation.W = -Quat.w;
				OutOrientation.Normalize();

				FVector position = FVector(InPoseMatrix.m[0][3], InPoseMatrix.m[1][3], InPoseMatrix.m[2][3]);
				OutPosition = GetVector3(position, WorldToMetersScale);
			}

			static float angleBetweenVector(FVector vector1, FVector vector2)
			{
				vector1.Normalize();
				vector2.Normalize();
				float dot_product = FVector::DotProduct(vector1, vector2);
				float angle = UKismetMathLibrary::DegAcos(dot_product);
				return angle;
			}

			static FRotator getVectorRotation(FVector vec)
			{
				FRotator rot = FRotator::ZeroRotator;
				rot.Yaw = angleBetweenVector(vec, FVector::RightVector);
				rot.Pitch = angleBetweenVector(vec, FVector::ForwardVector);
				rot.Roll = angleBetweenVector(vec, FVector::UpVector);
				return rot;
			}

			static FVector ConvertToUnityVector(float x, float y, float z, float WorldToMetersScale)
			{
				FVector _result;

				/// Unity.X = UE.Y
				/// Unity.Y = UE.Z
				/// Unity.Z = UE.X
				_result.X = y * WorldToMetersScale;
				_result.Y = z * WorldToMetersScale;
				_result.Z = x * WorldToMetersScale;

				return _result;
			}

			static FVector ConvertToUnityVector(FVector vec, float WorldToMetersScale)
			{
				FVector _result = ConvertToUnityVector(vec.X, vec.Y, vec.Z, WorldToMetersScale);
				return _result;
			}

			// Unity to Unreal
			static FVector ConvertToUnrealVector(float x, float y, float z, float WorldToMetersScale)
			{
				FVector _result;

				/// UE.X = Unity.Z
				/// UE.Y = Unity.X
				/// UE.Z = Unity.Y
				_result.X = z * WorldToMetersScale;
				_result.Y = x * WorldToMetersScale;
				_result.Z = y * WorldToMetersScale;

				return _result;
			}

			static FVector ConvertToUnrealVector(FVector vec, float WorldToMetersScale)
			{
				FVector _result = ConvertToUnrealVector(vec.X, vec.Y, vec.Z, WorldToMetersScale);
				return _result;
			}

			static FQuat ConvertToUnityQuaternion(float w, float x, float y, float z)
			{
				FQuat _result = FQuat(y, z, x, w);
				return _result;
			}

			static FQuat ConvertToUnityQuaternion(FQuat quat)
			{
				FQuat _result = ConvertToUnityQuaternion(quat.W, quat.X, quat.Y, quat.Z);
				return _result;
			}

			static FQuat ConvertToUnityQuaternion(FRotator rotator)
			{
				FQuat _quat = rotator.Quaternion();
				FQuat _result = ConvertToUnityQuaternion(_quat);
				return _result;
			}

			static FRotator ConvertToUnrealRotator(float w, float x, float y, float z)
			{
				FQuat _result = FQuat(z, x, y, w);
				return _result.Rotator();
			}

			static FRotator ConvertToUnrealRotator(FQuat quat)
			{
				FRotator _result = ConvertToUnrealRotator(quat.W, quat.X, quat.Y, quat.Z);
				return _result;
			}

			static FRotator ConvertToUnrealRotator(FRotator rotator)
			{
				FQuat _quat = rotator.Quaternion();
				FRotator _result = ConvertToUnrealRotator(_quat);
				return _result;
			}

			// FQuat(X, Y, Z, W) => Plane: Xx + Yy + Zz + W = 0
			static FQuat GetPlaneEquation(FVector p1, FVector p2, FVector p3)
			{
				FQuat plane = FQuat::Identity;

				FVector vec_p1p2 = FVector::ZeroVector;
				vec_p1p2.X = p2.X - p1.X;
				vec_p1p2.Y = p2.Y - p1.Y;
				vec_p1p2.Z = p2.Z - p1.Z;

				FVector vec_p1p3 = FVector::ZeroVector;
				vec_p1p3.X = p3.X - p1.X;
				vec_p1p3.Y = p3.Y - p1.Y;
				vec_p1p3.Z = p3.Z - p1.Z;

				FVector vec_cross_product = FVector::CrossProduct(vec_p1p2, vec_p1p3);

				plane.X = vec_cross_product.X;
				plane.Y = vec_cross_product.Y;
				plane.Z = vec_cross_product.Z;
				plane.W = -(vec_cross_product.X * p1.X) - (vec_cross_product.Y * p1.Y) - (vec_cross_product.Z * p1.Z);

				return plane;
			}

			// FVector2D(X, Y) => (u, v) coordinate: (X, Y)
			static FVector2D GetPlaneUV(FQuat qat_plane, FVector right_vector, FVector point)
			{
				FVector2D vec_uv = FVector2D::ZeroVector;

				right_vector.Normalize();
				FVector plane_vec = FVector(qat_plane.X, qat_plane.Y, qat_plane.Z);
				FVector u_vec = right_vector;
				FVector v_vec = FVector::CrossProduct(plane_vec, u_vec);

				vec_uv.X = FVector::DotProduct(u_vec, point);
				vec_uv.Y = FVector::DotProduct(v_vec, point);

				return vec_uv;
			}
		};
	}
}
