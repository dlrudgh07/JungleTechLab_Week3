#pragma once
#include "Vector.h"
#include "Matrix.h"
#include <cmath>
struct FQuaternion
{
	float X, Y, Z, W;
	//=i,j,k,w
	FQuaternion() : X(0), Y(0), Z(0), W(1) {}                 // identity
	FQuaternion(float InX, float InY, float InZ, float InW) : X(InX), Y(InY), Z(InZ), W(InW) {}
	FQuaternion(const FVector4& Q)  
	{
		// 생성시 무조건 정규화
		FVector Axis(Q.x, Q.y, Q.z);
		if (Axis.IsNearlyZero()) { X = 0; Y = 0; Z = 0; W = 1; return; }
		Axis.Normalize();
		float S, C;
		FMath::sincos<float>(S, C, FMath::DegreesToRadians(Q.w) * 0.5f);
		X = Axis.x * S; Y = Axis.y * S; Z = Axis.z * S; W = C;
	}
	FQuaternion operator*(const FQuaternion& Q) const        // #3: 실수부 = ww' − v⃗·v⃗', 벡터부 = wv⃗' + w'v⃗ + v⃗×v⃗'
	{
		FQuaternion R;
		R.W = W * Q.W - X * Q.X - Y * Q.Y - Z * Q.Z;
		R.X = W * Q.X + X * Q.W + Y * Q.Z - Z * Q.Y;
		R.Y = W * Q.Y - X * Q.Z + Y * Q.W + Z * Q.X;
		R.Z = W * Q.Z + X * Q.Y - Y * Q.X + Z * Q.W;
		return R;
	}

	void Normalize()
	{
		const float Scale = 1.0f / FMath::Sqrt(X * X + Y * Y + Z * Z + W * W);
		X *= Scale;Y *= Scale;Z *= Scale; W *= Scale;
	}
	void NormalizeAxis()
	{
		const float Scale = 1.0f / FMath::Sqrt(X * X + Y * Y + Z * Z);
		X *= Scale;Y *= Scale;Z *= Scale; 

	}
	FQuaternion Conjugate() const                        // #5: (-X,-Y,-Z, W)
	{
		return FQuaternion(-X, -Y, -Z, W);
	}


	 
	FMatrix ToMatrix() const
	{
		const float X2 = X + X, Y2 = Y + Y, Z2 = Z + Z;
		const float XX = X * X2, YY = Y * Y2, ZZ = Z * Z2;
		const float XY = X * Y2, XZ = X * Z2, YZ = Y * Z2;
		const float WX = W * X2, WY = W * Y2, WZ = W * Z2;

		FMatrix R = FMatrix::Identity;
		R.M[0][0] = 1.0f - (YY + ZZ);
		R.M[0][1] = XY + WZ;
		R.M[0][2] = XZ - WY;

		R.M[1][0] = XY - WZ;
		R.M[1][1] = 1.0f - (XX + ZZ);
		R.M[1][2] = YZ + WX;

		R.M[2][0] = XZ + WY;
		R.M[2][1] = YZ - WX;
		R.M[2][2] = 1.0f - (XX + YY);
		return R;
	}


	
	static FMatrix QuatRotateToMatrix(const FVector3 Q,float degree) //x,y,z,degree
	{
		return FQuaternion(FVector4(Q.x, Q.y, Q.z, degree)).ToMatrix();

	}

	FQuaternion ReverseRotate() 
	{
		return FQuaternion(X, Y, Z, -W);

	}

	//qvq* 최적화 구현함수
	static FVector qvq(FQuaternion q, FVector v)
	{
		const FVector Q(q.X, q.Y, q.Z);
		const FVector T = 2.0f * FVector::cross(Q, v);
		return v + q.W * T + FVector::cross(Q, T);	
	}

	// 각,축,점을 넣었을때 회전이후 점이 나오는 함수
	// 단, 각은 Degree, 축은 정규화 하지 않아도됨.
	static FVector3 QuaternionRotation(float degree, const FVector3 axis,FVector3 point) 
	{
		return qvq(FQuaternion(FVector4(axis.x, axis.y, axis.z, degree)), point);
	}
	static FVector3 QuaternionRotationRadian(float radian, const FVector3 axis, FVector3 point)
	{
		float degree = radian * 180 / PI;
		return qvq(FQuaternion(FVector4(axis.x, axis.y, axis.z, degree)), point);
	}
	// 축 하나만 도는 쿼터니언. FromEuler 와 같은 부호 규약(Pitch/Roll 반전)을 쓴다.
	static FQuaternion MakeYaw(float Degree) { return FQuaternion(FVector4(0, 0, 1, Degree)); }
	static FQuaternion MakePitch(float Degree) { return FQuaternion(FVector4(0, 1, 0, -Degree)); }
	static FQuaternion MakeRoll(float Degree) { return FQuaternion(FVector4(1, 0, 0, -Degree)); }

	// 90도 고정 버전
	static FQuaternion Yaw90() { return MakeYaw(90.0f); }
	static FQuaternion Pitch90() { return MakePitch(90.0f); }
	static FQuaternion Roll90() { return MakeRoll(90.0f); }

// 기존 FMatrix::Rotate(P,Y,R)와 같은 회전. RotateX/Y가 왼손이라 Pitch/Roll 부호 반전
	static FQuaternion FromEuler(const FRotator& R)
	{
	return FQuaternion(FVector4(0, 0, 1, R.Yaw))
		* FQuaternion(FVector4(0, 1, 0, -R.Pitch))
		* FQuaternion(FVector4(1, 0, 0, -R.Roll));
	}

	// 쿼터니언 -> 오일러(도). FromEuler 의 역. 인스펙터 표시/JSON 저장용.
	// Pitch ±90° 근처에서는 Yaw/Roll 이 불안정(짐벌락)하므로 계산에는 쓰지 말 것.
	FRotator ToEuler() const
	{
		const float M00 = 1.0f - 2.0f * (Y * Y + Z * Z);
		const float M01 = 2.0f * (X * Y + W * Z);
		const float M02 = 2.0f * (X * Z - W * Y);
		const float M12 = 2.0f * (Y * Z + W * X);
		const float M22 = 1.0f - 2.0f * (X * X + Y * Y);
		return FRotator(
			FMath::RadiansToDegrees(asinf(FMath::Clamp(M02, -1.0f, 1.0f))),   // Pitch
			FMath::RadiansToDegrees(atan2f(M01, M00)),                         // Yaw
			FMath::RadiansToDegrees(atan2f(-M12, M22)));                       // Roll
	}

};
