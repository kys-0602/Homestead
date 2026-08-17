# Homestead 개발 진행 기록

이 문서는 세션 간 작업 인수인계를 위한 누적 기록이다. 작업 순서와 공식 완료 조건은 `IMPLEMENTATION_ROADMAP.md`를 기준으로 하며, 이 문서에는 실제 구현 상태, 검증 결과, 크기 변화와 남은 확인 사항을 기록한다.

## 현재 상태

- 마지막 갱신: 2026-08-17
- 현재 완료 범위: 단계 0~4 구현
- 다음 작업: 단계 5 `입력과 고정 업데이트 루프`
- 제출 크기 상한: `1,474,560 bytes`
- 현재 Release EXE: `22,528 bytes`
- 현재 `data.pak`: 없음
- 현재 대표 save: 없음
- 현재 합계: `22,528 bytes`
- 남은 공간: `1,452,032 bytes`

## 단계별 기록

| 단계 | 상태 | main 커밋 | Release EXE | 이전 단계 대비 |
|---|---|---|---:|---:|
| 0. 프로젝트 기준선 | 완료 | `871ec1a` (#3) | 10,240 bytes | 기준선 |
| 1. Win32 애플리케이션 골격 | 완료 | `0d63fb0` (#4) | 12,800 bytes | +2,560 bytes |
| 2. Direct3D 11 초기화 | 구현 완료, Debug Layer 검증 대기 | `21e19d4` (#5) | 13,824 bytes | +1,024 bytes |
| 3. 고정 논리 화면과 letterbox | 구현 완료, Debug Layer 검증 대기 | `7559bae` (#6) | 15,872 bytes | +2,048 bytes |
| 4. 최소 SpriteBatch | 구현 완료, Debug Layer 검증 대기 | 병합 대기 | 22,528 bytes | +6,656 bytes |

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

## 다음 작업: 단계 5

`IMPLEMENTATION_ROADMAP.md`에 따라 다음 범위만 진행한다.

- 키보드와 마우스의 held/pressed/released 상태
- 물리 키에서 gameplay action으로의 변환
- focus 상실 시 held 상태 해제
- 60Hz fixed-update accumulator
- 긴 frame delta와 최대 fixed-update 반복 횟수 제한
- 한 render frame 안에서 pressed 입력의 중복 소비 방지
- 논리 화면 마우스 좌표 보고

단계 5에서도 Release EXE 크기와 전체 남은 byte를 기록한다.

## 기록 갱신 규칙

각 로드맵 단계가 끝날 때 다음을 갱신한다.

1. 구현 상태와 main에 병합된 커밋 또는 PR 번호
2. 수행한 자동 및 GUI 검증
3. 남아 있는 수동 또는 환경 의존 검증
4. `Homestead.exe`, `data.pak`, 대표 save의 byte 크기
5. 이전 단계 대비 증감과 전체 상한까지 남은 byte
6. 다음 시작 단계와 범위

진행 표시만을 위해 `IMPLEMENTATION_ROADMAP.md`를 수정하지 않는다. 지속적인 설계 결정이 바뀌면 `ARCHITECTURE.md`도 함께 갱신한다.
