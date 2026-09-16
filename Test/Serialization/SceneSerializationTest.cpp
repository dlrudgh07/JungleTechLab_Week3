#include "SceneSerialization.h"
#include "World.h"
#include "ResourceManager.h"
#include "FFontAsset.h"
#include "UTextComponent.h"
#include "LineSpotLightComponent.h"
#include "ParticleRainComponent.h"
#include "ParticleSubUVComponent.h"
#include "Texture.h"
#include <iostream>
#include <cmath>
#include <objbase.h>

static void Check(bool Value, const char* Message)
{
    if (!Value) throw std::runtime_error(Message);
}
template<class F> static void Reject(F Action)
{
    bool Failed = false;
    try { Action(); } catch (const std::exception&) { Failed = true; }
    Check(Failed, "Invalid input was accepted");
}
template<class T> static T* Add(UWorld& World)
{
    auto* Actor = FObjectFactory::ConstructObject<AActor>();
    auto* Component = FObjectFactory::ConstructObject<T>();
    Actor->AddRootSceneComponent(Component);
    World.AddActor(Actor);
    return Component;
}
int main()
{
    try
    {
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        Microsoft::WRL::ComPtr<ID3D11Device> Device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> Context;
        Check(SUCCEEDED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0,
            nullptr, 0, D3D11_SDK_VERSION, &Device, nullptr, &Context)), "WARP device");
        auto& Resources = FResourceManager::Get();
        Resources.Initialize(Device.Get(), Context.Get());
        Check(Resources.LoadFont_FNTFile("Gulim", "Assets/Fonts/Gulim/Gulim.fnt"), "Real font source");
        auto* Texture = FObjectFactory::ConstructObject<UTexture>();
        Texture->SourcePath = FString("Assets/FireAnimationTexture.png");
        Resources.RegisterTexture("TestAtlas", Texture);
        auto* Font = FObjectFactory::ConstructUnInitializedObject<FFontAsset>();
        Font->SetFontName("TestFont");
        Font->SetLineHeight(32);
        Font->SetBaseLine(24);
        Font->SetAtlasWidth(128);
        Font->SetAtlasHeight(128);
        FString PageName("TestAtlas");
        Font->AddPageName(0, PageName);
        Font->SetPageTexture(0, Texture);
        Font->AddCharInfo(65, FCharacterInfo{65, 0, 0, 12, 24, 1, 2, 13, 0});
        Resources.RegisterFontAsset("TestFont", Font);
        Resources.RegisterStaticMesh("Sphere", FObjectFactory::ConstructObject<UStaticMesh>());
        auto World = std::unique_ptr<UWorld>(FObjectFactory::ConstructObject<UWorld>());
        const std::wstring Text = L"\uD55C\uAE00 ABC\n\U0001F600 \\\\path \"quoted\"\t";
        auto* Label = Add<UTextComponent>(*World);
        Label->SetText(Text);
        Label->SetFontAsset(Font);
        Label->SetIsBillboard(false);
        auto* Label2 = Add<UTextComponent>(*World);
        Label2->SetFontAsset(Font);
        auto* Spot = Add<ULineSpotLightComponent>(*World);
        Spot->length = 12.5f;
        Spot->Segments = 42;
        Spot->OuterAngle = 0.7f;
        Spot->Point = FVector(1, 2, 3);
        Spot->SetRelativeScale3D(FVector(2, 3, 4));
        auto* Rain = Add<UParticleRainComponent>(*World);
        Rain->MaxRainDrops = 7;
        Rain->MaxSplashDrops = 12;
        Rain->RainSpawnRate = 0.3f;
        Rain->SpawnRadius = 19;
        auto* SubUV = Add<UParticleSubUVComponent>(*World);
        SubUV->SetCols(8);
        SubUV->SetRows(2);
        SubUV->SetPlayRate(3);

        json::JSON Scene;
        Resources.SerializeAssets(Scene["Assets"]);
        World->SerializeClass(Scene["World"]);
        ValidateSceneReferences(Scene);
        Scene = json::JSON::Load(Scene.dump());
        World.reset();
        Resources.ClearAll();
        {
            FSceneLoadScope Scope;
            Resources.DeserializeAssets(Scene.at("Assets"));
            World = PreloadObject<UWorld>(Scene.at("World"));
            World->DeserializeClass(Scene.at("World"));
            const auto& Actors = World->GetActors();
            auto* Loaded = Actors[0]->GetRootComponent()->Cast<UTextComponent>();
            Check(Loaded && Loaded->GetText() == Text && !Loaded->GetIsBillboard(), "Unicode text/billboard");
            Check(Loaded->GetFontAsset() == Resources.GetFontAsset("TestFont"), "Font GUID");
            Check(Actors[1]->GetRootComponent()->Cast<UTextComponent>()->GetFontAsset() == Loaded->GetFontAsset(), "Shared font");
            Check(Loaded->GetFontAsset()->GetPageTexture(0) == Resources.GetTexture("TestAtlas"), "Atlas GUID");
            Check(Loaded->GetFontAsset()->GetCharInfo(65)->XAdvance == 13, "Font metrics");
            Check(Resources.GetFontAsset("Gulim")->GetCharInfo(0xD55C) != nullptr, "Real Korean font round trip");
            auto* LoadedSpot = Actors[2]->GetRootComponent()->Cast<ULineSpotLightComponent>();
            Check(LoadedSpot->length == 12.5f && LoadedSpot->Segments == 42 && LoadedSpot->GetRelativeScale3D().y == 3, "Spotlight settings/scale");
            json::JSON SpotJson;
            LoadedSpot->SerializeClass(SpotJson);
            Check(ResolveReference<UStaticMesh>(SpotJson.at("Properties").at("HitMeshGUID")) == Resources.GetStaticMesh("Sphere"), "Spotlight mesh GUID");
            auto* LoadedRain = Actors[3]->GetRootComponent()->Cast<UParticleRainComponent>();
            Check(LoadedRain->MaxRainDrops == 7 && LoadedRain->MaxSplashDrops == 12 && LoadedRain->SpawnRadius == 19, "Rain settings");
            auto* LoadedSub = Actors[4]->GetRootComponent()->Cast<UParticleSubUVComponent>();
            Check(LoadedSub->GetCols() == 8 && LoadedSub->GetRows() == 2 && LoadedSub->GetIsBillboard(), "SubUV");
            json::JSON Bad;
            LoadedRain->SerializeClass(Bad);
            Bad["Properties"]["RainSpawnRate"] = 0;
            Reject([&] { LoadedRain->DeserializeClass(Bad); });
            LoadedSub->SerializeClass(Bad);
            Bad["Properties"]["Cols"] = 1.5;
            Reject([&] { LoadedSub->DeserializeClass(Bad); });
            Loaded->SerializeClass(Bad);
            Bad["Properties"]["FontAssetGUID"] = ObjectReference(Resources.GetTexture("TestAtlas"));
            Reject([&] { Loaded->DeserializeClass(Bad); });
            LoadedSub->SerializeClass(Bad);
            json::JSON Legacy = Bad;
            Legacy["Properties"] = json::JSON::Make(json::JSON::Class::Object);
            for (const auto& [Key, Value] : Bad.at("Properties").ObjectRange())
                if (Key != "bIsBillboard" && Key != "bIsOriginalColor" && Key != "PrimitiveColor") Legacy["Properties"][Key] = Value;
            auto LegacySub = std::unique_ptr<UParticleSubUVComponent>(FObjectFactory::LoadObject<UParticleSubUVComponent>(Legacy));
            Check(LegacySub->GetIsBillboard(), "Legacy billboard default");
        }
        World.reset();
        Resources.ClearAll();
        CoUninitialize();
        std::cout << "PASS: scene round trip, Unicode, shared font/atlas GUIDs, spotlight, rain, SubUV, legacy defaults, invalid inputs\n";
        return 0;
    }
    catch (const std::exception& Error)
    {
        std::cerr << Error.what() << '\n';
        return 1;
    }
}
