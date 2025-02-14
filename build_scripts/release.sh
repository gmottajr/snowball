if [[ "$OSTYPE" != "darwin"* ]]; then
    sudo -n apt install software-properties-common
    sudo -n add-apt-repository ppa:ubuntu-toolchain-r/test
    sudo -n apt update
    sudo -n apt install gcc-9 g++-9 -y
    sudo -n apt install cmake curl -y

    export CC="/usr/bin/gcc-9"
    export CXX="/usr/bin/g++-9"
fi

mkdir -p bin/Release
cmake  -DCMAKE_INSTALL_PREFIX:PATH=/usr/local -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release \
-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
-DLLVM_ENABLE_BACKTRACES=OFF \
-DLLVM_ENABLE_PER_TARGET_RUNTIME_DIR=OFF \
-DLLVM_ENABLE_TERMINFO=OFF \
-DLLVM_ENABLE_ZLIB=OFF \
-DLLVM_INCLUDE_EXAMPLES=OFF \
-DCMAKE_OSX_ARCHITECTURES="$ARCH" \
-DLLVM_INCLUDE_DOCS=OFF -DEXECUTABLE_OUTPUT_PATH="bin/Release" .
cmake --build . --config Release -- $@ -j10