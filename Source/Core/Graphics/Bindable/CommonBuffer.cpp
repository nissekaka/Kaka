#include "stdafx.h"
#include "CommonBuffer.h"

namespace Kaka
{
	void CommonBuffer::InitBuffers(const Graphics& aGfx)
	{
		vcb.Init(aGfx, commonData);
		pcb.Init(aGfx, commonData);
	}

	void CommonBuffer::UpdateAndBindBuffers(const Graphics& aGfx, const CommonContext& aContext)
	{
		commonData.historyViewProjection = commonData.viewProjection;
		commonData.viewProjection = aContext.camera->GetInverseView() * aContext.camera->GetJitteredProjection();
		commonData.inverseViewProjection = DirectX::XMMatrixInverse(nullptr, commonData.viewProjection);
		commonData.projection = DirectX::XMMatrixInverse(nullptr, aContext.camera->GetProjection());
		commonData.viewInverse = aContext.camera->GetInverseView();
		commonData.cameraPosition = { aContext.camera->GetPosition().x, aContext.camera->GetPosition().y, aContext.camera->GetPosition().z, 0.0f };
		commonData.resolution = aContext.currentResolution;
		commonData.currentTime = aContext.totalTime;

		pcb.Update(aGfx, commonData);
		pcb.Bind(aGfx);

		vcb.Update(aGfx, commonData);
		vcb.Bind(aGfx);
	}

}