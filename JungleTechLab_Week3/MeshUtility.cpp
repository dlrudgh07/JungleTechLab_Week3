
#include "MeshUtility.h"

bool IsSameVertex(const FVertexSimple& A, const FVertexSimple& B)
{

	float epsilon = 1e-5f;

	if (fabsf(A.x - B.x) > epsilon) return false;
	if (fabsf(A.y - B.y) > epsilon) return false;
	if (fabsf(A.z - B.z) > epsilon) return false;
	if (fabsf(A.r - B.r) > epsilon) return false;
	if (fabsf(A.g - B.g) > epsilon) return false;
	if (fabsf(A.b - B.b) > epsilon) return false;
	if (fabsf(A.a - B.a) > epsilon) return false;
	if (fabsf(A.u - B.u) > epsilon) return false;
	if (fabsf(A.v - B.v) > epsilon) return false;


	return true;
}

void Welding(const FVertexSimple* InVertices, uint32 InVertexCount,
	TArray<FVertexSimple>& OutVertices, TArray<uint32>& OutIndices)	 //Welding : 정점을 줄이고 인덱스를 만드는것
{

	for (uint32 i = 0;i < InVertexCount;++i) // 원본 Vertexes 순회
	{
		int32 FoundIndex = -1;
		for (uint32 j = 0;j < OutVertices.Num();++j) // Indices Vertex(OutVertices)순회
		{

			if (IsSameVertex(InVertices[i], OutVertices[j])) // 같으면
			{
				FoundIndex = j;
				break;
			}
		}

		if (FoundIndex == -1) // OutVertices안에 중복이 없으면
		{
			FoundIndex = OutVertices.Num();
			OutVertices.Add(InVertices[i]);
		}
		OutIndices.Add(FoundIndex);
	}
}
