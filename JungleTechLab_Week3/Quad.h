#pragma once
#include "Renderer.h"

// x축을 바라보는 쿼드
inline FVertexSimple Quad_vertices[6] = {
	//   x     y      z      r     g    b    a     u     v
	{ 0.0f, -0.5f,  0.5f,  0.6f, 1.f, 1.f, 1.f,  0.0f, 0.0f }, // 좌상단
	{ 0.0f,  0.5f,  0.5f,  1.f, 0.6f, 1.f, 1.f,  1.0f, 0.0f }, // 우상단
	{ 0.0f, -0.5f, -0.5f,  0.6f, 1.f, 1.f, 1.f,  0.0f, 1.0f }, // 좌하단
	{ 0.0f, -0.5f, -0.5f,  1.f, 1.f, 0.6f, 1.f,  0.0f, 1.0f }, // 좌하단
	{ 0.0f,  0.5f,  0.5f,  0.6f, 1.f, 1.f, 1.f,  1.0f, 0.0f }, // 우상단
	{ 0.0f,  0.5f, -0.5f,  1.f, 1.f, 0.6f, 1.f,  1.0f, 1.0f }  // 우하단
};
