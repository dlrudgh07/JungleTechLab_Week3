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
#include "SceneSerialization.h"
#include "Material.h"
#include "Texture.h"
void UStaticMesh::SerializeClass(json::JSON& outJson) const
{
	UObject::SerializeClass(outJson);
	auto& P = outJson["Properties"];
	P["Vertices"] = json::JSON::Make(json::JSON::Class::Array);
	for (const auto& V : CPUVertices)
	{
		auto A = json::JSON::Make(json::JSON::Class::Array);
		for (float F : {V.x, V.y, V.z, V.r, V.g, V.b, V.a, V.u, V.v})
			A.append(F);
		P["Vertices"].append(A);
	}
	P["Indices"] = json::JSON::Make(json::JSON::Class::Array);
	for (auto I : CPUIndices)
		P["Indices"].append(I);
	P["Materials"] = json::JSON::Make(json::JSON::Class::Array);
	for (auto* M : StaticMaterials)
		P["Materials"].append(ObjectReference(M));
}
void UStaticMesh::DeserializeClass(const json::JSON& inJson)
{
	UObject::DeserializeClass(inJson);
	const auto& P = inJson.at("Properties");
	CPUVertices.clear();
	CPUIndices.clear();
	StaticMaterials.Reset(0);
	for (const char* Key : {"Vertices", "Indices", "Materials"})
		if (P.at(Key).JSONType() != json::JSON::Class::Array)
			throw std::runtime_error("Invalid mesh array");
	for (const auto& A : P.at("Vertices").ArrayRange())
	{
		if (A.JSONType() != json::JSON::Class::Array || A.length() != 9)
			throw std::runtime_error("Invalid vertex");
		FVertexSimple V{};
		float* Fields[] = {&V.x, &V.y, &V.z, &V.r, &V.g, &V.b, &V.a, &V.u, &V.v};
		for (int I = 0; I < 9; ++I)
			*Fields[I] = NumberFromJson(A.at(I));
		CPUVertices.push_back(V);
	}
	for (const auto& A : P.at("Indices").ArrayRange())
	{
		if (A.JSONType() != json::JSON::Class::Integral || A.ToInt() < 0 ||
			static_cast<size_t>(A.ToInt()) >= CPUVertices.size())
			throw std::runtime_error("Invalid mesh index");
		CPUIndices.push_back(static_cast<uint32>(A.ToInt()));
	}
	for (const auto& A : P.at("Materials").ArrayRange())
		StaticMaterials.Add(ResolveReference<UMaterial>(A));
	CalculateLocalBounds();
}
