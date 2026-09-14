
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

// todo
// vector를 TArray로 변경하기 
void Welding(const FVertexSimple* InVertices, uint32 InVertexCount,
	std::vector<FVertexSimple>& OutVertices, std::vector<uint32>& OutIndices)	 //Welding : 정점을 줄이고 인덱스를 만드는것
{

	for (uint32 i = 0;i < InVertexCount;++i) // 원본 Vertexes 순회
	{
		int32 FoundIndex = -1;
		for (uint32 j = 0;j < OutVertices.size();++j) // Indices Vertex(OutVertices)순회
		{

			if (IsSameVertex(InVertices[i], OutVertices[j])) // 같으면
			{
				FoundIndex = j;
				break;
			}
		}

		if (FoundIndex == -1) // OutVertices안에 중복이 없으면
		{
			FoundIndex = static_cast<int32>(OutVertices.size());
			OutVertices.emplace_back(InVertices[i]);
		}
		OutIndices.emplace_back(FoundIndex);
	}
}
