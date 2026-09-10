# JungleTechLab Engine

DirectX 11 기반의 언리얼 스타일 미니 게임 엔진 / 씬 에디터입니다.
Week2 팀 과제 결과물이며, Week3의 공통 시작 엔진으로 사용됩니다.

```
JungleTechLab_Week3.sln  →  Visual Studio 2022+ 에서 열기  →  x64/Debug  →  F5
```

## 문서

처음 오셨다면 **[Docs/README.md](Docs/README.md)** 부터 보세요.

| 문서 | 내용 |
|---|---|
| [01. 엔진 개요와 설계 철학](Docs/01_엔진_개요와_철학.md) | 무엇을 만든 엔진인가, 왜 이렇게 설계했는가 |
| [02. 전체 아키텍처](Docs/02_아키텍처.md) | 모듈 구성, 프레임 흐름, 오브젝트 시스템, 렌더링, 피킹 |
| [03. 사용 설명서](Docs/03_사용_설명서.md) | 빌드, 조작법, 씬 저장/로드, 테스트 실행 |
| [04. 확장 가이드](Docs/04_확장_가이드.md) | 새 도형 / 컴포넌트 / 액터 추가 체크리스트 |
| [05. 발표용 요약](Docs/05_발표용_요약.md) | 10분 발표 대본 + 예상 질문 답변 |

## 저장소 구성

```
JungleTechLab_Week3.sln
├── JungleTechLab_Week3/        엔진 본체
│   ├── main.cpp                WinMain / WndProc
│   ├── LaunchEngineLoop.*      FEngineLoop — Init / Tick / End (전체 흐름의 시작점)
│   ├── Object.*, ObjectFactory.*  오브젝트 시스템 + 리플렉션
│   ├── World.*, Actor.*, *Component.*  월드 / 액터 / 컴포넌트 계층
│   ├── SceneManager.*          에디터 UI + 씬 저장·로드
│   ├── FEditorViewportClient.*, Camera.h, Gizmo.*  카메라 / 피킹 / 기즈모
│   ├── GraphicsManager.*, Renderer.*, ShaderW0.hlsl  렌더링
│   ├── T*.h                    자체 컨테이너 (TArray/TMap/TSet/TQueue/TSparseArray)
│   ├── ImGui/, Json/           외부 라이브러리 (소스 동봉)
│   └── Assets/SceneData/*.Scene  씬 파일 (JSON)
├── Test/UnitTest/              googletest 단위 테스트
└── Docs/                       이 문서들
```
