# Homestead 아키텍처 설계

## 1. 목표와 전제

Homestead는 C++17과 Direct3D 11로 만드는 소규모 타일 기반 2D 생활 게임이다. Stardew Valley의 전체 규모를 재현하는 대신 농사 중심의 짧고 반복 가능한 핵심 루프와 저장/불러오기에 집중한다. NPC, 계절, 날씨, 멀티플레이는 범위에서 제외한다.

제출물은 `Homestead.exe`와 `data.pak`으로 구성하고 둘의 합계를 플로피디스크 용량인 **1,474,560 bytes(1,440KiB)** 이내로 제한한다. 생성된 저장 파일도 크기 목표에 포함해, 대표 저장 파일까지 합산한 상태에서 같은 한계를 넘지 않도록 설계한다.

확정된 대상 환경과 콘텐츠 조건은 다음과 같다.

- Windows 10 이상, Direct3D feature level 11_0 이상
- x64를 우선 대상으로 사용하고 실제 Release 크기를 측정한 뒤에만 x86 전환을 검토
- 심사 환경에 Visual C++ Runtime이 설치되어 있다고 가정하되, 최종 심사 PC에서 사전 검증
- 16:9 고정 논리 화면, 최소 16×16 타일과 16×16 기반 구매 에셋
- 영어만 지원하므로 작은 ASCII 비트맵 폰트 사용
- 키보드와 마우스, 부드러운 자유 이동, 앞쪽 타일 상호작용
- 오프라인 싱글 플레이 전용
- 배경 음악과 효과음을 모두 제공하되 소수 리소스를 반복 사용
- 제작용 에디터와 패커는 제출물에서 제외

핵심 원칙은 다음과 같다.

- 게임 코드가 Win32 및 Direct3D 타입에 직접 의존하지 않게 한다.
- 객체 소유권은 명확하게 하고, 전역 싱글턴과 무분별한 `shared_ptr`를 피한다.
- 프레임 처리와 고정 주기 시뮬레이션을 분리한다.
- 플레이 중 동적 할당, RTTI, 예외, 리플렉션, 범용 ECS 등 크고 복잡한 기반 시설을 피한다.
- 데이터는 사람이 편집하는 원본과 게임이 읽는 압축된 단일 패키지로 나눈다.
- 화면에 필요한 스프라이트는 가능한 한 한 번의 동적 버텍스 갱신과 소수의 draw call로 그린다.
- 초기부터 실행 파일과 에셋의 크기 예산을 CI/빌드 과정에서 검사한다.

## 2. 전체 계층

```text
Main / WinMain
└─ Application                         생성, 실행, 종료 순서를 소유
   ├─ Platform
   │  ├─ Window                        Win32 창과 메시지 처리
   │  ├─ Clock                         고해상도 시간
   │  └─ FileSystem                    파일 읽기/저장 경로
   ├─ Graphics                         D3D11 장치와 렌더링 자원
   │  ├─ RenderDevice                  device/context/swap chain
   │  ├─ SpriteBatch                   타일·스프라이트 일괄 렌더
   │  ├─ Camera2D                      월드-화면 변환과 컬링 범위
   │  ├─ TextureAtlas                  단일/소수 아틀라스
   │  ├─ TileMapRenderer               보이는 청크만 제출
   │  ├─ TextRenderer                  비트맵 폰트
   │  └─ PostProcess                   선택 사항: 화면 단위 효과
   ├─ Input                            키보드·마우스·패드 상태
   ├─ Audio                            경량 사운드 재생
   ├─ Assets                           패키지 인덱스와 리소스 로드
   ├─ Game                             순수 게임 규칙과 월드 상태
   │  ├─ SceneStack                    Boot/Title/Play/Pause 등
   │  └─ PlayScene
   │     ├─ World                      맵, 시간, 작물, 전역 플래그
   │     ├─ EntityWorld                플레이어/아이템/작물
   │     ├─ Systems                    게임 규칙 갱신
   │     ├─ EventQueue                 프레임 내부의 느슨한 결합
   │     └─ Presentation               애니메이션·이펙트·HUD 연결
   ├─ UI                               메뉴, HUD, 인벤토리, 안내 메시지
   └─ SaveSystem                       버전이 있는 저장 데이터
```

의존성은 위에서 아래로만 흐른다. `Game`은 `ID3D11Device`나 `HWND`를 알지 못한다. 렌더러는 게임 상태를 변경하지 않으며, 필요한 읽기 전용 스냅샷 또는 렌더 명령만 받는다. 규모 제한상 모든 하위 항목이 반드시 별도 클래스일 필요는 없다. 책임의 경계는 유지하되, 구현이 작은 `Clock`, `Color`, `Rect` 등은 헤더의 단순 구조체로 둘 수 있다.

## 3. 최상위 수명과 소유권

### Application

`Application`은 장수 객체의 유일한 조립 지점(composition root)이다.

```cpp
class Application final {
public:
    bool Initialize(HINSTANCE instance);
    int Run();
    void Shutdown();

private:
    Window window_;
    Clock clock_;
    Graphics graphics_;
    Input input_;
    Audio audio_;
    AssetStore assets_;
    SaveSystem saves_;
    Game game_;
    bool running_ = true;
};
```

멤버 선언 순서는 생성/파괴 의존성을 나타낸다. `Application`이 값을 직접 소유하고, 하위 객체는 필요한 서비스에 대한 비소유 참조나 포인터만 받는다. 리소스 핸들은 정수 ID로 표현한다. COM 객체에만 작은 전용 RAII 래퍼 또는 `ComPtr`를 사용한다.

전역 싱글턴은 만들지 않는다. 전역 접근은 테스트, 재시작, 장치 재생성, 종료 순서를 어렵게 한다. 정말 전역적인 상수 데이터만 `constexpr`로 둔다.

## 4. 메인 루프와 시간

게임 규칙은 60Hz 고정 스텝, 렌더링은 가변 프레임으로 처리한다. 이렇게 하면 이동, 충돌, 작물 타이머가 프레임 속도에 따라 달라지지 않는다.

```text
Input::BeginFrame
→ Win32 메시지 처리
→ 실제 경과 시간 측정 및 과도한 delta 제한
→ accumulator에 delta 누적
→ while accumulator >= 1/60초
   → Input 명령 소비
   → Game::FixedUpdate(1/60초)
   → 이벤트 처리
   → accumulator 차감
→ Game::UpdatePresentation(delta)
→ Game::BuildRenderQueue(alpha)
→ Graphics::Render
→ Present
```

- 창 이동이나 디버거 정지 후 폭주하지 않도록 한 프레임의 `delta`를 제한한다.
- 첫 구현은 fixed step을 60Hz, 한 프레임 delta 상한을 0.25초, 프레임당 fixed update 상한을 5회로 둔다. 상한 뒤에도 한 step 이상 남은 누적 시간은 버려 다음 프레임으로 update 폭주가 이어지지 않게 한다.
- 일시정지는 월드 시뮬레이션만 멈추고 UI와 입력은 계속 갱신한다.
- 게임 내 시각은 부동소수 누적값 대신 정수 tick 또는 게임 내 분 단위로 저장한다.
- 첫 콘텐츠 버전은 60 fixed tick마다 게임 1분을 진행하고 오전 6시에 시작한다.
- 별도 침대 sprite가 없는 현재 farm에서는 `N` 명시적 입력으로 하루를 종료한다. 60 tick 화면 전환의 중앙에서 날짜를 정확히 한 번 증가시키고 `DayChanged` 농사 갱신을 호출한다.
- 렌더 보간이 필요하면 엔티티의 이전/현재 위치를 보관하고 `alpha`로 보간한다.
- 백그라운드에서는 프레임을 제한하고, 창 크기가 0이면 렌더링을 건너뛴다.

## 5. 플랫폼 계층

### Window

다음만 담당한다.

- Win32 클래스 등록, 창 생성과 소멸
- 메시지를 크기 변경, 종료, 포커스, 원시 입력 이벤트로 변환
- 논리 해상도와 실제 클라이언트 크기 관리
- 전체 화면/창 모드 전환은 필요할 때만 추가

`WndProc`는 게임 로직을 호출하지 않고 `Window`와 `Input`에 이벤트를 전달한다. 리사이즈 도중에는 GPU가 유휴 상태인지 확인한 뒤 백 버퍼 관련 view를 해제하고 swap-chain buffer를 재생성한다.

### FileSystem

배포물에는 가급적 `Homestead.exe`, `data.pak` 두 파일만 둔다. 쓰기 가능한 저장 위치는 실행 파일 옆이 아니라 `%LOCALAPPDATA%/Homestead`를 사용한다. 에셋 읽기와 사용자 저장 파일 쓰기는 API를 분리한다.

### Settings

`Application`은 시작 시 `%LOCALAPPDATA%/Homestead/settings.cfg`에서 version 1 설정을 먼저 읽고 그 값으로 `Window`를 생성한다. 설정은 640×360, 960×540, 1280×720 창 배율과 borderless fullscreen 여부, 0~10 범위의 master/music/effect 음량을 가진다. 설정 파일은 `HSCF` magic, version, FNV-1a checksum을 검증하며 temporary write, flush, atomic replace 방식으로 갱신한다. 음량은 단계 15의 audio bus가 소비하고 UI와 영속화는 단계 14에서 소유한다.

Escape는 world simulation을 멈추는 pause action이고 `I`는 inventory action이다. pause 메뉴의 keyboard focus와 mouse hover는 같은 항목 index를 갱신하며 keyboard activate와 mouse click은 같은 변경 함수를 사용한다. UI 문구는 `UI/Strings.hpp`에 모으고 bitmap font는 실제 문구에 필요한 대문자와 숫자 글리프만 `data.pak`에 포함한다.

## 6. 그래픽 계층

### Graphics / RenderDevice

소유 대상은 다음과 같다.

- `ID3D11Device`, `ID3D11DeviceContext`
- `IDXGISwapChain`과 back-buffer RTV
- 공용 blend, sampler, rasterizer 상태
- 동적 sprite vertex buffer와 고정 index buffer
- 정점/픽셀 셰이더, input layout, 상수 버퍼
- 선택적인 저해상도 scene render target

초기화 시 하드웨어 장치를 우선하고 실패하면 WARP 사용 여부를 정책으로 결정한다. 디버그 레이어는 개발 빌드에서만 활성화한다. 종료 시에는 GPU 리소스를 명시적으로 해제해 live-object 경고가 없게 한다.

현재 CMake의 `d3dcompiler` 링크는 런타임 셰이더 컴파일을 뜻하지 않는다. 배포 크기와 외부 DLL 문제를 줄이기 위해 **오프라인에서 컴파일한 셰이더 바이트코드**를 패키지에 넣고 `D3DCreateBlob`/런타임 컴파일 경로는 최종 빌드에서 제거하는 편이 좋다. 이 경우 최종적으로 `d3dcompiler` 링크도 불필요한지 확인한다.

### 논리 해상도

16:9 비율의 320×180을 초기 고정 논리 해상도로 사용한다. 16×16 타일 기준으로 화면에 20×11.25 타일이 보이므로 세로 가장자리 처리와 HUD 영역은 실제 에셋을 넣은 첫 데모에서 확인한다. 부족하면 구조 변경 없이 640×360으로 올릴 수 있다. 논리 화면을 창에 정수 배율로 확대하고 남는 부분은 letterbox 처리한다.

클라이언트 영역이 320×180보다 작아 정수 1배 표시가 불가능한 경우에만 16:9 비율을 유지하는 축소 표시를 허용한다. 이 예외 구간에서도 point sampling과 중앙 letterbox를 사용한다. 마우스 좌표는 실제 표시 viewport 안에서만 논리 좌표로 변환하고, letterbox 바깥 입력은 무효로 처리한다.

- 카메라 최종 좌표를 정수 픽셀에 맞춰 흔들림을 방지한다.
- 텍스처는 point sampling, clamp를 기본으로 한다.
- UI는 논리 화면 좌표, 월드는 월드 좌표를 사용한다.
- 마우스 좌표는 letterbox를 제거한 뒤 논리 좌표로 역변환한다.

### SpriteBatch

모든 타일, 캐릭터, 파티클, UI를 공통 `SpriteCommand`로 제출한다.

```cpp
struct SpriteCommand {
    float x, y, w, h;
    RectU16 uv;
    Color32 color;
    uint16_t depth;
    uint8_t material;
    uint8_t flags;
};
```

명령은 `material/texture → layer/depth` 기준으로 정렬하고, 한 번에 동적 버퍼에 복사해 배치한다. 하나의 큰 아틀라스를 사용하면 대부분 한 draw call 흐름으로 처리할 수 있다. 깊이값은 다음처럼 고정한다.

첫 구현은 `RenderQueue`를 1,024 command, GPU sprite batch를 256 sprite로 고정한다. queue 용량을 넘는 제출은 안전하게 거부하고, 같은 texture/material의 sprite가 batch 용량을 넘으면 여러 draw call로 flush한다. 이 값은 실제 타일맵과 UI가 들어온 뒤 메모리 사용량과 draw call 수를 측정해 조정한다.

```text
Ground → GroundDecoration → ObjectBack → Actor(y-sort)
→ ObjectFront → Weather/Effect → UI → Debug
```

반투명 스프라이트는 순서를 보존해야 한다. 캐릭터와 나무처럼 겹치는 물체는 발 위치의 Y를 정렬 키로 사용한다. 핫 루프에서 `std::function`, 가상 호출, 개별 heap allocation을 사용하지 않는다.

### Camera2D와 컬링

카메라는 중심, zoom, viewport와 흔들림 효과만 가진다. `WorldToScreen`, `ScreenToWorld`, 현재 보이는 월드 사각형을 제공한다. `TileMapRenderer`는 이 사각형에서 보이는 타일 범위만 순회하고 한 타일 정도 여유를 둔다. 엔티티와 파티클도 AABB로 화면 밖을 제외한다.

### 텍스트

운영체제 폰트나 DirectWrite 의존성 대신 영어 ASCII 글리프만 담은 작은 비트맵 폰트를 sprite atlas에 포함한다. 실제 사용하지 않는 제어 문자와 기호는 제외한다. UI 문구는 문자열 테이블로 모으고 반복되는 단어는 토큰 치환을 적용할 수 있다.

## 7. 에셋과 빌드 파이프라인

원본 PNG, WAV, JSON, CSV는 저장소의 제작용 데이터이며 배포하지 않는다. 빌드 도구가 이를 단일 `data.pak`으로 변환한다.

```text
assets-src/                 사람이 편집하는 원본
  sprites/
  maps/
  audio/
  data/
tools/asset_packer/         PC 전용 변환기
data.pak                    최종 실행용 패키지
```

패키지는 작은 헤더, 항목 테이블, 정렬된 payload로 구성한다. 이름 문자열 대신 빌드 시 생성한 32비트 ID 또는 해시를 사용하고 충돌은 패커가 검증한다.

```text
PakHeader { magic, version, entryCount, checksum }
PakEntry  { id, type, flags, offset, packedSize, rawSize }
Payloads  { atlas, maps, tables, shaders, audio ... }
```

현재 version 1 pak은 모든 정수 필드를 little-endian으로 기록한다. 32-byte
header에는 `HSPK` magic, version, header/index/payload 위치, file size와
header 이후 전체 checksum을 두고, 24-byte entry에는 `AssetId`, type, flags,
offset, packed/raw size와 payload checksum을 둔다. Entry는 `AssetId` 오름차순으로
정렬하고 payload는 4-byte 경계에 맞춘다. 런타임은 중복 ID, 겹치거나 범위를
벗어난 payload, 알 수 없는 type/flags, checksum 불일치를 모두 거부한다.

첫 atlas payload는 `HSPA` magic과 version, width/height, 최대 256개의 RGBA
palette, 8-bit pixel index로 구성한다. `AssetStore`는 이를 시작 시 RGBA로 한 번
확장해 D3D11 immutable texture를 만든다. Sprite table은 이름 문자열 없이
`AssetId`와 atlas rectangle, trim offset, 원본 sprite 크기만 저장한다.

첫 map payload는 `HSTM` magic의 version 1 little-endian 형식이다. 24-byte
header에 header size, 맵의 tile 크기와 가로/세로 tile 수, chunk 크기,
layer 수, tile record 크기와 tile 수를 기록한다. 뒤이어 각 tile을
`ground u16`, `object u16`, `flags u8`, `variant u8`의 6-byte record로 저장한다.
초기 상한은 16×16 tile, 16×16 tile chunk, 128×128 tile 맵이다. 런타임은
header와 전체 payload 크기, ID와 flag 범위를 검증한 뒤 row-major record를
고정 크기 chunk로 재배치한다. version 1은 비압축으로 측정하고 chunk 압축은
맵 콘텐츠가 늘어 실제 이득이 확인될 때 추가한다.

권장 변환은 다음과 같다.

- 여러 이미지를 palette 기반 atlas 하나 또는 소수로 병합
- UV, animation clip, item/crop 정의를 조밀한 이진 테이블로 변환
- 타일맵을 chunk 단위 RLE/사전 압축하고 빈 레이어 생략
- 셰이더를 release flag로 오프라인 컴파일하고 strip
- PCM 효과음은 낮은 sample rate의 mono ADPCM 등으로 변환
- 음악은 짧은 패턴/시퀀스와 작은 악기 샘플로 재생성하거나 절차 생성

범용 JSON 파서, PNG 디코더, Ogg 디코더를 런타임에 넣는 것은 편하지만 크기 예산에 불리하다. 개발용 포맷은 패커만 읽고 게임은 단순한 자체 이진 포맷만 읽는다. 압축 알고리즘은 압축률뿐 아니라 디코더 코드 크기까지 비교한다. 에셋별 RLE/delta와 전체 패키지용 소형 DEFLATE 계열 등 실제 결과를 측정해 선택한다.

`AssetStore`는 패키지 인덱스와 로드된 GPU/오디오 자원을 소유한다. 개발 중 hot reload는 별도 개발 빌드 기능으로 두며 최종 빌드에서는 제거한다. 큰 월드라면 맵 청크만 지연 로드하고, 매우 작은 전체 패키지라면 시작 시 메모리에 올리는 쪽이 더 단순하다.

현재 패키지는 stable ID `map/farm`과 `map/house` 두 맵을 가진다. 두 맵은 시작 시
검증해 메모리에 유지하고 `Application`이 현재 `MapId`에 해당하는 맵만 렌더링과
충돌에 제공한다. 농작물과 경작 delta는 농장 맵에만 속하며 집에 들어가도 농장
상태는 메모리에 유지된다.

## 8. 월드와 맵

### TileMap

맵은 고정 크기 청크(예: 16×16 또는 32×32)로 나눈다. 각 타일은 여러 객체를 두는 대신 조밀한 정수 ID와 플래그로 표현한다.

```cpp
struct Tile {
    uint16_t ground;
    uint16_t object;
    uint8_t flags;       // blocked, water, tilled 등
    uint8_t variant;
};
```

작물, 상자, 문처럼 상태가 풍부한 대상은 타일 안에 전부 넣지 않고 별도 sparse object 목록에서 해당 좌표와 연결한다. 렌더 레이어, 충돌 레이어, 상호작용 레이어, warp/trigger를 논리적으로 구분한다.

### 충돌과 공간 검색

2D 농장 게임에는 범용 물리 엔진이 필요하지 않다.

- 이동체는 AABB 또는 발 부분의 작은 collision box 사용
- 타일 grid에서 후보 벽만 조회
- X축과 Y축을 따로 해결해 벽 미끄러짐 구현
- 동적 객체는 청크별 ID 목록 또는 uniform grid로 검색
- 공격/도구 범위와 상호작용은 짧은 AABB/ray 검사

결정론이 중요하면 위치와 속도를 고정소수점으로 둘 수 있다. 싱글 플레이만 목표라면 float 위치와 정수 타일 상태로 시작해도 충분하다.

## 9. 엔티티 모델

작은 프로젝트에 대규모 범용 ECS는 과하다. ID 기반의 **고정 용량 component pool + 명시적인 system**을 권장한다.

```text
EntityId = index + generation

EntityWorld
├─ EntityPool
├─ TransformPool
├─ SpritePool
├─ ColliderPool
├─ ActorPool
├─ CropPool
├─ PickupPool
└─ InteractablePool
```

- 각 pool은 연속 배열과 사용 bitset/free list를 가진다.
- `EntityId`의 generation으로 삭제된 ID의 재사용 오류를 잡는다.
- 컴포넌트는 데이터만 보유하고 system이 동작을 수행한다.
- 최대 작물, 드롭, 파티클 수를 정해 메모리를 예측 가능하게 한다.
- 특수한 단일 객체인 `PlayerState`, `WorldClock`은 억지로 ECS에 넣지 않는다.

권장 시스템 순서는 다음과 같다.

1. `PlayerControlSystem`
2. `MovementSystem`
3. `CollisionSystem`
4. `InteractionSystem`
5. `ToolSystem`
6. `CropSystem`
7. `PickupSystem`
8. `TriggerSystem`
9. `AnimationSystem`
10. `CleanupSystem`

엔티티를 순회하는 동안 즉시 생성/삭제하지 않는다. 명령 버퍼에 기록하고 시스템 단계 끝에서 반영한다.

## 10. 게임플레이 도메인

다음 모듈은 처음부터 경계를 정하되, 실제 구현은 필요한 순서대로 추가한다.

### Player와 Inventory

- `PlayerState`: 체력/기력, 돈, 선택 슬롯, 도구 숙련도
- `Inventory`: 고정 슬롯 배열 `{ ItemId, count, quality }`
- `ItemDatabase`: 불변 item definition 테이블
- `Equipment`: 현재 도구와 업그레이드 단계

아이템 동작을 아이템별 C++ 상속 클래스로 만들지 않는다. 데이터의 category/flags와 `UseItemSystem`의 명시적 switch/table로 처리하면 코드와 vtable 크기를 줄이고 저장도 단순해진다.

### Farming

- 경작/물주기 상태는 타일 플래그
- 심어진 작물은 `CropInstance { tile, cropId, plantedDay, watered, stage }`
- 날짜가 바뀔 때만 성장 조건을 계산
- 모든 작물을 매 프레임 갱신하지 않음
- 성장 일수, 물 필요 여부, 수확 횟수는 정적 `CropDefinition`에서 조회

최종 콘텐츠는 밀·당근·토마토 3종이다. 각각 2·3·4번의 물 준 하루가
필요하고, `MarketEntry`의 씨앗 구매가와 수확물 판매가로 작은 경제 루프를
구성한다. 별도 NPC나 대화 시스템 없이 `M` 시장 overlay에서 구매·판매하며,
새 게임은 20G로 시작해 100G 보유를 완료 목표로 삼는다.

### 범위에서 제외하는 시스템

현재 버전에는 NPC, 호감도, NPC 일정, 계절, 날씨, 네트워크/협동 기능을 넣지 않는다. 관련 pool, system, 데이터 테이블, UI를 미리 만들지 않는다. 이후 범위가 변경될 때 독립 모듈로 추가한다.

간단한 안내문이나 튜토리얼이 필요하면 NPC 대화 시스템 대신 문자열 ID를 표시하는 `MessageTrigger`를 사용한다. 복잡한 퀘스트가 실제로 필요해질 경우에만 다음과 같은 작은 명령 집합을 추가한다.

```text
SHOW_TEXT, CHOICE, IF_FLAG, SET_FLAG, GIVE_ITEM,
TAKE_ITEM, ADD_MONEY, START_QUEST, WARP, END
```

### Event

`EventQueue`는 `ItemPickedUp`, `DayChanged`, `CropHarvested` 같은 짧은 값 타입 이벤트를 한 프레임 동안 전달한다. 영구 상태 전달 수단으로 이벤트 큐를 사용하지 않는다. 퀘스트와 컷신용 bytecode VM은 현재 범위에서 제외한다.

### 시간과 날짜

`WorldClock`은 day와 minute를 정수로 관리한다. `DayChanged`에서 작물 성장과 일일 상태를 일괄 갱신한다. 계절과 날씨 상태는 저장하거나 갱신하지 않는다.

## 11. Scene과 UI

`SceneStack`은 다음과 같은 상태를 관리한다.

```text
BootScene → TitleScene → PlayScene
                         ├─ Pause overlay
                         ├─ Inventory overlay
                         └─ Message overlay
```

scene은 입력 차단, 아래 scene 업데이트 여부, 아래 scene 렌더 여부를 명시한다. 메뉴마다 별도의 거대한 프레임워크를 만들지 않고 작은 위젯 집합을 사용한다.

- `Panel`, `Label`, `Icon`, `Button`, `SlotGrid`, `ScrollList`
- 방향키/패드 focus와 마우스 hit-test를 같은 action으로 변환
- 게임 액션(`Interact`, `UseTool`, `Menu`)과 물리 키를 분리
- UI layout은 논리 픽셀과 anchor로 계산
- 안내 메시지가 열리면 필요에 따라 플레이어 제어를 막되 scene 자체는 유지

즉시 모드 UI는 도구 화면에는 편리하지만 게임 UI의 focus, 애니메이션, 저장 상태에는 불편할 수 있다. 작은 retained state와 매 프레임 `SpriteBatch` 제출을 조합한다.

## 12. 입력

`Input`은 플랫폼 이벤트를 다음 세 상태로 정리한다.

- `held`: 현재 눌림
- `pressed`: 이번 프레임에 눌림
- `released`: 이번 프레임에 해제

게임은 키 코드 대신 `Action`을 읽는다. 고정 업데이트가 한 렌더 프레임에 여러 번 실행되어도 `pressed`가 중복 소비되지 않도록 command queue 또는 소비 플래그를 둔다. 포커스를 잃으면 모든 held 상태를 해제한다. 마우스 좌표는 논리 화면 좌표와 월드 좌표로 변환하고, letterbox 바깥 클릭은 무시한다. 게임패드와 XInput 코드는 포함하지 않는다.

초기 키 매핑은 WASD/방향키 이동, E/Space/오른쪽 클릭 상호작용, F/왼쪽 클릭 도구 사용, Escape 메뉴다. 렌더 프레임의 `pressed`와 fixed update가 소비할 pending press를 분리해 fixed update가 없는 고주사율 프레임에서도 단발 입력을 잃지 않는다.

## 13. 오디오

외부 미들웨어는 크기 제한에 불리하다. 구현 후보는 다음 우선순위로 측정한다.

1. XAudio2를 통한 자체 PCM/ADPCM voice 관리
2. 아주 작은 tracker/pattern 재생기
3. 절차적 효과음 생성

`Audio`는 effect/music bus 음량, 제한된 voice pool, 중복 효과음 억제를 담당한다. 월드 객체는 직접 voice를 잡지 않고 `PlaySound { soundId, volume, pan }` 명령만 보낸다. 음악 스트리밍을 쓸 경우 이중 버퍼를 사용하고 메인 스레드에서 디스크 I/O를 하지 않는다. 다만 전체 데이터가 매우 작다면 압축 데이터를 메모리에 올리는 편이 단순하다.

version 1 오디오는 AssetPacker가 16-bit PCM WAV를 8kHz mono 2-bit ADPCM `HSA2` payload로 결정적으로 변환해 pak type 4 항목으로 저장한다. 런타임은 시작 시 선택된 음악 1개와 효과음 6개를 PCM으로 한 번만 복원하고, XAudio2 source voice가 해당 고정 버퍼를 참조한다. 음악은 source buffer 전체를 무한 반복하며 효과음은 4개 voice pool을 공유한다. 동일 효과음이 아직 재생 중이면 새 요청을 버려 과도한 중복을 막는다. mastering voice는 master 설정을, music/effect source voice는 각 bus 설정을 적용한다. 출력 장치가 없거나 XAudio2 초기화가 실패하면 게임은 무음으로 계속 실행한다.

## 14. 저장/불러오기

저장 파일은 런타임 객체의 메모리 덤프가 아니라 명시적인 필드 기반 이진 포맷이어야 한다.

```text
SaveHeader { magic, version, payloadSize, checksum }
Player     { position, stats, inventory, equipment }
World      { time, global flags }
MapDelta[] { mapId, changed tiles/objects }
CropState[]
```

- 정적 정의와 원본 맵은 저장하지 않고 ID로 참조한다.
- 변경된 타일과 열린 상자처럼 원본과 다른 delta만 저장한다.
- 임시 파일에 기록하고 flush/close한 뒤 기존 파일을 교체한다.
- 이전 저장 파일 하나를 backup으로 유지한다.
- version별 migration 또는 최소한 호환성 거부 메시지를 둔다.
- ID 범위, 길이, checksum을 검증한 후 적용한다.
- 저장 중에는 월드 스냅샷을 고정해 부분 변경을 막는다.

자동 저장은 하루 종료 같은 안전한 시점에 실행한다. 집 침대 상호작용은 하루 종료
전환을 요청하며, 다음 날 오전 6시로 바뀌고 작물 갱신이 끝난 뒤 저장한다.

version 3 save는 `HSSV` magic, 16-byte header, little-endian payload와 FNV-1a
checksum을 사용한다. 플레이어 위치는 1/256 logical pixel 정수로 저장하고,
inventory 16칸은 명시적인 item/count 쌍으로 기록한다. 원본 map 대신
`Tilled`/`Watered` tile delta와 활성 crop만 저장하며 골드와 작물별 누적
물주기 일수, stable `MapId`와 현재 맵의 플레이어 위치도 명시적으로 기록한다.
version 1의 당근·도구 item/crop ID와 version 2의 단일 농장 위치는 version 3으로
마이그레이션한다. 전체 파일은 32KiB를
초과하면 거부한다. `%LOCALAPPDATA%/Homestead/representative.sav`를 primary로,
`.tmp`와 `.bak`을 각각 임시 파일과 직전의 검증된 저장으로 사용한다.
시작 시 primary가 유효하지 않으면 backup을 시도하고, 정상 종료와 하루 변경
직후 자동 저장한다.

## 15. 메모리와 성능 정책

1.44MB는 주로 디스크 크기 제한이지 RAM 제한은 아닐 가능성이 크다. 그래도 단순하고 예측 가능한 구조가 코드 크기에도 유리하다.

- 시작 시 장수 메모리를 예약하고 프레임 중 heap allocation을 피한다.
- frame arena를 렌더 명령, 임시 경로, 이벤트에 사용하고 매 프레임 한 번 초기화한다.
- string 대신 `StringId`, asset pointer 대신 작은 handle을 저장한다.
- 컨테이너 최대 용량을 실제 게임 디자인 수치에서 정한다.
- `unordered_map`은 에디터/패커에는 허용하되 런타임에서는 정렬 배열과 binary search를 우선한다.
- 로그, assert, 이름 테이블, 디버그 UI는 개발 빌드에만 포함한다.
- gameplay과 렌더링은 단일 스레드의 결정적 흐름을 유지한다. 시작 시 측정된 로딩 끊김만
  일회성 `StartupLoader` 작업 스레드로 숨기며, 이 스레드는 pak·맵·오디오 PCM·save만
  준비한다. HWND, 입력, D3D11과 XAudio2 장치 객체는 메인 스레드가 계속 소유하고,
  원자적 단계 상태와 종료 시 join 외의 범용 job system은 도입하지 않는다.

## 16. 1.44MB 크기 전략

제출 크기 상한은 정확히 `1,474,560 bytes`, 즉 `1,440KiB`다. 아래 예산은 대표 저장 파일까지 포함한 전체 목표이며, 표의 KB는 모두 KiB(1,024 bytes)를 뜻한다.

| 항목 | 목표 예산 |
|---|---:|
| PE 실행 파일과 게임 코드 | 340KiB |
| 셰이더 바이트코드 | 16KiB |
| sprite/font atlas | 430KiB |
| 맵과 정의/문자열 | 180KiB |
| 음악과 효과음 | 330KiB |
| pak 인덱스와 정렬 비용 | 48KiB |
| 대표 저장 파일 예산 | 32KiB |
| 안전 여유 | 64KiB |
| 합계 | 1,440KiB |

이는 출발점일 뿐이며 매 빌드에서 실제 수치로 조정한다. 구매한 원본 에셋은 압축 상태로 약 5.6MB이므로 필요한 16×16 타일과 sprite만 선별하고 palette 축소, 중복 제거, atlas 압축을 적용해야 한다. 오디오와 선별 후의 atlas가 가장 큰 위험 요소다.

Release 빌드 원칙:

- MSVC `/O1`(크기 최적화), `/GL`, 링커 `/LTCG`, `/OPT:REF`, `/OPT:ICF`
- RTTI가 불필요하면 `/GR-`; 예외를 쓰지 않는 정책이면 `/EH` 설정과 표준 라이브러리 영향을 검증
- 디버그 심볼은 별도 PDB로 만들고 배포물에서 제외
- 개발용 로그, 콘솔, 에디터, hot reload, GPU debug layer 제외
- 정적 CRT(`/MT`)와 동적 CRT(`/MD`)는 “배포물에 필요한 런타임 포함 여부”까지 합산해 둘 다 측정
- 실행 파일 패커 사용은 허용되는 것으로 보지만, UPX 등의 백신 오탐과 심사 PC 실행 여부를 검사하고 패커 없이도 목표에 가까운 빌드를 유지

크기 검사는 빌드 실패 조건으로 만든다.

```text
build release
→ exe + data.pak + 대표 save byte 합산
→ 제한 초과 시 실패
→ map 파일 또는 size report로 증가 원인 표시
```

템플릿 남용, iostream, locale, regex, filesystem, 범용 직렬화 라이브러리는 작은 코드에서도 바이너리를 크게 만들 수 있다. 무조건 금지하기보다 linker map으로 비용을 측정하고 작은 Win32/자체 구현과 비교한다.

## 17. 오류 처리와 진단

최종판은 작아야 하지만 개발판의 관측 가능성은 유지한다.

- `Result`/오류 코드로 초기화 실패를 위로 전달
- D3D debug layer와 live-object report는 Debug에서 사용
- 고정 크기 ring log와 화면 debug overlay 제공
- FPS, update/render 시간, draw calls, sprites, active entities, 메모리, 패키지 크기 표시
- GPU 생성 실패, 손상된 pak/save, 누락된 ID에는 안전한 fallback 또는 명확한 종료 메시지

장치 제거가 발생하면 최소한 오류를 기록하고 안전하게 종료한다. 완전한 device-lost 복구는 개발 비용 대비 필요성을 확인한 후 구현한다.

## 18. 권장 디렉터리 구조

```text
Homestead/
├─ CMakeLists.txt
├─ ARCHITECTURE.md
├─ include/Homestead/
│  ├─ App/              Application.hpp
│  ├─ Platform/         Window.hpp, Clock.hpp, FileSystem.hpp
│  ├─ Graphics/         Graphics.hpp, SpriteBatch.hpp, Camera2D.hpp
│  ├─ Input/            Input.hpp, Action.hpp
│  ├─ Audio/            Audio.hpp
│  ├─ Assets/           AssetStore.hpp, AssetId.hpp, PakFormat.hpp
│  ├─ Core/             Types.hpp, Math.hpp, Handle.hpp, Arena.hpp
│  ├─ Game/             Game.hpp, Scene.hpp, PlayScene.hpp
│  ├─ World/            World.hpp, TileMap.hpp, EntityWorld.hpp
│  ├─ Systems/          MovementSystem.hpp, CropSystem.hpp, ...
│  ├─ UI/               UiContext.hpp, Widgets.hpp
│  └─ Save/             SaveSystem.hpp, SaveFormat.hpp
├─ source/              include 구조와 대응하는 .cpp
├─ assets-src/          원본; 배포 제외
├─ shaders/             HLSL 원본; 배포 제외
├─ tools/asset_packer/  게임과 별도 target
└─ tests/               순수 게임 규칙/직렬화 테스트
```

폴더가 많아지는 것 자체가 목표는 아니다. 한 책임의 파일이 작을 때는 합치고, 의존성 경계가 달라질 때 분리한다. 공개 헤더에는 Win32 헤더를 가급적 노출하지 않고 PImpl보다 전방 선언과 전용 platform 헤더를 사용한다. PImpl은 컴파일 경계에는 좋지만 heap allocation과 간접 호출이 늘 수 있으므로 선택적으로 사용한다.

## 19. 핵심 인터페이스 초안

```cpp
class Game final {
public:
    Game(AssetStore& assets, Audio& audio, SaveSystem& saves);
    bool Initialize();
    void HandleInput(const InputSnapshot& input);
    void FixedUpdate(float fixedDelta);
    void UpdatePresentation(float realDelta);
    void BuildRenderQueue(RenderQueue& out, float alpha) const;
};

class Graphics final {
public:
    bool Initialize(Window& window);
    void Resize(uint32_t width, uint32_t height);
    void Render(const RenderQueue& queue);
    void Present(bool vsync);
};

class AssetStore final {
public:
    bool OpenPackage(const wchar_t* path);
    ByteView Get(AssetId id) const;
    TextureHandle GetTexture(AssetId id);
};

class Scene {
public:
    virtual ~Scene() = default;
    virtual void HandleInput(const InputSnapshot&) = 0;
    virtual void FixedUpdate(float) = 0;
    virtual void BuildRenderQueue(RenderQueue&) const = 0;
};
```

`Scene` 정도의 낮은 빈도 다형성에는 virtual을 허용할 수 있다. 수천 개 엔티티나 sprite처럼 뜨거운 경로에서는 데이터 지향 배열과 enum/함수로 처리한다. 실제 크기 측정에서 문제가 되면 scene도 tagged union으로 바꿀 수 있다.

## 20. 구현 순서

### 단계 1: 화면을 띄우는 최소 수직 단면

- `Application`, `Window`, `Clock`, `Graphics`
- D3D11 device/swap chain과 clear/present
- 오프라인 셰이더 1쌍, atlas 1장, `SpriteBatch`
- 고정 논리 해상도와 `Camera2D`
- Release 크기 보고서

### 단계 2: 움직일 수 있는 한 화면

- `Input` action mapping
- `TileMap`, 카메라 컬링, 플레이어 이동과 tile collision
- animation clip과 Y 정렬
- HUD 및 비트맵 텍스트

### 단계 3: 핵심 게임 루프

- 인벤토리, 도구, 경작, 물주기, 작물 성장과 수확
- 시간/날짜 전환
- 맵 전환과 상호작용
- 저장/불러오기와 버전 검사

### 단계 4: 생활 게임 시스템

- 필요한 경우 제작 또는 상점 중 핵심 루프에 직접 기여하는 기능 하나
- 안내용 `MessageTrigger`와 최소 목표/완료 조건
- 오디오와 설정

### 단계 5: 크기 및 안정화

- 패키지 압축 방식 A/B 측정
- linker map 기반 dead code 제거
- 장시간 플레이, 저장 손상, 포커스/리사이즈 테스트
- 저사양/WARP 및 다양한 배율 테스트
- 최종 배포물 byte 단위 제한 검사

각 단계는 플레이 가능한 빌드로 끝나야 한다. 시스템을 전부 빈 클래스로 먼저 만드는 대신, 한 기능을 입력부터 저장과 렌더링까지 수직으로 완성하면서 위 경계를 지킨다.

## 21. 피해야 할 구조

- 모든 것이 접근하는 `ServiceLocator` 또는 싱글턴 집합
- `GameObject` 하나가 입력, 물리, 렌더, 저장을 모두 처리하는 구조
- 아이템 종류마다 C++ 상속 클래스를 추가하는 구조
- 렌더러가 월드 상태를 변경하거나 게임 시스템이 D3D 리소스를 소유하는 구조
- 모든 엔티티를 매 프레임 검사하는 구조
- 원본 PNG/JSON/WAV를 그대로 배포하고 런타임 디코더를 다수 포함하는 구조
- 저장 파일에 포인터, padding이 있는 raw struct, 런타임 entity index를 그대로 기록하는 구조
- 실측 없이 “작을 것”이라고 가정하고 마지막에만 용량을 줄이는 방식

## 22. 첫 구현에서 고정할 결정

코딩을 시작하기 전에 다음 값을 작은 설계 상수 문서나 `Config.hpp`에 확정한다.

- 정확한 배포 크기 정의와 byte 상한
- 최종 x64 Release와 심사 PC 런타임 검증 결과
- 논리 해상도, tile 크기, 최대 atlas 크기
- 최대 엔티티/작물/파티클/인벤토리 수
- map chunk 크기와 최대 월드 크기
- 고정 update rate와 day 길이
- 지원 입력 장치
- 한국어 표기 전략
- 음악 포맷과 오디오 예산
- 저장 슬롯 수와 32KiB 저장 예산, 호환성 정책

이 값들은 클래스 수보다 더 큰 구조적 영향을 준다. 특히 플랫폼 아키텍처, 글꼴, 오디오, 에셋 압축은 1.44MB 달성 가능성을 초기에 결정하므로 첫 번째 플레이어 이동 데모와 동시에 실제 크기를 측정해야 한다.
