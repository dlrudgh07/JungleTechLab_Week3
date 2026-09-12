
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

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "imGui/imgui_impl_win32.h"

#include "FrameTimer.h"
#include "ActorComponent.h"

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
		ExecuteLoadScene();
		bPendingLoadScene = false;
	}

	if (mCurrentWorld)
	{
		mCurrentWorld->Update(DeltaTime);
	}
}

void FSceneManager::UpdateGUI(const FGuiReference& guiReference)
{
	//ImGui
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	updateControlPanelGUI(guiReference);
	updatePropertyWindowGUI(guiReference);
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
	const char* AssetNames[] = { "Cube", "Sphere", "Quad", "Crate"};
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

			// Factory를 통해 UStaticMeshComponent를 가진 진짜 액터를 스폰
			AActor* newActor = mCurrentWorld->SpawnStaticMeshActor(SelectedName, { FVector(0, 0, 0), FRotator(0, 0, 0), FVector(1, 1, 1) }, *guiReference.ResourceManager);
			if (newActor == nullptr)
				UE_LOG("Error: Asset not found in ResourceManager!");
		}
	}

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
		bool bWireFrame = guiReference.GraphicsManager->GetWireFrame();
		if (ImGui::Checkbox("Wire frame", &bWireFrame))
		{
			guiReference.GraphicsManager->SetWireFrame(bWireFrame);
		}

		bool bShowWorldAxis = guiReference.GraphicsManager->GetShowWorldAxis();
		if (ImGui::Checkbox("World axis", &bShowWorldAxis))
		{
			guiReference.GraphicsManager->SetShowWorldAxis(bShowWorldAxis);
		}

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

		ImGui::EndCombo();
	}

	ImGui::Text("FOV     ");
	ImGui::SameLine();
	ImGui::SliderFloat("##FOV", &camera.mFovDegree, 0.0f, 180.0f);

	// 1) 라벨 텍스트를 먼저 그리고 같은 줄로
	ImGui::Text("Location");
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

	ImGui::Text("Rotation");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(itemWidth);
	ImGui::DragFloat("##CamRotX", &camera.Transform.Rotation.Roll, 0.1f, 180.0f);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(itemWidth);
	ImGui::DragFloat("##CamRotY", &camera.Transform.Rotation.Pitch, 0.1f, 180.0f);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(itemWidth);
	ImGui::DragFloat("##CamRotZ", &camera.Transform.Rotation.Yaw, 0.1f, 180.0f);
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
		// Temporary variables to hold the values for ImGui input fields
		const FTransform& originalTransform = mSelectedActor->GetTransform();

		// Get the current transform of the clicked actor
		FVector translationInput = originalTransform.Location;
		FVector rotationInput = {
			originalTransform.Rotation.Roll,
			originalTransform.Rotation.Pitch,
			originalTransform.Rotation.Yaw
		};
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

void FSceneManager::SaveScene(
	std::string_view sceneName,
	const FFileManager& fileManager)
{
	FString fileName = kSceneDataDir;
	fileName += FString("/");
	fileName += sceneName;
	fileName += kSceneDataSuffix;

	// Read the current scene data to read the Version
	uint32 version = 0;

	try
	{
		FString readSceneString = fileManager.ReadFileToString(fileName);
		json::JSON readSceneJson = json::JSON::Load(readSceneString);

		if (!readSceneJson.hasKey("Version") || readSceneJson.at("Version").JSONType() != json::JSON::Class::Integral)
		{
			version = 0;
		}
		else
		{
			version = readSceneJson.at("Version").ToInt();
		}
	}
	catch (const std::exception& e)
	{
		// If the file does not exist or cannot be read, we can assume it's a new scene and set version to 0
		assert(e.what());
		version = 0;
	}

	json::JSON writeSceneJson = json::JSON::Make(json::JSON::Class::Object);
	json::JSON worldJson = json::JSON::Make(json::JSON::Class::Object);
	mCurrentWorld->SerializeClass(worldJson);

	writeSceneJson["Version"] = version;
	writeSceneJson["World"] = worldJson;

	FString jsonString = FString(writeSceneJson.dump(1, "  "));
	fileManager.WriteStringToFile(fileName, jsonString);
}

void FSceneManager::ExecuteLoadScene()
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

	json::JSON readSceneJson = json::JSON::Load(jsonString);

	json::JSON worldJson = readSceneJson.at("World");

	UWorld* newWorld = FObjectFactory::LoadObject<UWorld>(worldJson);
	if (!newWorld)
	{
		throw std::runtime_error(std::format("Failed to load world from scene: {}", PendingSceneName));
	}

	// Replace the contents of mCurrentWorld with newWorld
	delete mCurrentWorld;
	mCurrentWorld = newWorld;

	ResetSelectedActor();
}

void  FSceneManager::SetSelectedActor(AActor* actor)
{
	if (actor == nullptr)
	{
		UE_LOG_F("SetSelectedActor: Attempted to set selected actor to nullptr.");
		return;
	}

	if (actor == mSelectedActor)
	{
		UE_LOG("SetSelectedActor: Actor with GUID {} is already selected.", actor->ObjectID.GUID.ToString());
		return; // No change
	}

	UE_LOG("SetSelectedActor: Actor with GUID {} is now selected.", actor->ObjectID.GUID.ToString());
	mSelectedActor = actor;
}

float FSceneManager::GetPanelWidth() const
{
	return mPanelWidth;
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


