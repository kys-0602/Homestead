# Homestead CMake 빌드 가이드

이 문서는 Windows에서 Visual Studio IDE 없이 다음 도구를 이용해 Homestead를 빌드하는 방법을 설명한다.

```text
Visual Studio Build Tools + CMake + VS Code
```

## 1. Visual Studio Build Tools 설치

Visual Studio Build Tools Installer에서 **C++를 사용한 데스크톱 개발** 워크로드를 선택한다.

다음 개별 구성 요소가 포함되어 있는지 확인한다.

- MSVC C++ x64/x86 빌드 도구
- Windows 10 SDK 또는 Windows 11 SDK
- Windows용 C++ CMake 도구
- Ninja

Visual Studio IDE 전체를 설치할 필요는 없다.

이 프로젝트는 Direct3D 11 라이브러리인 `d3d11`, `dxgi`, `d3dcompiler`를 사용한다. 이 라이브러리들은 Windows SDK에 포함되어 있으므로 예전 DirectX SDK를 별도로 설치하지 않는다.

## 2. VS Code 설치 및 확장 설치

VS Code에 다음 Microsoft 확장을 설치한다.

- **C/C++** (`ms-vscode.cpptools`)
- **CMake Tools** (`ms-vscode.cmake-tools`)

확장 화면은 `Ctrl+Shift+X`로 열 수 있다.

## 3. 프로젝트 열기

최초 한 번은 VS Code에서 다음 폴더를 연다.

```text
C:\Users\A\Documents\GitHub\Homestead
```

메뉴에서 다음 순서로 열 수 있다.

```text
File → Open Folder → Homestead 선택
```

이후에는 VS Code의 `File → Open Recent`에서 Homestead를 선택하면 된다. 매번 `cd`와 `code .`을 실행할 필요는 없다.

VS Code에서 새 터미널을 열면 기본 작업 경로는 일반적으로 프로젝트 루트이다.

```powershell
PS C:\Users\A\Documents\GitHub\Homestead>
```

## 4. CMake 프리셋 확인

프로젝트에는 다음 프리셋이 정의되어 있다.

- `debug`: 개발 및 디버깅용
- `release`: 배포 및 성능 확인용

터미널에서 프리셋 목록을 확인한다.

```powershell
cmake --list-presets
```

정상적인 출력은 다음과 비슷하다.

```text
Available configure presets:

  "debug"   - Windows x64 Debug
  "release" - Windows x64 Release
```

## 5. 최초 Debug 빌드

### 5.1 CMake 구성

먼저 CMake가 Visual Studio 프로젝트와 빌드 파일을 생성하게 한다.

```powershell
cmake --preset debug
```

이 명령은 `build/debug` 디렉터리를 구성할 뿐, 실행 파일을 만들지는 않는다.

### 5.2 소스 빌드

구성이 끝난 다음 실제 컴파일과 링크를 실행한다.

```powershell
cmake --build --preset debug
```

빌드가 성공하면 실행 파일은 다음 위치에 생성된다.

```text
build/debug/Debug/Homestead.exe
```

### 5.3 프로그램 실행

PowerShell에서 실행한다.

```powershell
.\build\debug\Debug\Homestead.exe
```

전체 순서는 다음 세 명령으로 요약할 수 있다.

```powershell
cmake --preset debug
cmake --build --preset debug
.\build\debug\Debug\Homestead.exe
```

## 6. 평소 개발 중 빌드

`CMakeLists.txt`나 프리셋이 이미 구성되어 있다면 소스 수정 후에는 보통 빌드 명령만 실행하면 된다.

```powershell
cmake --build --preset debug
```

CMake가 필요한 경우 자동으로 구성을 다시 확인한다.

빌드 후 실행한다.

```powershell
.\build\debug\Debug\Homestead.exe
```

## 7. Release 빌드

최적화된 Release 실행 파일은 다음 순서로 만든다.

```powershell
cmake --preset release
cmake --build --preset release
```

실행 파일 위치는 다음과 같다.

```text
build/release/Release/Homestead.exe
```

실행 명령은 다음과 같다.

```powershell
.\build\release\Release\Homestead.exe
```

### 7.1 최종 제출 폴더 생성

Release 구성 후 제출 전용 target을 빌드한다.

```powershell
cmake --build --preset release --target HomesteadSubmission
```

`build/release/submission`에는 `Homestead.exe`, `data.pak`,
`representative.sav` 세 파일만 생성된다. 각 파일의 byte 크기와 SHA-256,
전체 합계는 제출 폴더 밖의 `build/release/submission-manifest.txt`에 기록된다.
이 target은 Debug 구성에서 실패하며 전체 합계가 1,474,560 bytes를 넘으면 실패한다.

## 8. VS Code 명령으로 빌드

터미널 명령 대신 CMake Tools 확장을 사용할 수도 있다.

1. `Ctrl+Shift+P`를 누른다.
2. `CMake: Select Configure Preset`을 실행한다.
3. `Windows x64 Debug`를 선택한다.
4. `CMake: Configure`를 실행한다.
5. `CMake: Build`를 실행한다.
6. `CMake: Run Without Debugging`으로 실행한다.

프리셋 선택과 최초 구성이 끝난 후에는 VS Code 하단 상태 표시줄의 Build 버튼을 사용해도 된다.

## 9. `cmake` 명령을 찾지 못하는 경우

다음 오류가 나타날 수 있다.

```text
cmake: The term 'cmake' is not recognized ...
```

이는 CMake가 없다는 뜻일 수도 있지만, Build Tools에 포함된 CMake가 일반 PowerShell의 `PATH`에 등록되지 않은 경우가 많다.

해결 방법은 다음 중 하나를 선택한다.

### 방법 A: VS Code CMake Tools 사용

VS Code에서 `CMake: Configure`와 `CMake: Build`를 실행한다. CMake Tools가 설치된 Visual Studio Build Tools와 CMake를 검색한다.

### 방법 B: Developer PowerShell 사용

시작 메뉴에서 **Developer PowerShell for VS**를 열고 프로젝트를 실행한다.

```powershell
cd C:\Users\A\Documents\GitHub\Homestead
cmake --preset debug
cmake --build --preset debug
```

### 방법 C: CMake를 PATH에 추가

현재 설치 환경에서 Build Tools에 포함된 CMake 경로는 다음과 같다.

```text
C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin
```

이 경로를 Windows 사용자 `Path` 환경 변수에 추가하고 VS Code를 완전히 종료한 뒤 다시 실행한다.

## 10. 깨끗하게 다시 구성하기

빌드 설정이 꼬였을 때는 VS Code 명령 팔레트에서 다음 명령을 실행하는 방법이 가장 안전하다.

```text
CMake: Delete Cache and Reconfigure
```

이후 다시 빌드한다.

```powershell
cmake --build --preset debug
```

`build` 디렉터리는 생성 파일만 포함하며 Git에는 커밋하지 않는다.

## 11. 자주 혼동하는 명령

| 명령 | 역할 | EXE 생성 여부 |
|---|---|---:|
| `cmake --preset debug` | Debug 빌드 환경 구성 | 아니요 |
| `cmake --build --preset debug` | 컴파일 및 링크 | 예 |
| `.\build\debug\Debug\Homestead.exe` | 완성된 프로그램 실행 | 해당 없음 |

빌드 오류가 발생하면 터미널에서 가장 먼저 나타난 `error` 줄을 확인한다. 경고인 `warning`은 빌드를 중단시키지 않지만, `error`가 하나라도 있으면 `Homestead.exe`가 생성되지 않을 수 있다.
