# Homestead 개발 진행 기록

이 문서는 세션 간 작업 인수인계를 위한 누적 기록이다. 작업 순서와 공식 완료 조건은 `IMPLEMENTATION_ROADMAP.md`를 기준으로 하며, 이 문서에는 실제 구현 상태, 검증 결과, 크기 변화와 남은 확인 사항을 기록한다.

## 현재 상태

- 마지막 갱신: 2026-08-20
- 현재 완료 범위: 단계 0~9 구현
- 다음 작업: 단계 10 `인벤토리와 아이템`
- 제출 크기 상한: `1,474,560 bytes`
- 현재 Release EXE: `44,032 bytes`
- 현재 `data.pak`: `88,216 bytes`
- 현재 대표 save: 없음
- 현재 합계: `132,248 bytes`
- 남은 공간: `1,342,312 bytes`

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
| 9. 상호작용과 도구 | 구현 및 gameplay 시각 검증 완료, D3D 검증 대기 | 병합 대기 | 44,032 bytes | +3,072 bytes |

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
- impact 이전에는 tile이 바뀌지 않고 animation frame 3 시점에 till/water flag가 적용되는지 검증
- mouse click이 fixed update 전에 release되어도 물리 입력 출처가 유지되는지 검증
- Debug 앱이 실제 pak과 overlay sprite를 로드한 상태로 3초간 조기 종료 없이 실행됨
- 사용자가 실제 표시 창에서 경작 후 물주기 동작과 dry/wet tile 전환을 확인함
- Release `Homestead.exe`: `44,032 bytes` (단계 8 대비 `+3,072 bytes`)
- `data.pak`: `88,216 bytes` (단계 8 대비 변경 없음)
- 대표 save: 없음
- 제출 합계: `132,248 bytes` (단계 8 대비 `+3,072 bytes`); 상한까지 `1,342,312 bytes`

남은 확인:

- 경작 tile 표현의 시각적 어색함은 이후 콘텐츠/표현 조정에서 재검토
- keyboard/mouse target overlay와 빠른 click 동작의 추가 수동 확인
- Graphics Tools 설치 후 D3D11 resource binding 및 종료 시 live-object 경고 확인

## 다음 작업: 단계 10

`IMPLEMENTATION_ROADMAP.md`에 따라 `인벤토리와 아이템` 범위를 진행한다.

## 기록 갱신 규칙

각 로드맵 단계가 끝날 때 다음을 갱신한다.

1. 구현 상태와 main에 병합된 커밋 또는 PR 번호
2. 수행한 자동 및 GUI 검증
3. 남아 있는 수동 또는 환경 의존 검증
4. `Homestead.exe`, `data.pak`, 대표 save의 byte 크기
5. 이전 단계 대비 증감과 전체 상한까지 남은 byte
6. 다음 시작 단계와 범위

진행 표시만을 위해 `IMPLEMENTATION_ROADMAP.md`를 수정하지 않는다. 지속적인 설계 결정이 바뀌면 `ARCHITECTURE.md`도 함께 갱신한다.
