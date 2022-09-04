
# WarfLang 
## Inspiration for the name
A legally distinct but phoneitically similar name to a sci-fi space warrior. 

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
      -DCMAKE_CXX_FLAGS="-fprofile-instr-generate -fcoverage-mapping"
ninja -C./build
LLVM_PROFILE_FILE="Warflang.profraw" ./build/test/WarfLang_TEST
$(brew --prefix llvm)/bin/llvm-profdata merge -sparse WarfLang.profraw -o WarfLang.profdata
$(brew --prefix llvm)/bin/llvm-cov show ./test/WarfLang_TEST -format="html" -instr-profile=WarfLang.profdata -output-dir=out/report
```

### Run Clang Tidy
After running cmake, the build dir will have a `compile_commands.json` file. Thats all you need to run `run-clang-tidy.py`.
```bash
run-clang-tidy.py -p build/ -header-filter='.*' -fix -format
```
## Docker Build & Run
- build: `docker build -t warflang:latest .`
- run: `docker run  --name test_vm -it warflang:latest`