#pragma once
#include "Renderer.h"
#include "TArray.h"

//컴포넌트 인스턴스 없이도 사용할수있게 따로분리


bool IsSameVertex(const FVertexSimple& A, const FVertexSimple& B);

void Welding(const FVertexSimple* InVertices, uint32 InVertexCount,
	TArray<FVertexSimple>& OutVertices, TArray<uint32>& OutIndices);	 //Welding : 정점을 줄이고 인덱스를 만드는것
