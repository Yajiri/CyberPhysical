# Compiles the solution and runs it
# Requires data to be present in Shared Memory

cd ../src
rm -rf build
mkdir build && \
    cd build && \
    cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/tmp .. && \
    make && make install

./solution --cid=253 --name=img --width=640 --height=480 --verbose