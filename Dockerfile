FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
    && apt install -y --no-install-recommends \
    build-essential cmake vim openssl curl ca-certificates \
    llvm clang

# Set the working directory to root (ie $HOME)
WORKDIR root

COPY . .

RUN cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-coverage"  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
RUN make -C./build
#
RUN ./build/test/WarfLang_TEST
#
#RUN ./build/fuzz/WarfLang_FUZZ