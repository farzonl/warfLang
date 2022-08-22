#!/bin/bash
export CODECOV_TOKEN=""
find src test \( -name \*.h -o -name \*.cpp \) -exec gcov -style=file -i build/{} \;
bash <(curl -s https://codecov.io/bash)