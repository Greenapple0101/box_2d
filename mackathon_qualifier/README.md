# Mackathon Qualifier Prototype

3분 웨이브 서바이벌 + 자동 공격 + 강화 선택 데모 (raylib).

## 조작

- 이동: `WASD` 또는 방향키
- 강화 선택: `1`, `2`, `3`
- 재시작: `R`

## 빌드

저장소 **루트**에서:

```bash
./premake5.osx gmake2
make -C mackathon_qualifier config=release_x64
./mackathon_qualifier/_bin/Release/mackathon_qualifier
```

Linux는 `./premake5`, Windows는 `premake5.exe`를 사용하면 됩니다.
