#pragma once

#include "SceneComponent.h"
#include <d3d11.h>

class FCharDataInfo;

class UPrimitiveComponent : public USceneComponent
{
	REFLECT_CLASS(UPrimitiveComponent, USceneComponent)
public:
	UPrimitiveComponent();

	//void Initialize(GraphicsManager* graphicsManager, EPrimitive ePrimitive);
	//void Initialize(GraphicsManager* graphicsManager, EPrimitive ePrimitive, FVector location, FRotator rotation, FVector scale3D);

	void Initialize(EPrimitive ePrimitive);
	void Initialize(EPrimitive ePrimitive, FVector location, FRotator rotation, FVector scale3D,
					FCharDataInfo pmCharInfo, ID3D11ShaderResourceView* pSRV = nullptr); 

	virtual ~UPrimitiveComponent();

	virtual void SerializeClass(json::JSON& outJson) const override;
	virtual void DeserializeClass(const json::JSON& inJson) override;

	//virtual void Render();
	void Update(TArray<FRenderInfo>* outRenderInfos) override final;
	void GetRenderInfos(TArray<FRenderInfo>* outRenderInfos) const override final;

protected:
	//GraphicsManager* mGraphicsManager;
	EPrimitive mePrimitive;
	FCharDataInfo mCharInfo;
	ID3D11ShaderResourceView* SRV;
};


