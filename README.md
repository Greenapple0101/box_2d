# box_2d

메커톤 예선용 **미니 서바이벌 프로토타입**만 담은 저장소입니다. (raylib 전체 예제 폴더는 포함하지 않습니다.)

게임 코드: [`mackathon_qualifier/`](mackathon_qualifier/)

## 빌드 (macOS)

저장소 루트에서:

```bash
./premake5.osx gmake2
make -C mackathon_qualifier config=release_x64
```

실행 파일:

`mackathon_qualifier/_bin/Release/mackathon_qualifier`

## 빌드 (Linux)

```bash
./premake5 gmake2
make -C mackathon_qualifier config=release_x64
```

## 빌드 (Windows)

`premake5.exe`로 Visual Studio / Makefile 생성 후 `mackathon_qualifier` 프로젝트를 빌드하세요.

## 라이선스

루트의 `LICENSE`는 원본 예제 레포에서 가져온 항목입니다. raylib은 [Zlib](https://github.com/raysan5/raylib/blob/master/LICENSE) 라이선스를 따릅니다.
