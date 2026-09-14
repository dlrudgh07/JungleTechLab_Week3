#include "StaticMesh.h"
#include "Renderer.h"


void UStaticMesh::CalculateLocalBounds()
{
	if (CPUVertices.empty())
	{
		LocalBounds = FBoxSphereBounds(); // 정점이 없으면 Invalid 기본값
		return;
	}

	FVector MinBound(FLT_MAX, FLT_MAX, FLT_MAX);
	FVector MaxBound(-FLT_MAX, -FLT_MAX, -FLT_MAX);


	/*
		float x, y, z;    // Position (12 byte)
	float r, g, b, a; // Color    (16 byte)
	float u, v;       //  UV 좌표 (8 byte)
	*/

	for (const FVertexSimple& Vertex : CPUVertices)
	{
		// X축 최소/최대
		if (Vertex.x < MinBound.x) MinBound.x = Vertex.x;
		if (Vertex.x > MaxBound.x) MaxBound.x = Vertex.x;

		// Y축 최소/최대
		if (Vertex.y < MinBound.y) MinBound.y = Vertex.y;
		if (Vertex.y > MaxBound.y) MaxBound.y = Vertex.y;

		// Z축 최소/최대
		if (Vertex.z < MinBound.z) MinBound.z = Vertex.z;
		if (Vertex.z > MaxBound.z) MaxBound.z = Vertex.z;
	}

	// 구한 Min, Max를 통해 LocalBounds 생성 (FBoxSphereBounds 생성자가 Center와 Extent를 자동 계산)
	LocalBounds = FBoxSphereBounds(MinBound, MaxBound);
}
