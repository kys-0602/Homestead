# Homestead 개발 진행 기록

이 문서는 세션 간 작업 인수인계를 위한 누적 기록이다. 작업 순서와 공식 완료 조건은 `IMPLEMENTATION_ROADMAP.md`를 기준으로 하며, 이 문서에는 실제 구현 상태, 검증 결과, 크기 변화와 남은 확인 사항을 기록한다.

## 현재 상태

- 마지막 갱신: 2026-08-23
- 현재 완료 범위: 단계 0~16 구현
- 다음 작업: 단계 16 후속 맵 콘텐츠 수동 검증
- 제출 크기 상한: `1,474,560 bytes`
- 현재 Release EXE: `87,552 bytes`
- 현재 `data.pak`: `354,648 bytes`
- 현재 대표 save: `68 bytes`
- 현재 합계: `442,268 bytes`
- 남은 공간: `1,032,292 bytes`

## 단계별 기록

| 단계 | 상태 | main 커밋 | Release EXE | 이전 단계 대비 |
|---|---|---|---:|---:|
| 0. 프로젝트 기준선 | 완료 | `871ec1a` (#3) | 10,240 bytes | 기준선 |
| 1. Win32 애플리케이션 골격 | 완료 | `0d63fb0` (#4) | 12,800 bytes | +2,560 bytes |
| 2. Direct3D 11 초기화 | 구현 완료, Debug Layer 검증 대기 | `21e19d4` (#5) | 13,824 bytes | +1,024 bytes |
| 3. 고정 논리 화면과 letterbox | 구현 완료, Debug Layer 검증 대기 | `7559bae` (#6) | 15,872 bytes | +2,048 bytes |
| 4. 최소 SpriteBatch | 구현 완료, Debug Layer 검증 대기 | `11d8f09` (#8) | 22,528 bytes | +6,656 bytes |
| 5. 입력과 고정 업데이트 루프 | 완료 | `48394d4` (#9) | 26,112 bytes | +3,584 bytes |
| 6. 에셋 패커의 최소 버전 | 구현 완료, 시각/D3D 검증 대기 | `bd51ebe` (#10) | 33,280 bytes | +7,168 bytes |
| 7. 카메라와 타일맵 | 구현 및 시각 검증 완료, D3D Debug Layer 검증 대기 | `b560ff3` (#12) | 36,864 bytes | +3,584 bytes |
| 8. 플레이어와 충돌 | 구현 및 gameplay 시각 검증 완료, D3D 검증 대기 | `12b31af` (#14) | 40,960 bytes | +4,096 bytes |
| 9. 상호작용과 도구 | 구현 및 gameplay 시각 검증 완료, D3D 검증 대기 | 병합 대기 | 44,544 bytes | +3,584 bytes |
| 10. 인벤토리와 아이템 | 구현 및 hotbar GUI 검증 완료, 세부 GUI 검증 대기 | `799a4d7` (#16) | 46,592 bytes | +2,048 bytes |
| 11. 농사 핵심 루프 | 구현 완료, gameplay GUI 검증 대기 | `0db6c8e` (#17) | 49,152 bytes | +2,560 bytes |
| 12. 시간, 하루 종료와 최소 목표 | 구현 완료, gameplay GUI 검증 대기 | `b02b256` (#18) | 51,200 bytes | +2,048 bytes |
| 13. 저장과 불러오기 | 구현 완료, 장시간 gameplay 검증 대기 | `74c1a13` (#19) | 62,976 bytes | +11,776 bytes |
| 14. UI와 설정 정리 | 구현 완료, GUI 검증 대기 | `17e48e6` (#20) | 69,120 bytes | +6,144 bytes |
| 15. 오디오 | 구현 및 실제 청취 검증 완료 | `ceb42d5` | 76,288 bytes | +7,168 bytes |
| 16. 콘텐츠와 게임 완결 | 구현 완료, 전체 수동 완주 검증 대기 | `c411174` (#21) | 78,848 bytes | +2,560 bytes |
| 16 후속. 농장·집 맵 콘텐츠 | 구현 완료, 수동 시각·저장 검증 대기 | `2dfec15`, 후속 UI 미커밋 | 87,552 bytes | +8,704 bytes |

### 단계 0: 프로젝트 기준선

구현:

- Windows 10 이상, x64, C++17 기준 CMake 구성
- Debug/Release 옵션 분리
- `/W4`, UTF-8, `WIN32_LEAN_AND_MEAN`, `NOMINMAX`
- Release `/O1`, `/GL`, `/LTCG`, `/OPT:REF`, `/OPT:ICF`
- Release EXE byte 크기 출력
- 기본 디렉터리 구조와 최소 Win32 진입점

검증:

- Debug/Release configure 및 build 성공
- 최소 실행 파일 종료 코드 `0`
- Release 크기 로그 출력 확인

### 단계 1: Win32 애플리케이션 골격

구현:

- `Application`이 초기화, 메시지 루프, 종료 순서를 소유
- Win32 창 클래스 등록, 1280×720 초기 클라이언트 영역과 메시지 처리
- `WM_CLOSE`, `WM_DESTROY`, `WM_NCDESTROY`, `WM_SIZE`, focus 변경 처리
- `QueryPerformanceCounter` 기반 `Clock`

검증:

- Debug/Release `/W4` build 성공
- 리사이즈, 최소화, 복원, focus, 창 닫기와 시스템 닫기 경로 확인
- 종료 코드 `0`, 잔류 프로세스 없음

### 단계 2: Direct3D 11 초기화

구현:

- D3D11 device, immediate context, DXGI swap chain과 back-buffer RTV
- 하드웨어 장치 우선, 실패 시 WARP fallback
- clear/present와 swap-chain resize
- 최소화 중 렌더링 중단
- Debug에서 validation layer 요청 및 live-object 보고
- Graphics Tools가 없을 때만 일반 D3D11 장치로 fallback
- 사용하지 않는 `d3dcompiler` 링크 제거

검증:

- Debug/Release `/W4` build 성공
- 초기, 리사이즈, 최소화 복원 후 동일한 clear 색 확인
- 정상 종료와 잔류 프로세스 없음
- Release에 Debug 전용 진단 문자열 및 `d3dcompiler` 의존 없음

남은 확인:

- 이 PC에 Windows 선택적 기능 `Graphics Tools`를 설치한 뒤 D3D11 validation message 확인
- Debug 종료 시 예상하지 않은 live-object 경고가 없는지 확인

### 단계 3: 고정 논리 화면과 letterbox

구현:

- 320×180 RGBA scene render target
- 최대 정수 배율, 중앙 letterbox와 point sampling
- 320×180보다 작은 창에서는 16:9 aspect-fit 축소
- client 좌표를 논리 좌표로 변환하고 letterbox 바깥 입력 거부
- Windows SDK `fxc`를 이용한 빌드 시 셰이더 컴파일 및 bytecode 내장
- presentation vertex/pixel shader bytecode 합계 `804 bytes`
- letterbox 계산과 좌표 변환 단위 테스트
- 축소 정책을 `ARCHITECTURE.md`에 기록

검증:

- Debug/Release build 및 단위 테스트 성공
- 1280×720 전체 scene 출력
- 984×661 정수 확대와 letterbox 출력
- 284×161 축소 정책과 letterbox 출력
- 최소화·복원 후 출력 유지 및 정상 종료
- Release에 HLSL 경로, 런타임 컴파일 코드와 `d3dcompiler` 의존 없음

남은 확인:

- Graphics Tools 설치 후 잘못된 resource binding이나 live-object 경고가 없는지 확인
- 구매 pixel-art atlas가 추가되는 이후 단계에서 실제 에셋의 픽셀 경계를 다시 확인

### 단계 4: 최소 SpriteBatch

구현:

- position, UV, color를 받는 sprite vertex/pixel shader의 빌드 시 오프라인 컴파일
- 256-sprite 동적 vertex buffer와 고정 index buffer
- point sampler와 straight-alpha blend state
- 고정 용량 `SpriteCommand`, `RenderQueue`, `SpriteBatch`
- texture/material, layer/depth, Actor Y와 제출 순서 기반 정렬
- 동일 texture batch 제출과 texture 변경 또는 256개 초과 시 flush
- 런타임 파일 없이 생성하는 16×16 RGBA 테스트 texture
- 투명도, tint와 겹침 순서를 확인할 여러 테스트 sprite
- presentation 포함 전체 shader bytecode `1,688 bytes`; sprite shader만 `884 bytes`

검증:

- Debug/Release `/W4` build 성공
- presentation 및 RenderQueue 단위 테스트 Debug/Release 통과
- 257 sprite가 2 batch로 분할되고 queue 용량 초과가 안전하게 거부되는지 확인
- 실제 GPU draw call 수가 batch 계산 결과와 일치하는지 런타임 확인
- 1280×720 캡처에서 UV, 흰색/노란색 tint, 반투명 red/blue 겹침과 Y 순서 확인
- 16×16 texture가 4배 정수 확대에서 선명한 픽셀 블록으로 표시되는지 확인
- 정상 종료 코드 `0` 및 Release에 HLSL 경로와 `d3dcompiler` 의존이 없음을 확인

남은 확인:

- Graphics Tools 설치 후 SpriteBatch resource binding과 종료 시 live-object 경고 확인
- 여러 실제 texture를 로드하는 단계에서 texture 전환별 GPU binding을 확장 및 검증

### 단계 5: 입력과 고정 업데이트 루프

구현:

- keyboard/mouse 물리 입력과 gameplay `Action` 분리
- held/pressed/released 및 fixed update용 pending press 상태
- WASD/방향키 이동, E/Space/오른쪽 클릭 상호작용, F/왼쪽 클릭 도구, Escape 메뉴 매핑
- 여러 물리 키가 같은 Action에 묶일 때 올바른 held/released 처리
- focus 상실 시 held와 pending press 해제
- client mouse를 320×180 논리 좌표로 변환하고 letterbox 바깥 위치 무효화
- 60Hz fixed update, frame delta 0.25초 clamp, 프레임당 최대 5회 update
- update 상한 초과 누적 시간 폐기로 spiral-of-death 방지
- 테스트 sprite의 fixed-speed 이동, 단발 tint 전환과 mouse 위치 이동
- 4단계 병합 과정에서 누락된 `SpriteBatch::Render()` 종료 중괄호 수정

검증:

- Debug/Release `/W4` build 성공
- presentation, RenderQueue, Input, FixedStep 테스트가 Debug/Release에서 모두 통과
- 30/60/144Hz 시뮬레이션이 2초 동안 동일하게 120 fixed update를 생성
- 긴 delta가 최대 5 update로 제한되고 reset/보간 alpha 범위를 지키는지 확인
- 실제 GUI에서 1초간 이동이 약 61 logical pixels로 측정됨
- focus 상실 뒤 held 이동이 고착되지 않고 위치가 유지되는지 확인
- 단발 Interact가 tint를 한 번 전환하는지 확인
- client `(800,400)` 클릭과 변환 후 sprite 중심 `(800,400)`이 일치하는지 확인
- 정상 종료 코드 `0` 및 잔류 프로세스 없음

남은 확인:

- 실제 게임 상태가 추가될 때 `InputSnapshot` 또는 동등한 읽기 전용 경계로 Input과 Game 의존성을 분리
- Graphics Tools 설치 후 기존 D3D11 validation/live-object 검증 수행

### 단계 6: 에셋 패커의 최소 버전

구현:

- 구매 원본을 Git에서 제외하고 추적 가능한 source/sprite/player-frame manifest 작성
- 29개 허용 source, 30개 정적 sprite 영역, 57개 플레이어 프레임 검증
- WIC 기반 개발 전용 PNG 디코딩과 플레이어/의상/도구 레이어 합성
- 투명 경계 트리밍, 동일 픽셀 중복 제거와 결정적 atlas 배치
- 87개 논리 sprite를 73개 고유 영역과 79색 palette atlas로 변환
- 안정적인 32-bit `AssetId`와 패커 충돌 검사
- version 1 `data.pak` header/index/payload, 전체 및 항목별 checksum
- 런타임 `AssetStore`의 pak/atlas/sprite table 검증과 D3D11 atlas texture 생성
- CMake 빌드에서 pak 생성 후 실행 파일 옆 자동 복사

검증:

- Debug/Release `/W4` build 성공
- presentation, RenderQueue, Input, FixedStep, manifest, AssetStore 테스트 6개 통과
- bad magic, unsupported version, truncated file, checksum 변조, invalid/overlapping offset,
  excessive entry count, invalid stable ID와 palette index 범위 초과 거부 확인
- palette atlas의 RGBA 무손실 복원 확인
- 반복 생성한 atlas metadata/pixels와 `data.pak` SHA-256 일치
- Debug 앱이 실제 pak과 atlas texture를 로드한 상태로 2초 이상 실행됨
- Release `Homestead.exe`: `33,280 bytes` (단계 5 대비 `+7,168 bytes`)
- `data.pak`: `83,560 bytes`
- 대표 save: 없음
- 제출 합계: `116,840 bytes`; 상한까지 `1,357,720 bytes`

선별 콘텐츠:

- 기본 농부 idle/walk/hoe/watering, 괭이와 물뿌리개
- 잔디, 길, 건조/젖은 농지, 당근 성장/수확
- 농가, 큰 참나무, 최소 울타리/꽃/허수아비/표지판
- 5x7 bitmap font와 pointer frame
- 큰 UI sheet는 source allowlist에만 두고 실제 영역 선택은 단계 14로 연기

남은 확인:

- 실제 표시 창에서 atlas sprite의 색, 투명도, pixel 경계와 point sampling 수동 확인
- Graphics Tools 설치 후 D3D11 resource binding 및 종료 시 live-object 경고 확인

### 단계 7: 카메라와 타일맵

구현:

- 논리 픽셀에 스냅되는 `Camera2D`의 world/screen 변환과 visible bounds
- 16×16 tile, 16×16 chunk와 최대 128×128 tile 상한을 가진 `TileMap`
- ground/object `uint16_t` ID와 collision/gameplay `uint8_t` flag
- `HSTM` version 1 map header와 6-byte 비압축 tile record
- 개발용 32×24 farm map source와 결정적 map payload 생성
- pak type 3 `map/farm` 항목과 런타임 payload 전달
- visible bounds와 여유 한 줄만 순회하는 `TileMapRenderer`
- ground와 object layer 분리, 큰 object의 tile-bottom anchor
- WASD/방향키로 독립 이동하는 단계 7 테스트 카메라

검증:

- Debug/Release `/W4` build 성공
- 기존 테스트와 Camera, TileMap, MapCompiler 테스트를 합한 9개가 Debug/Release에서 모두 통과
- world/screen 왕복, fractional camera 스냅, 음수·맵 경계 좌표 검증
- 320×180 화면과 한 tile 여유에서 최악의 2-layer 제출이 616 command로 1,024 한도 이내임을 검증
- bad magic/version, truncated payload, 0/상한 초과 크기, 잘못된 tile 크기·ID·flag 거부 확인
- 동일 입력으로 두 번 생성한 `data.pak` SHA-256 `8FC2F4E90B4EA50110B96E9144F4CEC74492BFE5832EA85877C8520AF541A634` 일치
- Debug 앱이 실제 map 포함 pak을 로드한 상태로 2초 이상 실행되고 종료됨
- 1280×720 초기/이동 캡처에서 pixel 경계, path, ground/object layer와 큰 object의 tile-bottom anchor 확인
- 직접 키 입력으로 오른쪽/아래 및 맵 왼쪽·위 경계 밖까지 이동해 컬링 결과와 정상 실행 확인
- 984×661 리사이즈 및 최소화·복원 뒤 정수 확대, letterbox와 point sampling 유지 확인
- Release `Homestead.exe`: `36,864 bytes` (단계 6 대비 `+3,584 bytes`)
- `data.pak`: `88,216 bytes` (단계 6 대비 `+4,656 bytes`, map payload `4,632 bytes`)
- 대표 save: 없음
- 제출 합계: `125,080 bytes` (단계 6 대비 `+8,240 bytes`); 상한까지 `1,349,480 bytes`

남은 확인:

- Graphics Tools 설치 후 D3D11 resource binding 및 종료 시 live-object 경고 확인

### 단계 8: 플레이어와 충돌

구현:

- generation 검사를 포함한 16-slot 고정 용량 `EntityWorld`와 별도 `PlayerState`
- 플레이어의 현재/이전 발 위치, sprite ID, 이동 속도와 방향/animation 상태
- 입력 벡터 정규화와 60Hz 고정 update 기반 자유 이동
- 발 위치 기준 10×6 논리 pixel AABB와 맵 밖을 막힌 영역으로 취급하는 tile collision
- 나무 줄기, 가로/세로 울타리와 표지판의 실제 발판에 맞춘 object별 collision footprint
- X/Y축 분리 해결과 벽 미끄러짐
- 보간된 플레이어 위치를 추적하고 viewport를 맵 경계 안에 고정하는 camera
- object와 player를 같은 Actor layer에서 발 위치 Y로 정렬하는 render queue
- 64×64 player source의 실제 발 위치(Y=41) anchor와 좌측 이동 sprite 수평 반전

검증:

- Debug/Release `/W4` build 성공
- 기존 테스트와 PlayerMovement 테스트를 합한 10개가 Debug/Release에서 모두 통과
- 축 이동과 대각선 이동의 1초 이동 거리가 각각 60 logical pixel로 동일함을 검증
- 수직 벽을 통과하지 않으면서 Y축 이동이 계속되는 벽 미끄러짐 검증
- 나무의 수관이 아닌 줄기 footprint에서 충돌하는지 검증
- 이미 지나친 좁은 object collider가 플레이어를 반대쪽 면으로 되돌리지 않는지 회귀 검증
- fixed update 전후의 previous/current 위치 보존과 idle 전환 검증
- camera의 왼쪽/위, 오른쪽/아래 경계 제한과 viewport보다 작은 맵 중앙 고정 검증
- Debug 앱이 실제 pak과 player sprite를 로드한 상태로 3초간 조기 종료 없이 실행됨
- 사용자가 실제 표시 창에서 자유/대각선 이동, 좌측 sprite 반전과 object 충돌을 확인하고 수용함
- 나무·울타리 collision 위치와 순간이동 회귀 수정 후 gameplay 동작을 수동 확인함
- Release `Homestead.exe`: `40,960 bytes` (단계 7 대비 `+4,096 bytes`)
- `data.pak`: `88,216 bytes` (단계 7 대비 변경 없음)
- 대표 save: 없음
- 제출 합계: `129,176 bytes` (단계 7 대비 `+4,096 bytes`); 상한까지 `1,345,384 bytes`

남은 확인:

- Graphics Tools 설치 후 D3D11 resource binding 및 종료 시 live-object 경고 확인

### 단계 9: 상호작용과 도구

구현:

- 플레이어 방향 기준 앞쪽 tile과 mouse world 좌표 기준 tile 선택
- tile 중심까지 최대 32 logical pixel 작업 거리와 map 범위 검증
- 범위 안은 초록색, 범위 밖은 빨간색인 개발용 pointer overlay
- fixed update 전에 click이 해제되어도 mouse/keyboard 입력 출처를 보존하는 pending press
- `Interact`는 object 기록만, `UseTool`은 map 상태만 변경하도록 분리
- 인벤토리 전 임시 문맥형 도구 동작: grass는 hoe, tilled tile은 watering can 사용
- blocked/water/path/object tile 경작 거부와 tilled/watered tile flag
- 36 fixed-tick 도구 상태, 18번째 tick의 상태 변경과 6-frame hoe/watering animation
- 도구 사용 중 이동·상호작용·중복 도구 사용 차단과 mouse target 방향 전환
- dry/wet farmland sprite 전환으로 경작·물주기 결과 표시

검증:

- Debug/Release `/W4` build 성공
- 기존 테스트와 InteractionTool 테스트를 합한 11개가 Debug/Release에서 모두 통과
- 네 방향 앞쪽 tile, mouse tile, map 밖과 최대 작업 거리 경계 검증
- object 상호작용이 tile 상태를 변경하지 않고 범위 밖 상호작용을 거부하는지 검증
- blocked/path/object/range 밖 tile에서 도구 시작을 거부하는지 검증
- 도구 사용 중 이동과 중복 사용을 차단하는지 검증
- 도구 입력이 시작된 동일 fixed tick부터 이동을 차단하는 update 순서 확인
- impact 이전에는 tile이 바뀌지 않고 animation frame 3 시점에 till/water flag가 적용되는지 검증
- serialized map에서 `Watered`만 있고 `Tilled`가 없는 모순된 flag 조합 거부 확인
- mouse click이 fixed update 전에 release되어도 물리 입력 출처가 유지되는지 검증
- Debug 앱이 실제 pak과 overlay sprite를 로드한 상태로 3초간 조기 종료 없이 실행됨
- 사용자가 실제 표시 창에서 경작 후 물주기 동작과 dry/wet tile 전환을 확인함
- Release `Homestead.exe`: `44,544 bytes` (단계 8 대비 `+3,584 bytes`)
- `data.pak`: `88,216 bytes` (단계 8 대비 변경 없음)
- 대표 save: 없음
- 제출 합계: `132,760 bytes` (단계 8 대비 `+3,584 bytes`); 상한까지 `1,341,800 bytes`

남은 확인:

- 경작 tile 표현의 시각적 어색함은 이후 콘텐츠/표현 조정에서 재검토
- keyboard/mouse target overlay와 빠른 click 동작의 추가 수동 확인
- Graphics Tools 설치 후 D3D11 resource binding 및 종료 시 live-object 경고 확인

### 단계 10: 인벤토리와 아이템

구현:

- 씨앗, 수확물, 괭이, 물뿌리개를 위한 조밀한 `ItemId`와 불변 `ItemDefinition` 테이블
- 16칸 고정 용량 `Inventory`와 8칸 hotbar
- 최대 stack을 지키는 추가, 원자적인 제거, 동일 item 병합 이동과 slot 교환
- 초기 hotbar에 괭이, 물뿌리개, 당근 씨앗 12개, 당근 3개 지급
- 선택 slot의 실제 도구에 따라 경작과 물주기가 실행되도록 단계 9 임시 문맥형 도구 선택 교체
- 숫자키 1~8과 mouse click hotbar 선택
- Escape inventory overlay, 방향키 cursor, E/Space 또는 mouse click을 이용한 2단계 이동/교환
- inventory가 열린 동안 이동, 상호작용, 도구와 world simulation 차단
- 논리 화면 좌표 기반 hotbar/inventory mouse hit-test와 atlas item icon 표시

검증:

- Debug/Release `/W4` build 성공
- 기존 테스트와 Inventory 테스트를 합한 12개가 Debug/Release에서 모두 통과
- stack 최대치 분할, 여러 slot 제거, 부족한 수량 제거의 원자성, 병합 이동, 교환 검증
- 가득 찬 inventory에서 남은 item 수량을 정확히 반환해 item이 사라지지 않는지 검증
- hotbar와 overlay의 경계 mouse hit-test 및 닫힌 overlay 입력 거부 검증
- Debug 앱이 실제 pak과 inventory UI를 로드한 상태로 3초간 조기 종료 없이 실행됨
- 사용자가 숫자키와 mouse click으로 hotbar 선택이 바뀌는지 확인함
- 원본 tool icon sheet에서 물뿌리개 sprite 좌표를 낚싯대 영역 `x=112`에서 바로 왼쪽 물뿌리개 영역 `x=96`으로 수정
- 사용자가 수정된 물뿌리개 icon 표시와 선택 후 물주기 동작을 실제 창에서 확인함
- Release `Homestead.exe`: `46,592 bytes` (단계 9 대비 `+2,048 bytes`)
- `data.pak`: `88,208 bytes` (단계 9 대비 `-8 bytes`)
- 대표 save: 없음
- 제출 합계: `134,800 bytes` (단계 9 대비 `+2,040 bytes`); 상한까지 `1,339,760 bytes`

남은 확인:

- 실제 표시 창에서 inventory 열기/닫기, keyboard/mouse slot 이동·교환과 world 입력 차단 확인
- item stack count 문자 표시는 단계 14의 bitmap text renderer와 함께 추가
- Graphics Tools 설치 후 D3D11 resource binding 및 종료 시 live-object 경고 확인

### 단계 11: 농사 핵심 루프

구현:

- 당근의 seed/harvest item과 4개 성장 sprite를 연결하는 조밀한 `CropDefinition`
- 256개 고정 용량 `CropField`와 tile 좌표 기반 `CropInstance`
- 경작된 빈 tile에만 당근 씨앗을 심고 성공한 뒤에만 씨앗 하나 소비
- 동일 tile 중복 심기, 범위 밖, 미경작지, object tile과 pool 초과 심기 거부
- `DayChanged` 시점에 물을 준 작물만 한 stage 성장시키고 모든 tile의 당일 물 상태 초기화
- 최종 stage 작물만 상호작용으로 수확하고 당근을 inventory에 추가
- inventory가 가득 차면 수확물을 잃지 않고 작물을 유지
- 보이는 작물만 `GroundDecoration` layer에 성장 stage별 sprite로 제출하는 `CropRenderer`
- 3번 seed 선택 후 keyboard/mouse 도구 입력으로 심기, 상호작용 입력으로 수확 연결

검증:

- Debug/Release `/W4` build 성공
- 기존 테스트와 Farming 테스트를 합한 13개가 Debug/Release에서 모두 통과
- 성공한 심기만 seed를 소비하고 중복/잘못된 item/미경작 tile은 inventory를 변경하지 않는지 검증
- 물을 주지 않은 날에는 성장하지 않고, 물을 준 날마다 한 stage만 성장하는지 검증
- 최종 stage 이후 추가 성장 방지와 매일 `Watered` flag 초기화 검증
- 성숙 전 수확 거부, 정상 수확의 crop 제거와 당근 획득 검증
- 가득 찬 inventory에서 수확 실패 시 crop이 사라지지 않는지 검증
- Debug 앱이 실제 pak과 CropRenderer를 초기화한 상태로 3초간 조기 종료 없이 실행됨
- Release `Homestead.exe`: `49,152 bytes` (단계 10 대비 `+2,560 bytes`)
- `data.pak`: `88,208 bytes` (단계 10 대비 변경 없음)
- 대표 save: 없음
- 제출 합계: `137,360 bytes` (단계 10 대비 `+2,560 bytes`); 상한까지 `1,337,200 bytes`

남은 확인:

- 실제 표시 창에서 3번 seed의 keyboard/mouse 심기, 중복 심기 거부와 stage 0 sprite 확인
- 하루 종료 입력과 성장 stage 전환은 단계 12에서 기존 `OnDayChanged` 함수에 연결한 뒤 gameplay 검증
- 성숙 작물의 상호작용 수확과 inventory 획득을 단계 12의 하루 진행 흐름에서 검증
- Graphics Tools 설치 후 D3D11 resource binding 및 종료 시 live-object 경고 확인

### 단계 12: 시간, 하루 종료와 최소 목표

구현:

- day와 minute를 정수로 보관하고 오전 6시에 시작하는 `WorldClock`
- 60 fixed tick마다 게임 1분 진행, inventory와 완료 화면에서는 world 시간 일시정지
- `N` 명시적 입력으로 시작하는 60 tick 하루 종료 전환과 중앙 시점 단일 `DayChanged`
- 하루 변경 시 단계 11 `CropField::OnDayChanged`를 호출해 작물 성장과 물 상태 초기화
- 전환 중 world 입력을 차단하고 pending 입력을 폐기해 다음 날 지연 실행 방지
- 5x7 bitmap font 기반 day/time, 당근 수확 진행도와 초기 안내 HUD
- bitmap font 숫자 행을 실제 `y=21`에 맞추고 inventory item stack 2~99 수량 표시
- uppercase와 숫자의 실제 5-pixel glyph bounds를 사용해 HUD와 stack count baseline 정렬
- `GROW 3 CARROTS`, `N ENDS DAY` 안내와 당근 3개 수확 목표
- 목표 달성 시 반투명 완료 화면과 `GOAL COMPLETE` 표시, world simulation 정지
- 완료된 저장을 다시 불러올 때 완료 화면을 반복 표시하지 않고, 목표 최초 달성 순간에만 표시

검증:

- Debug/Release `/W4` build 성공
- 기존 테스트와 WorldClock 테스트를 합한 14개가 Debug/Release에서 모두 통과
- 60 tick당 1분 진행, 하루 종료 중 중복 요청 거부와 날짜 변경 1회 검증
- 하루 변경 후 오전 6시 초기화, fade 최대 alpha와 전환 종료 검증
- reset 후 day/minute/transition 초기 상태 복원 검증
- `N` physical key의 `EndDay` action 변환과 단발 소비 검증
- Debug 앱이 실제 pak, bitmap HUD와 WorldClock을 초기화한 상태로 3초간 조기 종료 없이 실행됨
- Release `Homestead.exe`: `51,200 bytes` (단계 11 대비 `+2,048 bytes`)
- `data.pak`: `88,208 bytes` (단계 11 대비 변경 없음)
- 대표 save: 없음
- 제출 합계: `139,408 bytes` (단계 11 대비 `+2,048 bytes`); 상한까지 `1,335,152 bytes`

남은 확인:

- 실제 표시 창에서 bitmap HUD 글자, 안내 위치와 hotbar 겹침 여부 확인
- `N` 하루 종료 fade, 입력 차단, 날짜 증가와 작물 성장 stage 전환 확인
- 당근 3개 수확 후 완료 화면과 world simulation 정지 확인
- Graphics Tools 설치 후 D3D11 resource binding 및 종료 시 live-object 경고 확인

### 단계 13: 저장과 불러오기

구현:

- `HSSV` magic, version, payload size와 FNV-1a checksum을 가진 version 1 binary save
- raw struct dump 없이 모든 little-endian field를 명시적으로 encode/decode
- 플레이어 위치를 1/256 logical pixel 정수로 저장
- day/minute, 16-slot inventory, selected hotbar, 수확 목표 진행도 저장
- 원본 map과 다른 `Tilled`/`Watered` tile delta와 활성 crop만 sparse 저장
- 전체 save 32KiB, tile delta 4,096개, crop 256개 상한
- 완전한 임시 snapshot 검증 후에만 runtime state에 적용하는 transactional load
- `%LOCALAPPDATA%/Homestead/representative.sav` primary, `.tmp`, `.bak` 경로
- temporary write, flush/close, 검증된 기존 primary backup 후 atomic replace
- 시작 시 primary가 손상되면 backup fallback, 둘 다 없거나 invalid면 안전한 새 게임 유지
- 하루 변경 직후와 정상 종료 시 자동 저장

검증:

- Debug/Release `/W4` build 성공
- 기존 테스트와 SaveCodec 테스트를 합한 15개가 Debug/Release에서 모두 통과
- save round-trip으로 player/time/inventory/tile/crop state 복원 검증
- bad magic/version, truncated file, checksum 변조와 32KiB 초과 file 거부 검증
- invalid item ID, stack count, tile flag, crop ID와 excessive delta count 거부 검증
- 격리된 임시 `LOCALAPPDATA`에서 정상 종료 save 생성 후 재실행 load 성공, 두 실행 종료 코드 `0`
- 실제 대표 초기 save: `66 bytes` (`32KiB` 목표 이하)
- Release `Homestead.exe`: `62,976 bytes` (단계 12 대비 `+11,776 bytes`)
- `data.pak`: `88,208 bytes` (단계 12 대비 변경 없음)
- 제출 합계: `151,250 bytes` (단계 12 대비 `+11,842 bytes`); 상한까지 `1,323,310 bytes`

남은 확인:

- 실제 gameplay에서 경작·심기·물주기·날짜 진행 후 종료/재실행 상태 복원 확인
- primary checksum 손상 시 직전 valid backup이 복원되는 file-level 수동 검증
- 장시간 play save와 최대 tile/crop 상태의 대표 save 크기 측정
- Graphics Tools 설치 후 D3D11 resource binding 및 종료 시 live-object 경고 확인

### 단계 14: UI와 설정 정리

구현:

- `Escape` 일시정지와 `I` 인벤토리를 별도 action으로 분리하고 두 화면에서 world simulation 정지
- pause 메뉴에 resume, inventory, window size, fullscreen, master/music/effect volume 항목 배치
- 방향키/WASD focus 이동과 좌우 값 변경, E/Space 활성화, mouse hover/click을 동일 항목 흐름에 연결
- 640×360, 960×540, 1280×720 창 크기와 borderless fullscreen 즉시 적용
- 0~10 master/music/effect 음량 설정 저장; 실제 audio bus 연결은 단계 15 범위
- `%LOCALAPPDATA%/Homestead/settings.cfg`에 12-byte `HSCF` version 1 설정 저장
- 설정 magic/version/range/checksum 검증과 temporary write, flush, atomic replace
- pause/HUD 문구를 `UI/Strings.hpp` 문자열 테이블로 집중
- 목표 완료 화면에서 E/Space 또는 `E CONTINUE` 영역 click으로 world에 복귀
- UI에 실제 사용하는 대문자 20개와 숫자 10개만 개별 5×5 글리프로 pak에 포함
- pause panel과 모든 hit rectangle을 320×180 logical safe area 안에 배치

검증:

- Debug/Release `/W4` build 성공
- 기존 테스트와 Settings/PauseUI 테스트를 합한 17개가 Debug/Release에서 모두 통과
- 설정 범위, pause 항목 경계, Escape/I action 분리 검증
- 격리된 임시 `LOCALAPPDATA`에서 settings 저장/재로드와 checksum 손상 거부 검증
- 격리된 실제 앱을 두 번 정상 종료해 `settings.cfg` 12 bytes, save/backup 각 66 bytes 생성 및 두 실행 종료 코드 `0` 확인
- font source를 135×35 전체 sheet에서 사용 글리프 30개로 축소해 `data.pak` `-1,980 bytes`
- Release `Homestead.exe`: `69,120 bytes` (단계 13 대비 `+6,144 bytes`)
- `data.pak`: `86,228 bytes` (단계 13 대비 `-1,980 bytes`)
- 대표 save: `66 bytes`
- 제출 합계: `155,414 bytes` (단계 13 대비 `+4,164 bytes`); 상한까지 `1,319,146 bytes`

남은 확인:

- 실제 표시 창에서 pause/inventory 전환, keyboard/mouse focus·click과 문구 정렬 확인
- 3개 창 크기의 integer scaling, 창↔fullscreen 반복 전환, Alt+Tab과 재시작 후 복원 확인
- 단계 15에서 master/music/effect 값을 실제 audio bus gain에 연결
- Graphics Tools 설치 후 D3D11 resource binding 및 종료 시 live-object 경고 확인

### 단계 15: 오디오

구현:

- WAV 원본을 8kHz mono 2-bit ADPCM `HSA2` payload로 변환하는 AssetPacker 경로
- 배경 음악 1개와 경작·물주기·심기·수확·UI 이동·UI 확인 효과음 6개를 pak type 4로 패킹
- pak 오디오 header/version/rate/count/range/checksum 검증과 stable `AssetId` 조회
- 시작 시 선택 오디오를 PCM으로 한 번 복원하는 작은 2-bit ADPCM decoder
- XAudio2 mastering voice, 반복 music voice와 4개 고정 effect voice pool
- 배경 음악에 묻히던 괭이질과 물주기 효과음에 각각 1.7배, 1.9배 재생 게인 적용
- 동일 효과음이 재생 중이면 중복 요청을 거부하는 제한
- 단계 14 master/music/effect 설정을 실제 voice gain에 연결하고 즉시 변경
- 전체 124.92초 `FunCrafting` 음악 반복 재생
- 경작 impact, 물주기 impact, 씨앗 심기, 수확과 inventory/pause UI 이벤트 연결
- 출력 장치 또는 XAudio2가 없을 때 무음으로 계속 실행하는 fallback
- 배경음 출처와 사용 조건을 `assets-src/AUDIO_LICENSES.md`에 기록

검증:

- Debug/Release `/W4` build 성공
- 기존 테스트와 AudioCodec/실제 pak AudioAsset 테스트를 합한 19개가 Debug/Release에서 모두 통과
- 잘못된 magic과 truncated 2-bit ADPCM payload 거부 검증
- 실제 생성된 10-entry pak에서 7개 audio stable ID와 payload 로드 검증
- 오디오 출력 장치가 없는 자동 실행 환경에서 3초간 조기 종료 없이 무음 fallback 동작
- pak audio payload 합계: `253,439 bytes` (330KiB 오디오 예산까지 `84,481 bytes`)
- Release `Homestead.exe`: `76,288 bytes` (단계 14 대비 `+7,168 bytes`)
- `data.pak`: `339,836 bytes` (단계 14 대비 `+253,608 bytes`)
- 대표 save: `66 bytes`
- 제출 합계: `416,190 bytes` (단계 14 대비 `+260,776 bytes`); 상한까지 `1,058,370 bytes`

남은 확인:

- 실제 오디오 출력 장치에서 음악 반복 경계와 master/music/effect 0~10 gain 확인
- 경작·물주기·심기·수확 효과음의 재생 시점과 UI 중복 제한 청취 확인
- 장시간 실행, pause/inventory 전환과 종료 후 XAudio2 voice 누수 여부 확인
- Graphics Tools 설치 후 D3D11 resource binding 및 종료 시 live-object 경고 확인

### 단계 16: 콘텐츠와 게임 완결

구현:

- 밀·당근·토마토 3종과 각각 2·3·4일의 성장 기간 구성
- 씨앗 구매 가격 4G·6G·9G와 수확물 판매 가격 7G·12G·20G의 소형 경제 테이블
- `M` 시장 UI에서 씨앗을 한 개씩 구매하고 선택한 작물의 수확물을 한 번에 판매
- 마우스가 항목 위에 있어도 방향키/WASD 입력이 시장 선택을 우선 변경하도록 focus 처리
- 새 게임은 괭이·물뿌리개와 20G로 시작하고, 씨앗 구매부터 농사 루프가 시작되도록 조정
- HUD에 현재 골드와 100G 완료 목표를 표시하고 판매 직후 목표 달성 화면 연결
- 시작 안내를 도구 슬롯, 시장, 사용·수확·하루 종료 키가 드러나도록 교체
- version 2 save에 골드와 작물별 물주기 성장 일수를 추가하고 version 1 당근 save migration 지원
- 최종 맵과 UI에서 사용하지 않는 장식·건물·포인터 sprite 11개와 전용 source allowlist 제거
- 구매·다작물 재배·중간 저장/불러오기·수확·판매와 100G 도달을 검증하는 테스트 추가
- 완료 화면에서 시장 입력을 차단해 완료 흐름 중 골드가 감소하지 않도록 수정
- 최대 골드 초과 판매를 아이템 손실 없이 거부하고 시장 표시 이름·가격을 경제 테이블과 단일화
- save 인코딩과 디코딩 양쪽에서 작물 ID, 물 준 일수와 파생 성장 단계의 일관성을 검증
- version 1의 폐기 필드 범위 검증을 유지하고 실제 legacy 당근 작물 migration 회귀 테스트 추가

검증:

- Debug/Release `/W4` build 성공
- 기존 테스트와 ContentFlow/Economy 테스트를 합한 21개가 Debug/Release에서 모두 통과
- 세 작물 구매·성장 기간·save round-trip·수확·판매와 재투자를 통한 100G 도달 검증
- version 2 save round-trip과 version 1 item/crop ID migration 검증
- 최종 manifest가 50개 source, 63개 static sprite와 57개 player frame만 허용
- pak에는 120개 논리 sprite, 106개 고유 영역, 62색 palette만 포함
- Release `Homestead.exe`: `78,848 bytes` (단계 15 대비 `+2,560 bytes`)
- `data.pak`: `312,720 bytes` (단계 15 대비 `-27,116 bytes`)
- 대표 save: `67 bytes`
- 제출 합계: `391,635 bytes` (단계 15 대비 `-24,555 bytes`); 상한까지 `1,082,925 bytes`

남은 확인:

- 기존 save를 별도로 보존한 뒤 새 게임의 시장·HUD·안내 정렬 확인
- 실제 입력으로 세 작물 구매, 경작, 심기, 성장, 수확, 판매와 100G 완료까지 전체 완주
- 진행 중 종료/재실행 후 이어서 완료할 수 있는지 수동 확인
- Graphics Tools 설치 후 D3D11 resource binding 및 종료 시 live-object 경고 확인

## 다음 작업: 단계 16 후속 맵 콘텐츠

구현:

- 농장 맵을 집 진입로, 중앙 길, 출입 가능한 경작 구역 중심의 32×24 배치로 교체
- 선별한 96×128 목조 farmhouse와 충돌 영역, 출입 지점 배치
- 나무 바닥·벽·침대·문만 사용하는 20×12 집 내부 맵 추가
- stable `MapId::Farm`/`MapId::House`와 두 `TileMap` 상주 방식으로 농장↔집 전환 구현
- 집 침대에 `E` 상호작용하면 하루 종료 전환을 시작하고 다음 날 작물 갱신 후 저장
- 큰 집·침대·문 sprite의 단일 anchor와 화면 판정 불일치를 보완하는 40px 근거리 keyboard 상호작용
- 맵 전환 시 출입문 방향으로 facing을 맞추고 같은 tick의 잔여 입력을 차단
- 침대 또는 `N` 하루 종료 저장 성공 시 다음 날 3초간 `GAME SAVED` HUD 안내 표시
- 기존 단일 dry/wet 경작지 tile 선택을 폐기하고 상·우·하·좌 연결 mask 16종 자동 타일링 적용
- 물 준 칸은 동일 연결 mask의 투명 wet variant를 dry farmland 위에 overlay
- 가로·세로 울타리 충돌을 선택 sprite의 실제 불투명 영역에 맞춰 투명 영역 충돌 제거
- 64×80 나무와 96×128 farmhouse가 화면 가장자리에서 조기 소멸하지 않도록 object 전용 확장 컬링
- save version 3에 현재 map ID와 해당 맵 위치를 기록하며 version 1/2는 농장으로 migration
- HUD 시간을 콜론 glyph를 포함한 `HH:MM` 형식으로 표시
- `UI_Frames.png`에서 필요한 기본·선택 슬롯 2개만 추출해 8칸 핫바와 인벤토리에 적용
- 기본 슬롯 프레임을 9-slice로 재사용해 인벤토리와 시장에 확장 가능한 패널 적용
- 시장 선택 행 강조, 작물별 씨앗·수확 아이콘과 패널 색상에 맞춘 문자 정렬 적용
- 인벤토리 슬롯 프레임을 26px로 축소하고 28px 간격으로 배치해 슬롯 사이 2px 여백 확보
- 시작 안내에 집 침대 저장 방법 추가

검증:

- Debug/Release `/W4` build 성공
- 기존 테스트와 실제 pak의 두 맵 콘텐츠 검증을 합한 22개가 Debug/Release에서 모두 통과
- pak에 `map/farm`과 `map/house`가 모두 존재하고 집·침대·문 및 양쪽 도착 지점 검증
- save version 3 집 위치 round-trip, invalid map ID 거부, version 1/2 농장 migration 검증
- 최종 manifest가 58개 source, 110개 static sprite와 57개 player frame만 허용
- pak에는 167개 논리 sprite, 152개 고유 영역, 81색 palette와 map payload 6,096 bytes 포함
- Release `Homestead.exe`: `87,552 bytes` (단계 16 대비 `+8,704 bytes`)
- `data.pak`: `354,648 bytes` (단계 16 대비 `+41,928 bytes`)
- 대표 save: `68 bytes` (단계 16 대비 `+1 byte`)
- 제출 합계: `442,268 bytes` (단계 16 대비 `+50,633 bytes`); 상한까지 `1,032,292 bytes`

남은 확인:

- 실제 창에서 farmhouse 외관, 충돌 영역과 농장 동선 확인
- 농장 문에서 `E`로 진입하고 실내 문에서 `E`로 나오는 전환 확인
- 침대에서 `E` 저장 후 종료·재실행했을 때 집 내부 위치와 농장 상태 복원 확인
- 수동 검증 뒤 단계 17 `크기 최적화` 진행

## 기록 갱신 규칙

각 로드맵 단계가 끝날 때 다음을 갱신한다.

1. 구현 상태와 main에 병합된 커밋 또는 PR 번호
2. 수행한 자동 및 GUI 검증
3. 남아 있는 수동 또는 환경 의존 검증
4. `Homestead.exe`, `data.pak`, 대표 save의 byte 크기
5. 이전 단계 대비 증감과 전체 상한까지 남은 byte
6. 다음 시작 단계와 범위

진행 표시만을 위해 `IMPLEMENTATION_ROADMAP.md`를 수정하지 않는다. 지속적인 설계 결정이 바뀌면 `ARCHITECTURE.md`도 함께 갱신한다.
