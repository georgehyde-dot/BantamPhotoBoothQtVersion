#!/binbash
set -e # Exit on error
PROJECT_DIR=~/src/cpp_version
BUILD_DIR=${PROJECT_DIR}/build
QT_PATH=~/Qt/6.7.3/gcc_arm64

echo "--- Cleaning and Rebuilding ---"
cd "${BUILD_DIR}"
rm -rf CMakeCache.txt CMakeFiles/ Makefile cmake_install.cmake QtPhotoBoothApp_autogen/
cmake -DCMAKE_PREFIX_PATH=${QT_PATH} -DCMAKE_BUILD_TYPE=Debug "${PROJECT_DIR}"
make -j$(nproc)

echo "--- Running Memory Test (e.g., Valgrind) ---"
valgrind --leak-check=full ./QtPhotoBoothApp 
# Or just run with ASan/LSan if compiled with it:
# ./QtPhotoBoothApp 
# (If your app auto-runs test cycles or you trigger them manually quickly)
echo "--- Test Finished ---"
