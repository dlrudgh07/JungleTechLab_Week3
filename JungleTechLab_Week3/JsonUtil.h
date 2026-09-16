#pragma once

#include "Json/json.hpp"
#include "Vector.h"
#include "Rotator.h"
#include "enum.h"

json::JSON FVectorToJson(const FVector& Vector);
json::JSON FVector4ToJson(const FVector4& Vector);
json::JSON FRotatorToJson(const FRotator& Rotator);
json::JSON EPrimitiveToJson(const EPrimitive& Primitive);

FVector FVectorFromJson(const json::JSON& json);
FVector4 FVector4FromJson(const json::JSON& json);
FRotator FRotatorFromJson(const json::JSON& json);
EPrimitive EPrimitiveFromJson(const json::JSON& json);

// JSON distinguishes integer and floating tokens; scene numbers accept both.
float NumberFromJson(const json::JSON& Value);

bool BoolFromJson(const json::JSON& Value);
