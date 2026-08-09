# Homestead 구현 로드맵

## 1. 진행 원칙

이 로드맵은 [ARCHITECTURE.md](ARCHITECTURE.md)의 설계를 실제 코드로 옮기는 순서다. 모든 클래스를 빈 골격으로 먼저 만드는 대신, 매 단계가 실행 가능한 결과물로 끝나는 수직 구현 방식을 사용한다.

최종 제출물의 제한은 다음과 같다.

```text
Homestead.exe + data.pak + 대표 저장 파일 ≤ 1,474,560 bytes
```

각 단계에서 지켜야 할 공통 규칙은 다음과 같다.

- Debug 빌드로 기능과 D3D11 오류를 검사한다.
- Release 빌드로 실행 파일과 에셋 크기를 기록한다.
- 이전 단계의 실행 가능한 상태를 깨뜨린 채 다음 단계로 넘어가지 않는다.
- 현재 단계에서 사용하지 않을 시스템은 미리 구현하지 않는다.
- 에디터, 패커, 테스트 도구는 게임 실행 파일과 별도 target으로 만든다.
- NPC, 계절, 날씨, 게임패드, 멀티플레이는 현재 범위에서 제외한다.

## 2. 단계별 작업 순서

## 단계 0: 프로젝트 기준선 정리

### 작업

- 현재 삭제 상태인 초기 소스 대신 새 디렉터리 구조를 생성한다.
- CMake target을 `Homestead`와 향후 추가할 도구 target으로 구분할 수 있게 정리한다.
- Debug와 Release 빌드 옵션을 분리한다.
- Windows 10 이상, x64, C++17, Direct3D 11.0을 기본 대상으로 확정한다.
- 경고 수준 `/W4`, UTF-8 소스, `WIN32_LEAN_AND_MEAN`, `NOMINMAX`를 유지한다.
- Debug에 D3D11 debug layer와 assertion을 활성화한다.
- Release에 `/O1`, `/GL`, `/LTCG`, `/OPT:REF`, `/OPT:ICF`를 적용한다.
- 빌드 결과의 byte 크기를 출력하는 간단한 CMake script를 추가한다.

### 생성 대상

```text
include/Homestead/
source/
shaders/
assets-src/
tools/
tests/
```

### 완료 조건

- 깨끗한 디렉터리에서 CMake configure가 성공한다.
- 비어 있는 Win32 프로그램의 Debug/Release 빌드가 모두 성공한다.
- Release EXE 크기가 빌드 로그에 byte 단위로 표시된다.

## 단계 1: Win32 애플리케이션 골격

### 작업

- `wWinMain` 진입점을 작성한다.
- `Application`이 초기화, 메인 루프, 종료 순서를 소유하게 한다.
- `Window`에서 Win32 창 클래스 등록, 창 생성, 메시지 처리를 구현한다.
- `WM_CLOSE`, `WM_DESTROY`, `WM_SIZE`, focus 변경을 처리한다.
- `Clock`에서 `QueryPerformanceCounter` 기반 시간을 제공한다.
- 종료 시 객체가 생성 순서의 역순으로 정리되는지 확인한다.

### 주요 파일

```text
source/Main.cpp
include/Homestead/App/Application.hpp
source/App/Application.cpp
include/Homestead/Platform/Window.hpp
source/Platform/Window.cpp
include/Homestead/Platform/Clock.hpp
source/Platform/Clock.cpp
```

### 완료 조건

- 16:9 클라이언트 영역을 가진 창이 열린다.
- 창 닫기와 Alt+F4가 정상 작동한다.
- 창 크기 변경과 최소화에서 충돌하지 않는다.
- idle 상태에서 메시지 루프가 정상적으로 반복된다.

## 단계 2: Direct3D 11 초기화

### 작업

- `Graphics` 또는 내부 `RenderDevice`를 구현한다.
- D3D11 device, immediate context, DXGI swap chain을 생성한다.
- back-buffer RTV를 생성하고 창 크기 변경 시 재생성한다.
- 화면 clear와 `Present`를 구현한다.
- Debug 빌드에서 D3D11 validation message를 확인한다.
- 종료 시 모든 COM 리소스가 해제되는지 검사한다.

### 완료 조건

- 창이 지정한 색상으로 매 프레임 지워진다.
- 창 크기 변경 후에도 정상 출력된다.
- 최소화 상태에서 불필요한 렌더링이나 오류가 발생하지 않는다.
- 종료 시 D3D11 live object 경고가 없다.

### 크기 점검

- `Homestead.exe` 크기를 기록한다.
- Debug 전용 코드가 Release에 들어가지 않았는지 확인한다.
- 아직 런타임 셰이더 컴파일은 추가하지 않는다.

## 단계 3: 고정 논리 화면과 letterbox

### 작업

- 320×180 RGBA scene render target을 생성한다.
- 게임 화면은 scene target에 렌더링하고 마지막에 back buffer로 확대한다.
- 창 크기에 맞는 최대 정수 배율과 중앙 letterbox 영역을 계산한다.
- 창이 논리 해상도보다 작을 때의 축소 정책을 정의한다.
- 물리 마우스 좌표를 320×180 논리 좌표로 변환한다.
- letterbox 바깥의 마우스 입력은 무효 처리한다.

### 완료 조건

- 여러 창 크기에서 화면 비율이 찌그러지지 않는다.
- 정수 배율에서는 픽셀 경계가 선명하다.
- 마우스로 가리킨 논리 픽셀과 실제 표시 위치가 일치한다.

## 단계 4: 최소 SpriteBatch

### 작업

- position, UV, color를 받는 최소 HLSL 셰이더를 작성한다.
- 셰이더를 빌드 시 오프라인 컴파일한다.
- Release에는 HLSL 원문과 런타임 컴파일 코드를 넣지 않는다.
- point sampler와 alpha blend state를 생성한다.
- 동적 vertex buffer와 고정 index buffer를 만든다.
- `SpriteCommand`, `RenderQueue`, `SpriteBatch`를 구현한다.
- 하나의 테스트 texture에서 16×16 sprite를 출력한다.
- layer/depth 정렬과 Y 정렬을 구현한다.

### 완료 조건

- 한 장 및 여러 장의 sprite가 올바른 UV와 투명도로 출력된다.
- 동일 texture의 sprite가 한 batch로 제출된다.
- sprite 수가 batch 용량을 넘으면 안전하게 flush된다.
- RenderDoc 또는 D3D11 debug layer에서 잘못된 resource binding이 없다.

### 크기 점검

- 셰이더 bytecode 크기를 별도로 기록한다.
- `d3dcompiler`가 최종 런타임 링크에 정말 필요한지 확인한다.

## 단계 5: 입력과 고정 업데이트 루프

### 작업

- `Input`에서 키보드와 마우스의 held/pressed/released 상태를 관리한다.
- 물리 키를 `MoveUp`, `MoveDown`, `MoveLeft`, `MoveRight`, `Interact`, `UseTool`, `Menu` 같은 action으로 변환한다.
- focus를 잃으면 held 상태를 해제한다.
- 60Hz fixed update accumulator를 `Application`에 구현한다.
- 긴 프레임 이후 update 폭주를 막도록 delta와 최대 반복 횟수를 제한한다.
- 한 렌더 프레임에 fixed update가 여러 번 실행되어도 pressed 입력은 한 번만 소비한다.

### 완료 조건

- 프레임 속도를 바꿔도 테스트 sprite의 이동 속도가 같다.
- 키를 누르고 있는 상태와 한 번 누른 상태가 구분된다.
- 창 focus를 잃었다가 돌아와도 입력이 고착되지 않는다.
- 마우스 위치가 논리 화면 좌표로 정확히 보고된다.

## 단계 6: 에셋 패커의 최소 버전

런타임 PNG 디코더와 JSON 파서를 먼저 추가하지 않는다. 첫 sprite 이후 바로 패커를 만들어 이후 콘텐츠가 최종 배포 형식으로 들어오게 한다.

### 작업

- 별도 `AssetPacker` 실행 파일 target을 만든다.
- 패커가 원본 이미지와 간단한 메타데이터를 읽게 한다.
- 필요한 16×16 구매 에셋만 선별한다.
- 동일 타일 제거, 불필요한 투명 영역 제거, palette 축소를 적용한다.
- sprite와 ASCII font를 하나 또는 소수의 atlas로 병합한다.
- asset 이름을 정수 `AssetId`로 변환하고 충돌을 검사한다.
- `data.pak` 헤더, 인덱스, payload를 작성한다.
- 런타임 `AssetStore`가 pak을 열고 texture를 생성하게 한다.

### 완료 조건

- 게임이 원본 PNG 없이 `data.pak`만으로 테스트 sprite를 출력한다.
- 잘못된 magic/version/offset을 가진 pak을 안전하게 거부한다.
- 같은 입력으로 만든 pak의 내용과 크기가 재현 가능하다.
- 패커 자체는 제출 파일에 포함되지 않는다.

### 크기 점검

- 원본 5.6MB 중 실제 선택한 에셋 목록과 변환 후 크기를 기록한다.
- sprite/font atlas 목표인 430KiB를 넘으면 즉시 아트 범위를 줄인다.

## 단계 7: 카메라와 타일맵

### 작업

- `Camera2D`의 `WorldToScreen`, `ScreenToWorld`, visible bounds를 구현한다.
- 카메라 위치를 논리 픽셀 경계에 맞춘다.
- `TileMap`을 고정 크기 chunk로 나눈다.
- ground, object, collision 정보를 작은 정수 ID와 flag로 저장한다.
- 현재 카메라에 보이는 타일과 여유 한 줄만 `SpriteBatch`에 제출한다.
- 테스트 맵을 패커에서 이진 형식으로 변환한다.

### 완료 조건

- 카메라 이동 중 타일 경계가 흔들리거나 갈라지지 않는다.
- 화면 밖 타일은 렌더 명령에 들어가지 않는다.
- 맵 가장자리 접근에서 배열 범위를 벗어나지 않는다.
- ground와 object layer 순서가 정확하다.

## 단계 8: 플레이어와 충돌

### 작업

- `PlayerState`와 최소 `EntityWorld`를 만든다.
- 플레이어의 현재/이전 위치, sprite, 이동 속도를 저장한다.
- 입력 방향을 정규화해 대각선 이동이 빨라지지 않게 한다.
- 발 부분의 작은 AABB로 tile collision을 검사한다.
- X축과 Y축 충돌을 나눠 해결한다.
- 카메라가 플레이어를 따라가되 맵 경계를 벗어나지 않게 한다.
- 발 위치 Y를 기준으로 object와 player의 렌더 순서를 정한다.

### 완료 조건

- 플레이어가 맵을 자유롭게 이동한다.
- 벽과 물체를 통과하지 않는다.
- 대각선 이동 속도가 축 이동과 같다.
- 나무 같은 전경 물체의 앞뒤 가림이 자연스럽다.
- 서로 다른 프레임 속도에서도 같은 이동 결과를 얻는다.

## 단계 9: 상호작용과 도구

### 작업

- 플레이어 방향과 앞쪽 tile 계산을 구현한다.
- 마우스가 가리키는 tile 계산과 최대 작업 거리를 적용한다.
- 선택된 tile을 개발용 overlay로 표시한다.
- `Interact`와 `UseTool`을 분리한다.
- hoe와 watering can의 최소 동작을 구현한다.
- 도구 사용 중 짧은 상태와 animation을 관리한다.
- 동일 프레임 중복 사용을 방지한다.

### 완료 조건

- 키보드와 마우스로 의도한 한 tile을 정확하게 선택한다.
- 범위 밖 tile에는 도구를 사용할 수 없다.
- 충돌 tile이나 금지된 지면에 경작할 수 없다.
- 도구 animation과 실제 상태 변경 시점이 일치한다.

## 단계 10: 인벤토리와 아이템

### 작업

- `ItemId`, `ItemDefinition`, 고정 슬롯 `Inventory`를 구현한다.
- 아이템 stack 추가, 제거, 이동, 교환을 구현한다.
- 선택 hotbar와 간단한 inventory overlay를 만든다.
- 씨앗과 수확물 아이템을 정의한다.
- 마우스 hit-test와 키보드 메뉴 조작을 연결한다.

### 완료 조건

- 씨앗을 획득하고 선택하고 소비할 수 있다.
- stack 최대치와 빈 슬롯 처리가 정확하다.
- inventory가 가득 찬 경우 아이템이 사라지지 않는다.
- 메뉴가 열려 있을 때 플레이어 입력이 차단된다.

## 단계 11: 농사 핵심 루프

### 작업

- tile에 tilled/watered 상태를 기록한다.
- `CropDefinition`과 조밀한 `CropInstance` pool을 만든다.
- 씨앗 심기, 물주기, 날짜 변경, 성장, 수확을 구현한다.
- 모든 작물을 매 프레임 갱신하지 않고 `DayChanged`에서 처리한다.
- crop stage에 맞는 sprite를 출력한다.
- 수확물이 inventory로 들어가지 못할 때의 규칙을 정한다.

### 완료 조건

다음 루프가 처음부터 끝까지 가능해야 한다.

```text
땅 경작 → 씨앗 선택 → 심기 → 물주기
→ 하루 종료 → 성장 → 최종 단계 → 수확 → 인벤토리 획득
```

- 물을 주지 않은 날의 성장 규칙이 일관적이다.
- 같은 tile에 작물을 중복 배치할 수 없다.
- 날짜를 여러 번 넘겨도 crop pool과 tile 상태가 손상되지 않는다.

## 단계 12: 시간, 하루 종료와 최소 목표

### 작업

- `WorldClock`을 day/minute 정수로 구현한다.
- 게임 시간의 실제 진행 배율을 정한다.
- 침대 또는 명시적인 상호작용으로 하루를 종료한다.
- 화면 fade 후 `DayChanged`를 발생시킨다.
- 간단한 안내용 `MessageTrigger`를 추가한다.
- 공모전 버전에 필요한 최소 목표와 완료 화면을 만든다.

### 완료 조건

- 일시정지와 메뉴 중에는 월드 시간이 흐르지 않는다.
- 하루 변경이 정확히 한 번만 처리된다.
- 작물 성장과 일일 상태 초기화 순서가 일정하다.
- 플레이어가 게임의 목표와 완료 여부를 알 수 있다.

## 단계 13: 저장과 불러오기

### 작업

- version과 checksum이 있는 명시적 이진 저장 포맷을 만든다.
- 플레이어 위치, 날짜/시간, inventory, tool 상태를 저장한다.
- 원본 맵과 다른 tile delta 및 crop 상태만 저장한다.
- `%LOCALAPPDATA%/Homestead`에 저장한다.
- 임시 파일 기록 후 기존 save를 교체하고 이전 save를 backup으로 남긴다.
- 손상된 길이, 잘못된 ID, 알 수 없는 version을 검증한다.
- 대표적인 장기 플레이 상태에서도 저장 파일을 32KiB 이내로 유지한다.

### 완료 조건

- 종료 후 다시 실행해 같은 월드 상태를 복원한다.
- 저장 도중 실패해도 직전 정상 save를 불러올 수 있다.
- 손상된 save 때문에 crash하거나 임의 메모리를 읽지 않는다.
- 여러 번 저장해도 불필요하게 파일이 커지지 않는다.

## 단계 14: UI와 설정 정리

### 작업

- HUD, hotbar, inventory, pause, message, 완료 화면을 정리한다.
- 영어 ASCII 비트맵 폰트의 실제 사용 글리프만 포함한다.
- 창 크기, 전체 화면 여부, master/music/effect 음량을 설정으로 둔다.
- UI focus와 마우스 hover/click을 동일 action 흐름으로 처리한다.
- UI 문구를 문자열 테이블로 모은다.

### 완료 조건

- 모든 기능을 키보드와 마우스로 사용할 수 있다.
- UI가 320×180 안전 영역을 벗어나지 않는다.
- 긴 문구가 panel 밖으로 넘치지 않는다.
- 게임 재시작 후 설정이 복원된다.

## 단계 15: 오디오

### 작업

- XAudio2 기반의 작은 voice pool을 구현한다.
- music/effect bus와 음량 조절을 구현한다.
- 경작, 물주기, 심기, 수확, UI에 필요한 최소 효과음을 선정한다.
- 소수의 배경 음악을 낮은 sample rate/mono 또는 작은 pattern 형식으로 변환한다.
- 동일 효과음의 과도한 중복 재생을 제한한다.
- 오디오 데이터도 `data.pak`에 포함한다.

### 완료 조건

- 장시간 실행과 scene 전환 후 voice가 누수되지 않는다.
- 여러 효과음이 동시에 재생되어도 clipping이 과도하지 않다.
- 음량 0과 mute가 정상 작동한다.
- 음악과 효과음이 배정된 330KiB 예산 안에 들어간다.

## 단계 16: 콘텐츠와 게임 완결

### 작업

- 실제 사용할 tile, player, crop, item sprite만 최종 선별한다.
- 맵 수와 크기를 공모전 범위에 맞게 확정한다.
- 작물, 씨앗, 수확물과 도구 수치를 조정한다.
- 필요할 경우 제작 또는 상점 중 핵심 루프에 기여하는 기능 하나만 선택한다.
- 시작 안내, 진행 목표, 완료 조건을 연결한다.
- 플레이어가 막히는 지점과 반복 작업 시간을 playtest한다.

### 완료 조건

- 새 게임부터 완료 화면까지 진행할 수 있다.
- 저장/불러오기 후에도 완료까지 진행할 수 있다.
- 사용되지 않는 이미지, 문자열, 아이템, 맵 데이터가 pak에 없다.
- NPC, 계절, 날씨가 없어도 게임의 목표와 진행 흐름이 이해된다.

## 단계 17: 크기 최적화

기능이 완성되기 전에도 크기를 측정하지만, 이 단계에서 최종 예산을 확정한다.

### 목표 예산

| 항목 | 목표 |
|---|---:|
| PE 실행 파일과 게임 코드 | 340KiB |
| 셰이더 bytecode | 16KiB |
| sprite/font atlas | 430KiB |
| 맵과 정의/문자열 | 180KiB |
| 음악과 효과음 | 330KiB |
| pak 인덱스와 정렬 비용 | 48KiB |
| 대표 저장 파일 | 32KiB |
| 안전 여유 | 64KiB |
| 합계 | 1,440KiB |

### 작업

- linker map으로 실행 파일에서 큰 함수와 라이브러리를 찾는다.
- 사용하지 않는 코드, 로그, debug 이름, 에셋을 제거한다.
- `iostream`, locale, regex, filesystem, RTTI, 예외의 실제 비용을 확인한다.
- `/MT`와 `/MD`를 심사 환경과 전체 제출 크기 기준으로 비교한다.
- atlas palette, 중복 tile, map RLE, 문자열 table을 재검사한다.
- 오디오 sample rate, channel, loop point, 압축 형식을 비교한다.
- UPX 적용 전후 크기, 실행 가능성, 백신 오탐을 별도 검사한다.
- 최종 빌드 후 모든 파일을 byte 단위로 합산하는 검사를 빌드 실패 조건으로 만든다.

### 완료 조건

```text
Homestead.exe + data.pak + representative.sav ≤ 1,474,560 bytes
```

- 제한을 가까스로 맞추지 않고 안전 여유를 남긴다.
- clean build 결과가 개발 PC와 검증 PC에서 동일하다.
- 압축 도구 없이도 원인별 크기 보고서를 만들 수 있다.

## 단계 18: 안정화와 제출 검증

### 기능 테스트

- 새 게임, 저장, 불러오기, 하루 변경, 수확, 완료 흐름
- inventory full, 맵 가장자리, 잘못된 입력, 빠른 메뉴 전환
- 창 최소화, resize, focus loss, Alt+Tab, Alt+F4
- 손상되거나 없는 pak/save 처리
- 장시간 반복 플레이와 날짜 누적

### 그래픽 테스트

- 여러 16:9 및 비정수 창 크기에서 letterbox 확인
- sprite bleeding, pixel 흔들림, layer/Y 정렬 확인
- D3D11 debug layer 오류와 live object 확인
- Direct3D 11.0 지원 Windows 10 검증 PC에서 실행

### 제출 테스트

- 깨끗한 Windows 환경 또는 심사 환경과 가까운 VM에서 실행한다.
- 필요한 Visual C++ Runtime이 실제로 존재하는지 확인한다.
- 누락된 DLL이나 개발 PC 절대 경로 의존성이 없는지 확인한다.
- 제출물에는 EXE와 pak 및 규정상 필요한 파일만 넣는다.
- 최종 파일 합계를 다시 계산한다.
- 구매 에셋과 음악의 재배포 라이선스를 확인한다.

### 완료 조건

- 별도 개발 도구나 원본 에셋 없이 실행된다.
- 첫 실행부터 완료 화면까지 치명적 오류가 없다.
- 저장 파일을 포함한 전체 크기가 제한 이하다.
- 최종 제출물의 hash와 크기를 기록한다.

## 3. 첫 번째 구현 마일스톤

가장 먼저 달성할 마일스톤은 다음 한 문장으로 정의한다.

> Win32 창을 생성하고 Direct3D 11로 320×180 논리 화면을 clear한 뒤, 창 크기에 맞춰 정수 배율 letterbox로 표시한다.

이 마일스톤은 단계 0~3에 해당한다. 여기까지 완료하기 전에는 타일맵, 인벤토리, 농사 시스템을 구현하지 않는다.

첫 마일스톤의 작업 순서는 다음과 같다.

1. 새 `Main`, `Application`, `Window`, `Clock` 작성
2. D3D11 device/context/swap chain 생성
3. back-buffer clear와 `Present`
4. resize와 최소화 처리
5. 320×180 scene render target 생성
6. scene target을 back buffer에 확대
7. letterbox와 마우스 좌표 변환 확인
8. Debug 검증 후 Release 크기 기록

## 4. 의존 관계

```text
프로젝트 기준선
→ Window/Application
→ D3D11 초기화
→ 논리 화면
→ SpriteBatch
→ Input/fixed update
→ AssetPacker
→ Camera/TileMap
→ Player/Collision
→ Interaction/Tools
→ Inventory
→ Farming
→ Time/Goal
→ Save
→ UI/Audio
→ Content
→ Size optimization
→ Submission validation
```

앞 단계의 public interface가 불편하다는 사실이 다음 단계에서 드러나면 작은 범위에서 수정한다. 그러나 크기 제한 때문에 당장 사용하지 않는 확장 지점이나 추상화 계층을 미리 추가하지 않는다.

## 5. 단계별 기록 양식

각 단계를 마칠 때 다음 내용을 commit 또는 개발 기록에 남긴다.

```text
단계:
완료한 기능:
검증한 항목:
Debug 결과:
Release EXE bytes:
data.pak bytes:
representative save bytes:
전체 bytes:
남은 용량:
알려진 문제:
다음 단계:
```

이 기록을 유지하면 어느 기능이나 에셋이 용량을 크게 증가시켰는지 마지막 단계가 아니라 추가된 시점에 발견할 수 있다.
