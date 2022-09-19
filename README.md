
# WarfLang 
## Inspiration for the name
A legally distinct but phonetically similar name to a sci-fi space warrior.

## Open in Gitpod
[![Open in Gitpod](https://gitpod.io/button/open-in-gitpod.svg)](https://gitpod.io/github.com/farzonl/warflang)

## Build Instructions
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

## Setup Clang Tidy 
### On MacOS
```bash
ln -s "$(brew --prefix llvm)/share/clang/run-clang-tidy.py" "/usr/local/bin/run-clang-tidy.py"
ln -s "$(brew --prefix llvm)/bin/clang-apply-replacements" "/usr/local/bin/clang-apply-replacements"
ln -s "$(brew --prefix llvm)/share/clang/run-clang-tidy.py" "/usr/local/bin/run-clang-tidy.py"
```

## Building the fuzzer
### On MacOS
```bash
cmake -G Ninja -B build -DCMAKE_C_COMPILER=$(brew --prefix llvm)/bin/clang \
      -DCMAKE_CXX_COMPILER=$(brew --prefix llvm)/bin/clang++
```

## Building with Coverage
### on MacOS
```bash
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_FLAGS="-fprofile-instr-generate -fcoverage-mapping" \
      -DCMAKE_C_COMPILER=$(brew --prefix llvm)/bin/clang \
      -DCMAKE_CXX_COMPILER=$(brew --prefix llvm)/bin/clang++
ninja -C./build
LLVM_PROFILE_FILE="Warflang.profraw" ./build/test/WarfLang_TEST
$(brew --prefix llvm)/bin/llvm-profdata merge -sparse WarfLang.profraw -o WarfLang.profdata
$(brew --prefix llvm)/bin/llvm-cov show -output-dir=out/report -format=html -instr-profile=Warf.profdata -object=build/test/WarfLang_TEST build/src/cli/Warf src/
```

### Run Clang Tidy
After running cmake, the build dir will have a `compile_commands.json` file. Thats all you need to run `run-clang-tidy.py`.
```bash
run-clang-tidy.py -p build/ -header-filter='.*' -fix -format
```
## Docker Build & Run
- Build: `docker build -t warflang:latest .`
- Run: `docker run  --name test_vm -it warflang:latest`
- Print out container id: `inspect test_vm -f '{{.Id}}'`
- Get valgrind log: `docker cp <container id>:root/valgrind.log .`

## Docker Build & Run aarch64
- Build: `docker build -f Dockerfile.aarch64 -t warflang_arch64:latest .`
- Run: `docker run  --name arm_test_vm -it warflang_arch64:latest`

## Docker Build & Run mingw x86_64
- Build: `docker build -f Dockerfile.mingw-cross -t warflang_mingw:latest .`
- Run: `docker run  --name mingw_test_vm -it warflang_mingw:latest`

## Docker Build & Run osxcross x86_64
- Build: `docker build -f Dockerfile.osx-cross -t warflang_osx:latest .`
- Run: `docker run  --name osx_test_vm -it warflang_osx:latest`