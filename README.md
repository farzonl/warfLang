
# Build Instructions
```bash
cmake -B build
make
```
### Build with Ninja
```bash
cmake -G Ninja -B build
ninja -C./build
```
### Building on Windows
- Using Ninja
```powershell
cmake -GNinja -Bbuild
ninja -C./build
```
- Using MSBUILD
```powershell
cmake -B build
msbuild build\WarfLang.sln -t:Build -p:Configuration=Release
```

## Docker Build & Run
- build: `docker build -t warflang:latest .`
- run: `docker run  --name test_vm -it warflang:latest`