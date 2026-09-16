#include "SceneSerialization.h"

#include "SceneManager.h"

#include <algorithm>
#include <format>

#include "FileManager.h"
#include "ResourceManager.h"
#include "EngineStatics.h"
#include "JsonUtil.h"
#include "ObjectFactory.h"
#include "PrimitiveComponent.h"
#include "TArray.h"
#include "World.h"
#include "FEditorViewportClient.h"
#include "Camera.h"
#include "Console.h"
#include "UTextComponent.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "imGui/imgui_impl_win32.h"

#include "FrameTimer.h"
#include "ActorComponent.h"

HIMC FSceneManager::s_savedImc = nullptr;
bool FSceneManager::s_imeDisabled = false;

FSceneManager::FSceneManager()
{
	ImGuiIO& io = ImGui::GetIO();
	mPanelWidth = io.DisplaySize.x * MIN_WIDTH_RATIO;

	mCurrentWorld = FObjectFactory::ConstructObject<UWorld>();

	// Todo: Test code, move to other function
	//{
	//	UCubeComponent* cubeComponent = FObjectFactory::ConstructObject<UCubeComponent>(FVector(0), FRotator(), FVector(1));
	//	AActor* cubeActor = FObjectFactory::ConstructObject<AActor>();
	//	cubeActor->AddComponent(cubeComponent);
	//	mCurrentWorld->AddActor(cubeActor);

	//	UCubeComponent* cubeComponent2 = FObjectFactory::ConstructObject<UCubeComponent>(FVector(1, 1, 1), FRotator(), FVector(0.5));
	//	AActor* cubeActor2 = FObjectFactory::ConstructObject<AActor>();
	//	cubeActor2->AddComponent(cubeComponent2);
	//	mCurrentWorld->AddActor(cubeActor2);
	//}
}

FSceneManager::~FSceneManager()
{
	delete mCurrentWorld;
}

void FSceneManager::Update(float DeltaTime)
{
	// 프레임의 가장 안전한 시점(Update 시작 전)에 씬 교체 진행
	if (bPendingNewScene)
	{
		ExecuteNewScene();
		bPendingNewScene = false;
	}
	if (bPendingLoadScene && PendingFileManager)
	{
		ExecuteLoadScene(*PendingFileManager);
		bPendingLoadScene = false;
	}

	if (mCurrentWorld)
	{
		mCurrentWorld->Update(DeltaTime);
	}
}

void FSceneManager::UpdateGUI(const FGuiReference& guiReference)
{
	//한글 입력 버그 방지
	UpdateImeAssociation();

	//ImGui
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	updateControlPanelGUI(guiReference);
	updatePropertyWindowGUI(guiReference);

	//프레임드랍의 원인
	updateObjectListPanelGUI(guiReference);

	ConsoleWindow::GetInstance().Draw(mPanelWidth);
}

void FSceneManager::ProcessPendingKills()
{
	if (mSelectedActor && mSelectedActor->IsPendingKill())
	{
		mSelectedActor = nullptr; 
	}

	if (mCurrentWorld)
	{
		mCurrentWorld->ProcessPendingKills();
	}
}

void FSceneManager::updateControlPanelGUI(const FGuiReference& guiReference)
{
	ImGuiIO& io = ImGui::GetIO();

	float panelHeight = io.DisplaySize.y * CONTROL_PANEL_HEIGHT_RATIO;

	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);

	ImGui::SetNextWindowSizeConstraints(
		ImVec2(io.DisplaySize.x * MIN_WIDTH_RATIO, panelHeight),
		ImVec2(io.DisplaySize.x * MAX_WIDTH_RATIO, panelHeight)
	);
	ImGui::SetNextWindowSize(ImVec2(mPanelWidth, panelHeight), ImGuiCond_Always);

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
	ImGui::Begin("Jungle Control Panel", nullptr, flags);
	mPanelWidth = ImGui::GetWindowWidth();

	ImGui::Text("Hello Jungle World!");
	ImGui::Text("FPS: %.1f  dt: %.4f", guiReference.FrameTimer.GetFPS(), guiReference.FrameTimer.GetDeltaTime());

	/* Spawn Actor */
	ImGui::SeparatorText("Spawn Actor");

	// 1. ResourceManager에 등록된 스태틱 메쉬 에셋 이름들 (하드코딩 Enum을 대체)
	// todo : 이건 추후 자동화해야할듯함
	const char* AssetNames[] = { "Cube", "Sphere", "Quad", "Crate", "Text Mesh", "SubUVMesh", "Rain", "Boat"};
	int32 spawnCount = mGuiInputField.SpawnCount;

	// 2. 콤보 박스 UI (선택한 인덱스가 mGuiInputField.SelectedMeshIndex에 저장됨)
	if (ImGui::Combo("Mesh Asset", &mGuiInputField.SelectedMeshIndex, AssetNames, IM_ARRAYSIZE(AssetNames)))
	{
	}

	if (ImGui::Button("Spawn"))
	{
		for (int32 i = 0; i < mGuiInputField.SpawnCount; ++i)
		{
			// 3. 선택된 인덱스를 문자열 이름으로 변환 ("Cube", "Sphere" 등)
			std::string SelectedName = AssetNames[mGuiInputField.SelectedMeshIndex];

			if (SelectedName == "Text Mesh")
			{
				AActor* newActor = mCurrentWorld->SpawnTextMeshActor({ FVector(0, 0, 0), FRotator(0, 0, 0), FVector(1, 1, 1) }, *guiReference.ResourceManager);
			}
			else if (SelectedName == "SubUVMesh")
			{
				AActor* newActor = mCurrentWorld->SpawnParticleActor({ FVector(0, 0, 0), FRotator(0, 0, 0), FVector(1, 1, 1) }, *guiReference.ResourceManager);
			}
			else if (SelectedName == "Rain")
			{
				AActor* newActor = mCurrentWorld->SpawnRainActor({ FVector(0, 0, 0), FRotator(0, 0, 0), FVector(1, 1, 1) }, *guiReference.ResourceManager);
			}
			else
			{
				// Factory를 통해 UStaticMeshComponent를 가진 진짜 액터를 스폰
				AActor* newActor = mCurrentWorld->SpawnStaticMeshActor(SelectedName, { FVector(0, 0, 0), FRotator(0, 0, 0), FVector(1, 1, 1) }, *guiReference.ResourceManager);
				if (newActor == nullptr)
					UE_LOG("Error: Asset not found in ResourceManager!");
			}			
		}
	}

	////임시
	//if (ImGui::Button("Spawn SubUV"))
	//{
	//	mCurrentWorld->SpawnSubUVActor(
	//		{ FVector(0,0,0), FRotator(0,0,0), FVector(1,1,1) },
	//		*guiReference.ResourceManager);
	//}
	
	ImGui::SameLine();
	if (ImGui::InputInt("Number of spawn", &spawnCount))
	{
		if (spawnCount < 1)
		{
			spawnCount = 1;
		}
		mGuiInputField.SpawnCount = spawnCount;
	}

	/* Scene Control */
	ImGui::SeparatorText("Scene Control");
	ImGui::InputText("Scene Name", mGuiInputField.SceneName, IM_ARRAYSIZE(mGuiInputField.SceneName));

	if (ImGui::Button("New scene"))
	{
		guiReference.ViewportClient->Reset();
		RequestNewScene(); 
	}
	if (ImGui::Button("Save scene"))
	{
		SaveScene(mGuiInputField.SceneName, *guiReference.FileManager);
	}
	if (ImGui::Button("Load scene"))
	{
		guiReference.ViewportClient->Reset();
		RequestLoadScene(mGuiInputField.SceneName, *guiReference.FileManager);
	}


	/* Camera Control */
	ImGui::SeparatorText("Camera Control");

	FCamera& camera = guiReference.ViewportClient->GetCamera();
	URenderer* renderer = guiReference.GraphicsManager->GetRenderer();

	//ImGui::SliderFloat("Speed", &Camera.Speed, -10.0f, 10.0f);
	if (ImGui::BeginCombo("##ShowFlags", "Show Flags"))
	{
		/* -------------사용법--------------------
		bool b시각화대상 = HasFlag(guiReference.ViewportClient->GetShowFlags(), EEngineShowFlags::SF_시각화대상);
		if (ImGui::Checkbox("시각화대상", &b시각화대상))
		{
			guiReference.ViewportClient->SetShowFlag(EEngineShowFlags::SF_시각화대상, b시각화대상);
		}
		*/
		bool bShowWorldAxis = HasFlag(guiReference.ResourceManager->IniConfig.GetShowFlags(), EEngineShowFlags::SF_WorldAxis);
		if (ImGui::Checkbox("World axis", &bShowWorldAxis))
		{
			guiReference.ResourceManager->IniConfig.SetShowFlag(EEngineShowFlags::SF_WorldAxis, bShowWorldAxis);
			guiReference.ResourceManager->IniConfig.Save();
		}

		bool bShowPrimitive = HasFlag(guiReference.ResourceManager->IniConfig.GetShowFlags(), EEngineShowFlags::SF_Primitives);
		if (ImGui::Checkbox("Show Primitives", &bShowPrimitive))
		{
			guiReference.ResourceManager->IniConfig.SetShowFlag(EEngineShowFlags::SF_Primitives, bShowPrimitive);
			guiReference.ResourceManager->IniConfig.Save();
		}

		bool bShowAABB = HasFlag(guiReference.ResourceManager->IniConfig.GetShowFlags(), EEngineShowFlags::SF_BoundingBoxes);
		if (ImGui::Checkbox("Show AABB", &bShowAABB))
		{
			guiReference.ResourceManager->IniConfig.SetShowFlag(EEngineShowFlags::SF_BoundingBoxes, bShowAABB);
			guiReference.ResourceManager->IniConfig.Save();
		}

		bool bShowGizmo = HasFlag(guiReference.ResourceManager->IniConfig.GetShowFlags(), EEngineShowFlags::SF_Gizmo);
		if (ImGui::Checkbox("Gizmo", &bShowGizmo))
		{
			guiReference.ResourceManager->IniConfig.SetShowFlag(EEngineShowFlags::SF_Gizmo, bShowGizmo);
			guiReference.ResourceManager->IniConfig.Save();
		}

		bool bBillBoard = HasFlag(guiReference.ResourceManager->IniConfig.GetShowFlags(), EEngineShowFlags::SF_BillboardText);
		if (ImGui::Checkbox("BillBoard", &bBillBoard))
		{
			guiReference.ResourceManager->IniConfig.SetShowFlag(EEngineShowFlags::SF_BillboardText, bBillBoard);
			guiReference.ResourceManager->IniConfig.Save();
		}

		bool bGrid = HasFlag(guiReference.ResourceManager->IniConfig.GetShowFlags(), EEngineShowFlags::SF_Grid);
		if (ImGui::Checkbox("Grid", &bGrid))
		{
			guiReference.ResourceManager->IniConfig.SetShowFlag(EEngineShowFlags::SF_Grid, bGrid);
			guiReference.ResourceManager->IniConfig.Save();
		}

		bool bUUID = HasFlag(guiReference.ResourceManager->IniConfig.GetShowFlags(), EEngineShowFlags::SF_UUID);
		if (ImGui::Checkbox("UUID", &bUUID))
		{
			guiReference.ResourceManager->IniConfig.SetShowFlag(EEngineShowFlags::SF_UUID, bUUID);
			guiReference.ResourceManager->IniConfig.Save();
		}

		ImGui::EndCombo();
	}

	ImGui::SameLine();
	bool bOrthographic = guiReference.GraphicsManager->IsOrthographicTarget();
	if (ImGui::Checkbox("Orthogonal", &bOrthographic))
	{
		if (mSelectedActor && bOrthographic && guiReference.GraphicsManager->GetPerspectiveRatio() == 1.0f)
		{
			const FVector offset = mSelectedActor->GetTransform().Location - camera.Transform.Location;
			const float depth = FVector::dot(offset, camera.GetForwardVector());
			camera.mOrthoDistance = FMath::Max(depth, 0.1f);
		}
		guiReference.GraphicsManager->StartProjectionTransition(bOrthographic);
	}

	// viewMode UI. 배열 순서는 EViewModeIndex 선언 순서(Lit, Unlit, Wireframe)와 맞아야 한다
	const char* viewModeNames[] = { "Lit", "Unlit", "Wireframe" };
	int32 viewModeIndex = static_cast<int32>(guiReference.ViewportClient->GetViewMode());
	if (ImGui::Combo("View Mode", &viewModeIndex, viewModeNames, IM_ARRAYSIZE(viewModeNames)))
	{
		guiReference.ViewportClient->SetViewMode(static_cast<EViewModeIndex>(viewModeIndex));
	}

	ImGui::Text("FOV        ");
	ImGui::SameLine();
	ImGui::SliderFloat("##FOV", &camera.mFovDegree, 0.0f, 180.0f);


	float Speed = guiReference.ResourceManager->IniConfig.GetCameraSpeed();
	ImGui::Text("Speed      ");
	ImGui::SameLine();
	if (ImGui::SliderFloat("##Speed", &Speed, 0.1f, 100.0f))
	{
		guiReference.ResourceManager->IniConfig.SetCameraSpeed(Speed);
	}
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		guiReference.ResourceManager->IniConfig.Save();
	}

	float Sensitivity = guiReference.ResourceManager->IniConfig.GetCameraSensitivity();
	ImGui::Text("Sensitivity");
	ImGui::SameLine();
	if (ImGui::SliderFloat("##Sensitivity", &Sensitivity, 0.01f, 0.5f))
	{
		guiReference.ResourceManager->IniConfig.SetCameraSensitivity(Sensitivity);
	}
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		guiReference.ResourceManager->IniConfig.Save();
	}


	// 1) 라벨 텍스트를 먼저 그리고 같은 줄로
	ImGui::Text("Location   ");
	ImGui::SameLine();

	// 2) 텍스트를 그린 "뒤"의 남은 폭을 기준으로 계산
	const float spacing = ImGui::GetStyle().ItemSpacing.x;
	const float itemWidth = (ImGui::GetContentRegionAvail().x - spacing * 2.0f) / 3.0f;

	ImGui::SetNextItemWidth(itemWidth);
	ImGui::DragFloat("##CamLocX", &camera.Transform.Location.x, 0.1f, 10.0f);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(itemWidth);
	ImGui::DragFloat("##CamLocY", &camera.Transform.Location.y, 0.1f, 10.0f);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(itemWidth);
	ImGui::DragFloat("##CamLocZ", &camera.Transform.Location.z, 0.1f, 10.0f);

	// 회전은 쿼터니언으로 보관하므로 각도로 풀어서 편집하고 바뀌면 다시 변환한다
	FRotator camEuler = camera.Transform.Rotation.ToEuler();
	bool camRotChanged = false;
	ImGui::Text("Rotation   ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(itemWidth);
	camRotChanged |= ImGui::DragFloat("##CamRotX", &camEuler.Roll, 0.1f, 180.0f);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(itemWidth);
	camRotChanged |= ImGui::DragFloat("##CamRotY", &camEuler.Pitch, 0.1f, 180.0f);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(itemWidth);
	camRotChanged |= ImGui::DragFloat("##CamRotZ", &camEuler.Yaw, 0.1f, 180.0f);
	if (camRotChanged)
	{
		camera.Transform.Rotation = FQuaternion::FromEuler(camEuler);
	}
	//ImGui::Checkbox("Depth Test", &renderer->bDepthTestEnabled);
	//ImGui::TextUnformatted(renderer->bDepthTestEnabled
	//	? "ON : orange (near) stays in front"
	//	: "OFF: blue (far, drawn last) overwrites");

	/* Memory Info */
	ImGui::SeparatorText("Memory Info");

	ImGui::Text("Total allocated memory count: %d", UEngineStatics::sTotalAllocationCount);
	ImGui::Text("Total allocated memory size: %d bytes", UEngineStatics::sTotalAllocationBytes);

	/* Gizmo Control */
	ImGui::SeparatorText("Gizmo Control");

	// Display the current gizmo mode dropdown
	const char* gizmoModeNames[] = { "Translate", "Rotate", "Scale" };
	int32 gizmoModeIndex = static_cast<int32>(guiReference.ViewportClient->mGizmo.eType);
	if (ImGui::Combo("Gizmo Mode", &gizmoModeIndex, gizmoModeNames, IM_ARRAYSIZE(gizmoModeNames)))
	{
		guiReference.ViewportClient->mGizmo.SetGizmoType(static_cast<EGIZMO_TYPE>(gizmoModeIndex));
	}
	if (ImGui::Button("Next Gizmo Mode"))
	{
		guiReference.ViewportClient->mGizmo.CycleGizmoType();
	}

	float Offset = guiReference.ResourceManager->IniConfig.GetGridOffset();

	ImGui::SeparatorText("Grid Control");
	ImGui::Text("Offset ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(150);
	if (ImGui::DragFloat("##OFFSET", &Offset, 0.01f, 0.1f, 20.0f))
	{
		guiReference.ResourceManager->IniConfig.SetGridOffset(Offset);
	}
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		guiReference.ResourceManager->IniConfig.Save();
	}

	int Range = guiReference.ResourceManager->IniConfig.GetGridRange();

	ImGui::Text("Range  ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(150);
	if (ImGui::DragInt("##RANGE", &Range, 0.1f, 20, 100))
	{
		guiReference.ResourceManager->IniConfig.SetGridRange(Range);
	}
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		guiReference.ResourceManager->IniConfig.Save();
	}

	ImGui::End();
}

void FSceneManager::updatePropertyWindowGUI(const FGuiReference& guiReference)
{
	ImGuiIO& io = ImGui::GetIO();

	float controlPanelHeight = io.DisplaySize.y * CONTROL_PANEL_HEIGHT_RATIO;
	float propertyHeight = io.DisplaySize.y * WINDOW_PROPERTY_HEIGHT_RATIO;

	ImGui::SetNextWindowPos(ImVec2(0.0f, controlPanelHeight), ImGuiCond_Always);

	ImGui::SetNextWindowSizeConstraints(
		ImVec2(io.DisplaySize.x * MIN_WIDTH_RATIO, propertyHeight),
		ImVec2(io.DisplaySize.x * MAX_WIDTH_RATIO, propertyHeight)
	);
	ImGui::SetNextWindowSize(ImVec2(mPanelWidth, propertyHeight), ImGuiCond_Always);

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

	ImGui::Begin("Jungle Property Window", nullptr, flags);

	mPanelWidth = ImGui::GetWindowWidth();

	if (mSelectedActor)
	{
		FName CurrentFName = mSelectedActor->GetName();
		FString CurrentFString = CurrentFName.ToString();

		ImGui::Text("FName: %s", CurrentFString.CStr());

		uint32 CurrentComparisonIndex = CurrentFName.GetComparisonIndex();
		uint32 CurrentDisplayIndex = CurrentFName.GetDisplayIndex();
		ImGui::Text("FName CurrentComparisonIndex : %u", CurrentComparisonIndex);
		ImGui::Text("FName CurrentDisplayIndex : %u", CurrentDisplayIndex);
		char Buf[128] = "";
		if (ImGui::InputText("SetFName", Buf, sizeof(Buf), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			mSelectedActor->SetName(FString(Buf));
		}

		// Temporary variables to hold the values for ImGui input fields
		const FTransform& originalTransform = mSelectedActor->GetTransform();

		// Get the current transform of the clicked actor
		FVector translationInput = originalTransform.Location;
		const FRotator euler = originalTransform.Rotation.ToEuler();   // 표시용으로만 각도로 풀어냄
		FVector rotationInput = { euler.Roll, euler.Pitch, euler.Yaw };
		FVector scaleInput = originalTransform.Scale;

		// Display and edit the transform properties using ImGui input fields
		if (ImGui::DragFloat3("Translation", &translationInput.x, 0.1f))
		{
			mSelectedActor->SetLocation(translationInput);
		}
		if (ImGui::DragFloat3("Rotation", &rotationInput.x, 0.1f))
		{
			mSelectedActor->SetRotation({
				rotationInput.y, // Pitch
				rotationInput.z, // Yaw
				rotationInput.x  // Roll
				});

		}
		if (ImGui::DragFloat3("Scale", &scaleInput.x, 0.1f, MIN_SCALE, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp))
		{
			mSelectedActor->SetScale(scaleInput);
		}

		//TextComponent를 갖고 있다면
		//UTextComponent*
		const TArray<UActorComponent*>& AllComp = mSelectedActor->GetComponents();
		for (auto Elem : AllComp)
		{
			if (Elem->IsA(UTextComponent::GetClass()))
			{
				//직전에 활성 상태였는지
				UTextComponent* TextComp = Elem->Cast<UTextComponent>();
				//bool bIsEditing = ImGui::IsItemActivated();
				static char buffer[256] = {};

				static bool bWasEditingLastFrame = false;

				if (LastComp != TextComp || !bWasEditingLastFrame)
				{
					std::wstring text = TextComp->GetText();

					std::string text_ToString = WStringToString(text);

					strncpy_s(buffer, text_ToString.c_str(), sizeof(buffer) - 1);

					LastComp = TextComp;
				}

				//빌보드 옵션
				bool IsBillboard = TextComp->GetIsBillboard();
				if (ImGui::Checkbox("Is Billboard", &IsBillboard))
				{
					TextComp->SetIsBillboard(IsBillboard);
				}

				//글자가 바뀐다면
				if (ImGui::InputText("Text", buffer, sizeof(buffer)))
				{
					TextComp->SetText(StringToWString(buffer));
				}

				//활성화 중인지 편집중이라면 1, 아니라면 0이다.
				bWasEditingLastFrame = ImGui::IsItemActive();
			}
		}
	}
	ImGui::End();
}

void FSceneManager::updateObjectListPanelGUI(const FGuiReference& guiReference)
{
	ImGuiIO& io = ImGui::GetIO();

	float offsetHeight = io.DisplaySize.y * (CONTROL_PANEL_HEIGHT_RATIO + WINDOW_PROPERTY_HEIGHT_RATIO);
	float objectListPanelHeight = io.DisplaySize.y - offsetHeight;

	ImGui::SetNextWindowPos(ImVec2(0.0f, offsetHeight), ImGuiCond_Always);

	ImGui::SetNextWindowSizeConstraints(
		ImVec2(io.DisplaySize.x * MIN_WIDTH_RATIO, objectListPanelHeight),
		ImVec2(io.DisplaySize.x * MAX_WIDTH_RATIO, objectListPanelHeight)
	);
	ImGui::SetNextWindowSize(ImVec2(mPanelWidth, objectListPanelHeight), ImGuiCond_Always);

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

	ImGui::Begin("Object List Panel", nullptr, flags);
	{
		/* Object Lists */
		ImGui::SeparatorText("Object Lists");
		if (ImGui::BeginChild("ObjectList", ImVec2(0, 0),
			ImGuiChildFlags_Borders))
		{
			if (mGuiInputField.LastGUObjectRevision != UObject::GetGObjectRevision())
			{
				mGuiInputField.SortedObjectLists = UObject::GetGObjectArray().ToTArray();
				mGuiInputField.LastGUObjectRevision = UObject::GetGObjectRevision();

				// Sort the objects by index
				std::sort(mGuiInputField.SortedObjectLists.begin(), mGuiInputField.SortedObjectLists.end(),
					[](UObject* a, UObject* b) { return a->ObjectID.InternalIndex < b->ObjectID.InternalIndex; });
			}

			FGuid selectedActorGUID;
			if (mSelectedActor)
				selectedActorGUID = mSelectedActor->ObjectID.GUID;

			// Todo: rbegin()
			//for (UObject* object : mGuiInputField.SortedObjectLists)

			UObject* bDeleteActorOrNull = nullptr;
			for (int32 objectsIndex = 0; objectsIndex < mGuiInputField.SortedObjectLists.Num(); ++objectsIndex)
			{
				UObject* object = mGuiInputField.SortedObjectLists[objectsIndex];

				bool bSelected = false;
				// todo: 이 인덱스는 유니크하지 않을 수 있음
				ImGui::PushID(object->ObjectID.InternalIndex); // Ensure unique ID for each child

				// Highlight the frame if this object is the clicked actor
				if (object->ObjectID.GUID == selectedActorGUID)
				{
					bSelected = true;
					ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(255, 255, 0, 50)); // Light yellow background
				}

				if (ImGui::BeginChild("ObjectFrame", ImVec2(0, 0),
					ImGuiChildFlags_FrameStyle | ImGuiChildFlags_AutoResizeY))
				{
					ImGui::Text("Class: %s", object->GetRuntimeClass()->Name.CStr());
					ImGui::Text("GUID: %s", object->ObjectID.GUID.ToString().CStr());
					ImGui::Text("Name: %s", object->GetName().ToString().CStr());

					// TODO: Move implement delete to where?
					if (object->IsA<AActor>())
					{
						AActor* actor = object->Cast<AActor>();

						if (ImGui::Button("Select"))
						{
							SetSelectedActor(actor);
						}
						else
						{
							ImGui::SameLine();
							if (ImGui::Button("Delete"))
							{
								bDeleteActorOrNull = object;
							}
						}
					}
				}
				ImGui::EndChild();

				if (bSelected)
				{
					ImGui::PopStyleColor(); // Pop the border color if it was pushed
				}


				ImGui::PopID();
			}

			if (bDeleteActorOrNull != nullptr)
			{
				AActor* deleteActor = bDeleteActorOrNull->Cast<AActor>();

				if (mSelectedActor != nullptr && mSelectedActor->ObjectID.GUID == deleteActor->ObjectID.GUID)
				{
					mSelectedActor = nullptr;
				}

				deleteActor->Destroy();
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();
}
void FSceneManager::RequestNewScene()
{
	bPendingNewScene = true;
}
void FSceneManager::RequestLoadScene(std::string_view sceneName, const FFileManager& fileManager)
{
	bPendingLoadScene = true;
	PendingSceneName = sceneName;
	PendingFileManager = &fileManager;
}
void FSceneManager::ExecuteNewScene()
{
	ResetSelectedActor();
	if (mCurrentWorld != nullptr)
	{
		delete mCurrentWorld;
	}
	mCurrentWorld = FObjectFactory::ConstructObject<UWorld>();
	if (mResources)
	{
		mResources->ClearAll();
		if (mGraphics)
			mResources->InitializeDefaultAssets(mGraphics);
	}
}
void FSceneManager::DeleteScene()
{
	if (mCurrentWorld != nullptr)
	{
		delete mCurrentWorld;
		mCurrentWorld = nullptr;
	}
	ResetSelectedActor();
}

// Scene format version 1 stores assets and GUID references.
void FSceneManager::SaveScene(std::string_view sceneName, const FFileManager& fileManager)
{
	try
	{
		if (!mCurrentWorld)
			throw std::runtime_error("No world to save");
		FString fileName = kSceneDataDir;
		fileName += FString("/");
		fileName += sceneName;
		fileName += kSceneDataSuffix;

		json::JSON writeSceneJson = json::JSON::Make(json::JSON::Class::Object);
		json::JSON worldJson = json::JSON::Make(json::JSON::Class::Object);
		mCurrentWorld->SerializeClass(worldJson);

		writeSceneJson["Version"] = 1;
		if (!mResources)
			throw std::runtime_error("Scene resource manager is not initialized");
		mResources->SerializeAssets(writeSceneJson["Assets"]);
		writeSceneJson["World"] = worldJson;
		ValidateSceneReferences(writeSceneJson);

		FString jsonString = FString(writeSceneJson.dump(1, "  "));
		fileManager.WriteStringToFile(fileName, jsonString);
	}
	catch (const std::exception& Error)
	{
		UE_LOG("Failed to save scene: %s", Error.what());
	}
}

void FSceneManager::ExecuteLoadScene(const FFileManager& FileManager)
{
	// 파일 탐색기를 열고, 기본 파일명으로 PendingSceneName을 띄워줍니다.
	FString absoluteFilePath = FileManager.OpenFileDialog(PendingSceneName, kSceneDataSuffix, kSceneDataDir);

	// 유저가 탐색기에서 '취소'를 눌렀다면 로드를 중단합니다.
	if (absoluteFilePath.Empty())
	{
		//UE_LOG("Scene load canceled by user.");
		return;
	}

	FString jsonString;

	try
	{
		// 탐색기에서 받아온 절대 경로(absoluteFilePath)를 이용해 바로 파일을 읽습니다.
		jsonString = PendingFileManager->ReadFileToString(absoluteFilePath);
	}
	catch (...)
	{
		UE_LOG("Failed to load scene {}: file not found.", absoluteFilePath.CStr());
		return;
	}

	try
	{
		auto Data = json::JSON::Load(jsonString);
		if (!mResources)
			throw std::runtime_error("Scene resource manager is not initialized");

		FResourceManager& StagedResources = FResourceManager::Get();
		StagedResources.ClearAll();
		mResources->InitializeForLoad(StagedResources);

		FSceneLoadScope Scope;
		if (Data.hasKey("Version") &&
			(Data.at("Version").JSONType() != json::JSON::Class::Integral || Data.at("Version").ToInt() > 1))
			throw std::runtime_error("Unsupported scene version");

		if (Data.hasKey("Version") && Data.at("Version").ToInt() == 1 && !Data.hasKey("Assets"))
			throw std::runtime_error("Missing scene assets");

		if (Data.hasKey("Assets"))
		{
			if (!Data.hasKey("Version") || Data.at("Version").ToInt() != 1)
				throw std::runtime_error("Unsupported scene version");
			StagedResources.DeserializeAssets(Data.at("Assets"));
		}
		else if (mGraphics)
			StagedResources.InitializeDefaultAssets(mGraphics);

		auto NewWorld = PreloadObject<UWorld>(Data.at("World"));
		NewWorld->DeserializeClass(Data.at("World"));
		ResetSelectedActor();
		delete mCurrentWorld;
		mCurrentWorld = NewWorld.release();
		mResources->SwapAssets(StagedResources);

		if (!Data.hasKey("Assets"))
			UE_LOG("Legacy scene has no asset data; missing mesh references cannot be recovered.");
	}
	catch (const std::exception& Error)
	{
		UE_LOG("Failed to load scene %s: %s", absoluteFilePath.CStr(), Error.what());
	}
}

/*
void FSceneManager::ExecuteLoadScene(const FFileManager& FileManager)
{
	FString fileName = kSceneDataDir;
	fileName += FString("/");
	fileName += PendingSceneName;
	fileName += kSceneDataSuffix;

	FString jsonString;

	try
	{
		jsonString = PendingFileManager->ReadFileToString(fileName);
	}
	catch (...)
	{
		UE_LOG_F("Failed to load scene {}: file not found.", PendingSceneName);
		return;
	}

	try
	{
		auto Data = json::JSON::Load(jsonString);
		if (!mResources)
			throw std::runtime_error("Scene resource manager is not initialized");
		FResourceManager &StagedResources = FResourceManager::Get();
		StagedResources.ClearAll();
		mResources->InitializeForLoad(StagedResources);
		FSceneLoadScope Scope;
		if (Data.hasKey("Version") &&
			(Data.at("Version").JSONType() != json::JSON::Class::Integral || Data.at("Version").ToInt() > 1))
			throw std::runtime_error("Unsupported scene version");
		if (Data.hasKey("Version") && Data.at("Version").ToInt() == 1 && !Data.hasKey("Assets"))
			throw std::runtime_error("Missing scene assets");
		if (Data.hasKey("Assets"))
		{
			if (!Data.hasKey("Version") || Data.at("Version").ToInt() != 1)
				throw std::runtime_error("Unsupported scene version");
			StagedResources.DeserializeAssets(Data.at("Assets"));
		}
		else if (mGraphics)
			StagedResources.InitializeDefaultAssets(mGraphics);
		auto NewWorld = PreloadObject<UWorld>(Data.at("World"));
		NewWorld->DeserializeClass(Data.at("World"));
		ResetSelectedActor();
		delete mCurrentWorld;
		mCurrentWorld = NewWorld.release();
		mResources->SwapAssets(StagedResources);
		if (!Data.hasKey("Assets"))
			UE_LOG("Legacy scene has no asset data; missing mesh references cannot be recovered.");
	}
	catch (const std::exception& Error)
	{
		UE_LOG("Failed to load scene %s: %s", PendingSceneName.c_str(), Error.what());
	}
}
*/



void  FSceneManager::SetSelectedActor(AActor* actor)
{
	if (actor == nullptr)
	{
		UE_LOG_F("SetSelectedActor: Attempted to set selected actor to nullptr.");
		return;
	}

	if (actor == mSelectedActor)
	{
		UE_LOG("SetSelectedActor: Actor with GUID {%s} is already selected.", actor->ObjectID.GUID.ToString().CStr());
		return; // No change
	}

	UE_LOG("SetSelectedActor: Actor with GUID {%s} is now selected.", actor->ObjectID.GUID.ToString().CStr());
	mSelectedActor = actor;
}

float FSceneManager::GetPanelWidth() const
{
	return mPanelWidth;
}

std::string FSceneManager::WStringToString(const std::wstring& wstr)
{
	if (wstr.empty()) return std::string("");

	int sizeNeeded = WideCharToMultiByte(
		CP_UTF8, 0, wstr.c_str(), (int)wstr.size(),
		nullptr, 0, nullptr, nullptr
	);

	std::string result(sizeNeeded, 0);
	WideCharToMultiByte(
		CP_UTF8, 0, wstr.c_str(), (int)wstr.size(),
		result.data(), sizeNeeded, nullptr, nullptr
	);

	return result;
}

std::wstring FSceneManager::StringToWString(const std::string& utf8)
{
	if (utf8.empty()) return std::wstring(L"");

	int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), nullptr, 0);
	std::wstring result(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), result.data(), sizeNeeded);
	return result;
}

void FSceneManager::SetHwnd(HWND& phwnd)
{
	hwnd = phwnd;
}

void FSceneManager::UpdateImeAssociation()
{
	ImGuiIO& io = ImGui::GetIO();

	if (!io.WantTextInput && !s_imeDisabled)
	{
		// 지금 아무 텍스트 위젯도 입력을 원하지 않음 -> IME를 창에서 완전히 떼어냄
		s_savedImc = ImmAssociateContext(hwnd, nullptr);
		s_imeDisabled = true;
	}
	else if (io.WantTextInput && s_imeDisabled)
	{
		// 다시 어떤 위젯(InputText)이 텍스트 입력을 원함 -> IME를 원래대로 다시 연결
		ImmAssociateContext(hwnd, s_savedImc);
		s_imeDisabled = false;
	}
}

const TArray<FRenderInfo>& FSceneManager::GetRenderInfos() const
{
	if (mCurrentWorld)
	{
		return mCurrentWorld->GetRenderInfos();
	}

	static const TArray<FRenderInfo> EmptyInfos;
	return EmptyInfos;
}

const TArray<FRenderInfo> FSceneManager::GetAxisRenderInfos()
{
	// TODO: Implement axis render info retrieval logic
	return TArray<FRenderInfo>();
}
