#include "JsonUtil.h"

#include "Json/json.hpp"

json::JSON FVectorToJson(const FVector& Vector)
{
	json::JSON vectorJson = json::JSON::Make(json::JSON::Class::Array);
	vectorJson[0] = Vector.x;
	vectorJson[1] = Vector.y;
	vectorJson[2] = Vector.z;
	return vectorJson;
}

json::JSON FRotatorToJson(const FRotator& Rotator)
{
	json::JSON rotatorJson = json::JSON::Make(json::JSON::Class::Array);
	rotatorJson[0] = Rotator.Pitch;
	rotatorJson[1] = Rotator.Yaw;
	rotatorJson[2] = Rotator.Roll;
	return rotatorJson;
}

json::JSON EPrimitiveToJson(const EPrimitive& Primitive)
{
	switch (Primitive)
	{
	case EPrimitive::EP_Sphere:
		return json::JSON("Sphere");
	case EPrimitive::EP_Cube:
		return json::JSON("Cube");
	case EPrimitive::EP_Triangle:
		return json::JSON("Triangle");
	case EPrimitive::EP_GizmoArrow:
		return json::JSON("GizmoArrow");
	case EPrimitive::EP_Circle:
		return json::JSON("Circle");
	default:
		throw std::runtime_error("Unknown EPrimitive value");
	}
}

FVector FVectorFromJson(const json::JSON& json)
{
	if (json.JSONType() != json::JSON::Class::Array)
	{
		throw std::runtime_error("Json Array expected for FVector");
	}

	return FVector(NumberFromJson(json.at(0)), NumberFromJson(json.at(1)), NumberFromJson(json.at(2)));
}

FRotator FRotatorFromJson(const json::JSON& json)
{
	if (json.JSONType() != json::JSON::Class::Array)
	{
		throw std::runtime_error("Json Array expected for FRotator");
	}

	return FRotator(NumberFromJson(json.at(0)), NumberFromJson(json.at(1)), NumberFromJson(json.at(2)));
}

EPrimitive EPrimitiveFromJson(const json::JSON& json)
{
	if (json.JSONType() != json::JSON::Class::String)
	{
		throw std::runtime_error("Json String expected for EPrimitive");
	}
	std::string primitiveStr = json.ToString();
	if (primitiveStr == "Sphere")
	{
		return EPrimitive::EP_Sphere;
	}
	else if (primitiveStr == "Cube")
	{
		return EPrimitive::EP_Cube;
	}
	else if (primitiveStr == "Triangle")
	{
		return EPrimitive::EP_Triangle;
	}
	else if (primitiveStr == "GizmoArrow")
	{
		return EPrimitive::EP_GizmoArrow;
	}
	else if (primitiveStr == "Circle")
	{
		return EPrimitive::EP_Circle;
	}
	else
	{
		throw std::runtime_error("Unknown EPrimitive value in JSON");
	}
}

#include <cmath>
float NumberFromJson(const json::JSON& Value)
{
	double Number;
	if (Value.JSONType() == json::JSON::Class::Integral)
		Number = Value.ToInt();
	else if (Value.JSONType() == json::JSON::Class::Floating)
		Number = Value.ToFloat();
	else
		throw std::runtime_error("Expected a number");
	if (!std::isfinite(Number) || std::abs(Number) > FLT_MAX)
		throw std::runtime_error("Invalid scene number");
	return static_cast<float>(Number);
}
