FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
    && apt install -y --no-install-recommends \
    build-essential cmake vim openssl curl ca-certificates \
    llvm clang valgrind libedit-dev

# Set the working directory to root (ie $HOME)
WORKDIR root

COPY test test
COPY fuzz fuzz
COPY CMakeLists.txt CMakeLists.txt
COPY scripts scripts
COPY cli-test-cases cli-test-cases
COPY src src

RUN cmake -B build -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-coverage"  -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ -DBUILD_FUZZER=true
RUN make -C./build
#
RUN ./build/test/WarfLang_TEST
#
#RUN ./build/fuzz/WarfLang_FUZZ
RUN valgrind --tool=memcheck --leak-check=full --track-origins=yes --log-file=valgrind.log ./build/test/WarfLang_TEST