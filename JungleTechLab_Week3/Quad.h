#pragma once

#include "Renderer.h"

// 삼각형 두개로 만드는 Quad이다.
// 첫 생성 용도는 텍스트를 렌더하기 위함이다.

inline FVertexSimple Quad_vertices[] =
{
	{ 0.0f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.f, 0.f }, // Top-left R
	{ 0.0f, 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.f, 0.f }, // Top-right G
	{ 0.0f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.f, 1.f }, // Bottom-left vertex B
	{ 0.0f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.f, 0.f }, // Top-right vertex G
	{ 0.0f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.f, 1.f }, // Bottm_right R
	{ 0.0f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.f, 1.f }, // Bottom-left B
}; 
//
//inline FVertexSimple Quad_vertices[] =
//{
//	{ 0.0f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.f, 0.f }, // Top-left
//	{ 0.0f, 0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.f, 0.f }, // Top-right
//	{ 0.0f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.f, 1.f }, // Bottom-left vertex
//	{ 0.0f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.f, 1.f }, // Bottom-left
//	{ 0.0f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.f, 0.f }, // Top-right vertex
//	{ 0.0f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.f, 1.f }, // Bottm_right
//};
//
//inline FVertexSimple Quad_vertices[] =
//{
//	{ 0.0f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.f, 0.f }, // Top-left
//	{ 0.0f, 0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.f, 0.f }, // Top-right
//	{ 0.0f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.f, 1.f }, // Bottom-left vertex
//	{ 0.0f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.f, 1.f }, // Bottm_right
//	{ 0.0f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.f, 1.f }, // Bottom-left
//	{ 0.0f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.f, 0.f }, // Top-right vertex
//};
