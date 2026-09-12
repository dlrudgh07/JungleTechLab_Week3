#pragma once
#include "Core.h"

enum class EAxis : int { X = 0, Y = 1, Z = 2 };

enum class EPrimitive
{
	EP_Sphere,
	EP_Cube,
	EP_Triangle,
	EP_GizmoArrow,
	EP_Circle,
};

enum EGIZMO_AXIS //어떤축이 선택되었는지
{
	NONE,
	X,
	Y,
	Z
};

enum EGIZMO_TYPE {
	TRANSLATE,
	ROTATE,
	SCALE,
};

enum class EViewModeIndex : uint32  // 오직하나만 있어야하는 친구들
{
	VMI_Lit, //week3 미구현
	VMI_Unlit, //week3 미구현
	VMI_Wireframe,
};

enum class EEngineShowFlags : uint64 // 여러개가 동시일수도 있는 친구들(비트마스크)
{
	SF_NONE= 0,
	SF_Primitives = 1ull <<0,
	SF_BillboardText =1ull<< 1,
	SF_WorldAxis = 1ull << 2,
	SF_Grid = 1ull <<3,
	SF_BoundingBoxes = 1ull << 4,
	SF_Gizmo = 1ull << 5,
	SF_UUID = 1ull << 6,

	// u = unsinged int
	// ll= long long
	// ull unsinged long long(=uint64)
};

// 연산자 오버로딩
	inline constexpr EEngineShowFlags operator|(EEngineShowFlags a, EEngineShowFlags b)
	{
		return static_cast<EEngineShowFlags>(
			static_cast<uint64>(a) | static_cast<uint64>(b));
	}

	inline constexpr EEngineShowFlags operator&(EEngineShowFlags a, EEngineShowFlags b)
	{
		return static_cast<EEngineShowFlags>(
			static_cast<uint64>(a) & static_cast<uint64>(b));
	}

	inline constexpr EEngineShowFlags operator~(EEngineShowFlags a)
	{
		return static_cast<EEngineShowFlags>(~static_cast<uint64>(a));
	}

	inline EEngineShowFlags& operator|=(EEngineShowFlags & a, EEngineShowFlags b)
	{
		a = a | b;
		return a;
	}

	inline EEngineShowFlags& operator&=(EEngineShowFlags & a, EEngineShowFlags b)
	{
		a = a & b;
		return a;
	}
	inline constexpr bool HasFlag(EEngineShowFlags flags, EEngineShowFlags test) //Enum 등호비교를 위한 함수
	{
		return (flags & test) != EEngineShowFlags::SF_NONE;
	}
