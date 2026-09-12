#pragma once
#include "Renderer.h"

// 정점 버퍼 (6개 -> 4개로 최적화)
inline FVertexSimple Quad_vertices[4] = {
	// 위치(x,y,z)           색상(R,G,B,A)             UV(U,V)
	{ -0.5f, 0.0f,  0.5f,  0.6f, 1.0f, 1.0f, 1.0f,  0.0f, 0.0f }, // 0: 좌상단 (TL)
	{ 0.5f, 0.0f,  0.5f,  1.0f, 0.6f, 1.0f, 1.0f,  1.0f, 0.0f }, // 1: 우상단 (TR)
	{ -0.5f, 0.0f, -0.5f,  0.6f, 1.0f, 1.0f, 1.0f,  0.0f, 1.0f }, // 2: 좌하단 (BL)
	{ 0.5f, 0.0f, -0.5f,  1.0f, 1.0f, 0.6f, 1.0f,  1.0f, 1.0f }  // 3: 우하단 (BR)
};

// 인덱스 버퍼 (총 6개, 삼각형 2개)
inline uint32 Quad_indices[6] = {
	0, 1, 2,  // 첫 번째 삼각형 (TL, TR, BL)
	2, 1, 3   // 두 번째 삼각형 (BL, TR, BR)
};
