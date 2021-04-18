#!/bin/bash

# set -v
clear; printf '\033[3J'

export OMP_PROC_BIND=FALSE

pushd()
{
    cd $1
}
   
popd()
{
    cd ../
}

copy()
{
    cp $1/$2 $3/$2
}

# export HOME=/c/POSIX/mingw64/home
# export PATH="/c/POSIX/usr/bin":"/mingw64/bin":"/usr/local/bin":"/usr/bin":"/bin":"/mingw64/bin":"/usr/bin":"/c/POSIX/LLVM/bin":"/c/POSIX/mingw64/bin":"/c/POSIX/GnuWin32/bin":"/c/POSIX/MKL/bin/x64":"/c/PROGRA~1/NVIDIA GPU Computing Toolkit/CUDA/v8.0/bin":"/c/PROGRA~1/NVIDIA GPU Computing Toolkit/CUDA/v8.0/libnvvp":"/c/PROGRA~1/Git/cmd":"/c/PROGRA~1/CMake/bin":"/c/PROGRA~1/NVIDIA GPU Computing Toolkit/CUDA/v8.0/bin":"/c/PROGRA~1/NVIDIA GPU Computing Toolkit/CUDA/v8.0/libnvvp":"/c/WINDOWS/system32":"/c/WINDOWS":"/c/WINDOWS/System32/Wbem":"/c/WINDOWS/System32/WindowsPowerShell/v1.0":"/c/WINDOWS/System32/OpenSSH":"/c/PROGRA~1/Microsoft SQL Server/130/Tools/Binn":"/c/PROGRA~2/NVIDIA Corporation/PhysX/Common":"/c/PROGRA~2/Microsoft Visual Studio/Shared/Python36_64":"/c/PROGRA~1/Microsoft VS Code/bin":"/c/PROGRA~1/dotnet":"/c/PROGRA~2/dotnet":"/c/PROGRA~1/Microsoft VS Code/bin":"/c/POSIX/swig":"/c/Users/hx11342/AppData/Local/Microsoft/WindowsApps":"/c/Users/hx11342/.dotnet/tools":"/usr/bin/vendor_perl":"/usr/bi/core_perl"

# mkdir Debug-x64

CONFIGURATION="Release"
BUILD_TYPE="$CONFIGURATION"


# Debug, Release, RelWithDebInfo, MinSizeRel

PLATFORM="x64"

buildDir=$PWD

if [ ! -d "$PLATFORM-$CONFIGURATION-Shared" ]; then
  mkdir "$PLATFORM-$CONFIGURATION-Shared"
fi

pushd "$PLATFORM-$CONFIGURATION-Shared"

if [ "$1" = "cmake" ]; then
    
    # -C C:/POSIX/ClangCudaToolchain.cmake \
    
    # get_filename_component(LLVM_BIN_DIR ${CMAKE_CXX_COMPILER_AR} DIRECTORY)
    # COMPILER_SPECIFIC_LIBS(Clang "${LLVM_BIN_DIR}/../lib/libomp.lib")    
    
    shift & cmake -G "Unix Makefiles" \
    -Wno-dev \
    -C C:/POSIX/ClangCudaToolchain.cmake \
    -DCMAKE_TOOLCHAIN_FILE:FILEPATH="C:/POSIX/ClangCudaToolchain.cmake" \
    -DBUILD_SHARED_LIBS:BOOL=ON \
    -DCUDA_ENABLED:BOOL=TRUE \
    -DCMAKE_BUILD_TYPE:STRING="$BUILD_TYPE" \
    -DCMAKE_INSTALL_PREFIX="E:/AssemblyCache/kokkos" \
    -DKokkos_ENABLE_ASM:BOOL=ON \
    -DKokkos_ARCH_HSW:BOOL=ON \
    -DKokkos_ENABLE_OPENMP:BOOL=ON \
    -DKokkos_ARCH_MAXWELL52:BOOL=ON \
    -DKokkos_CXX_STANDARD:STRING=20 \
    -DKokkos_ENABLE_AGGRESSIVE_VECTORIZATION:BOOL=ON \
    -DKokkos_ENABLE_COMPILER_WARNINGS:BOOL=OFF \
    -DKokkos_ENABLE_COMPLEX_ALIGN:BOOL=ON \
    -DKokkos_ENABLE_CUDA:BOOL=ON \
    -DKokkos_ENABLE_CUDA_CONSTEXPR:BOOL=ON \
    -DKokkos_ENABLE_CUDA_LAMBDA:BOOL=ON \
    -DKokkos_ENABLE_CUDA_LDG_INTRINSIC:BOOL=ON \
    -DKokkos_ENABLE_CUDA_RELOCATABLE_DEVICE_CODE:BOOL=OFF \
    -DKokkos_ENABLE_CUDA_UVM:BOOL=ON \
    -DKokkos_ENABLE_DEBUG:BOOL=OFF \
    -DKokkos_ENABLE_DEBUG_BOUNDS_CHECK:BOOL=OFF \
    -DKokkos_ENABLE_DEBUG_DUALVIEW_MODIFY_CHECK:BOOL=OFF \
    -DKokkos_ENABLE_DEPRECATED_CODE:BOOL=OFF \
    -DKokkos_ENABLE_HWLOC:BOOL=ON \
    -DKokkos_ENABLE_SERIAL:BOOL=ON \
    -DKokkos_ENABLE_TASKDAG:BOOL=ON \
    -DKokkos_ENABLE_TESTS:BOOL=OFF \
    -DKokkos_ENABLE_EXAMPLES:BOOL=OFF \
    -DKOKKOS_LINK_LIBRARIES:INTERNAL=C:/POSIX/usr/local/lib/libomp.lib \
    D:/TFS_Sources/Github/Compilation/kokkos/kokkos

    echo "fixing -fPIC"
    find . -name 'build.make' -exec sed -i -e 's| -fPIC | |g' {} \;
    find . -name 'flags.make' -exec sed -i -e 's| -fopenmp=libomp -march=core-avx2 -mtune=core-avx2 -x cuda --cuda-gpu-arch=sm_52 -std=c++20| |g' {} \;
    find . -name 'linklibs.rsp' -exec sed -i -e 's| -lON | |g' {} \;
    
    
    
    
    
fi

if [ "$1" == "fix" ]; then
    shift
    
    find . -name 'linklibs.rsp' -exec sed -i -e 's|C:/POSIX/usr/local/lib/libomp.lib C:/POSIX/mingw64/lib/libdl.a|C:/POSIX/usr/local/lib/libomp.lib E:/AssemblyCache/hwloc/lib/libhwloc.a C:/POSIX/mingw64/lib/libdl.a|g' {} \;
    
    find . -name 'linklibs.rsp' -exec sed -i -e 's|C:/POSIX/usr/local/lib/libomp.lib E:/AssemblyCache/hwloc/lib/libhwloc.a C:/POSIX/mingw64/lib/libdl.a|C:/POSIX/usr/local/lib/libomp.lib E:/AssemblyCache/hwloc/lib/libhwloc.a C:/POSIX/mingw64/lib/libltdl.a C:/POSIX/mingw64/lib/libdl.a|g' {} \;
        
    
    
    # echo "fixing -fPIC"
    # find . -name 'build.make' -exec sed -i -e 's| -fPIC / |g' {} \;
    
    
    # echo "fixing lib names"
    
    # pushd D:/TFS_Sources/Build/kokkos/x64-Release/core/src
    
    # find . -name 'build.make' -exec sed -i -e 's|--out-implib,libKokkos.dll.a|--out-implib,libkokkoscore.dll.a|g' {} \;
    
    # popd
    
    # pushd D:/TFS_Sources/Build/kokkos/x64-Release/core/unit_test/CMakeFiles/kokkos_gtest.dir
    
    # find . -name 'build.make' -exec sed -i -e 's|--out-implib,libKokkos.dll.a|--out-implib,libkokkos_gtest.dll.a|g' {} \;
    
    # popd
    
    # pushd D:/TFS_Sources/Build/kokkos/x64-Release/containers/src
    
    # find . -name 'build.make' -exec sed -i -e 's|--out-implib,libKokkos.dll.a|--out-implib,libkokkoscontainers.dll.a|g' {} \;
    
    # popd
    
    
    
	# find . -name 'KokkosCore_config.h' -exec sed -i -e 's|KOKKOS_ARCH_MAXWELL52/KOKKOS_ARCH_MAXWELL52\n#define KOKKOS_ENABLE_TASKDAG|g' {} \;
    # ./fixCygwin.sh
    # echo "fixing -x c cuda"
    # find . -name '' -exec sed -i -e 's|-x c cuda/-x cuda|g' {} \;
    # echo "fixing -x c++ cuda"
    # find . -name 'flags.make' -exec sed -i -e 's|-x c++ cuda/-x cuda|g' {} \;
        
    # find . -name 'build.make' -exec sed -i -e 's|-isysroot C:\/AssemblyCache\/Cygwin/-isysroot C:\/AssemblyCache\/Cygwin\/usr|g' {} \;
    
    # find . -name 'build.make' -exec sed -i -e 's|clang++ -iwithsysroot D:\/POSIX\/Cygwin -isysroot D:\/POSIX\/Cygwin/clang++ -isysroot C:\/AssemblyCache\/Cygwin|g' {} \;
    
    # find . -name 'build.make' -exec sed -i -e 's|clang++/clang++ -iwithsysroot D:\/POSIX\/Cygwin -isysroot D:\/POSIX\/Cygwin|g' {} \;
    
    # find . -name 'includes_C.rsp' -exec sed -i -e 's|D:\/POSIX\/Cygwin\/usr\/include/-ID:\/POSIX\/Cygwin\/usr\/include|g' {} \;
    # find . -name 'includes_CXX.rsp' -exec sed -i -e 's|D:\/POSIX\/Cygwin\/usr\/include/-ID:\/POSIX\/Cygwin\/usr\/include|g' {} \;
fi

if [ "$1" == "make" ]; then
    shift
    if [ -z $1 ]; then
        make -j 16
    else
        make $@
    fi
fi

if [ "$1" == "install" ]; then

    make install/fast -j 4
    # cmake -P cmake_install.cmake  
    
    copy D:/TFS_Sources/Build/kokkos/x64-Release/example/query_device KokkosExample_query_device.exe E:/AssemblyCache/kokkos/bin
    copy D:/TFS_Sources/Build/kokkos/x64-Release/example/tutorial/01_hello_world KokkosExample_tutorial_01_hello_world.exe E:/AssemblyCache/kokkos/bin
    copy D:/TFS_Sources/Build/kokkos/x64-Release/example/tutorial/01_hello_world_lambda KokkosExample_tutorial_01_hello_world_lambda.exe E:/AssemblyCache/kokkos/bin
    copy D:/TFS_Sources/Build/kokkos/x64-Release/example/tutorial/02_simple_reduce KokkosExample_tutorial_02_simple_reduce.exe E:/AssemblyCache/kokkos/bin
    copy D:/TFS_Sources/Build/kokkos/x64-Release/example/tutorial/02_simple_reduce_lambda KokkosExample_tutorial_02_simple_reduce_lambda.exe E:/AssemblyCache/kokkos/bin
    copy D:/TFS_Sources/Build/kokkos/x64-Release/example/tutorial/03_simple_view KokkosExample_tutorial_03_simple_view.exe E:/AssemblyCache/kokkos/bin
    copy D:/TFS_Sources/Build/kokkos/x64-Release/example/tutorial/04_simple_memoryspaces KokkosExample_tutorial_04_simple_memoryspaces.exe E:/AssemblyCache/kokkos/bin
    copy D:/TFS_Sources/Build/kokkos/x64-Release/example/tutorial/05_simple_atomics KokkosExample_tutorial_05_simple_atomics.exe E:/AssemblyCache/kokkos/bin
    copy D:/TFS_Sources/Build/kokkos/x64-Release/example/tutorial/06_simple_mdrangepolicy KokkosExample_tutorial_06_simple_mdrangepolicy.exe E:/AssemblyCache/kokkos/bin
    copy D:/TFS_Sources/Build/kokkos/x64-Release/example/tutorial/Advanced_Views/01_data_layouts KokkosExample_tutorial_advancedviews_01_data_layouts.exe E:/AssemblyCache/kokkos/bin
    copy D:/TFS_Sources/Build/kokkos/x64-Release/example/tutorial/Advanced_Views/02_memory_traits KokkosExample_tutorial_advancedviews_02_memory_traits.exe E:/AssemblyCache/kokkos/bin
    copy D:/TFS_Sources/Build/kokkos/x64-Release/example/tutorial/Advanced_Views/03_subviews KokkosExample_tutorial_advancedviews_03_subviews.exe E:/AssemblyCache/kokkos/bin
    copy D:/TFS_Sources/Build/kokkos/x64-Release/example/tutorial/Advanced_Views/04_dualviews KokkosExample_tutorial_advancedviews_04_dualviews.exe E:/AssemblyCache/kokkos/bin
    copy D:/TFS_Sources/Build/kokkos/x64-Release/example/tutorial/Advanced_Views/05_NVIDIA_UVM KokkosExample_tutorial_advancedviews_05_nvidia_uvm.exe E:/AssemblyCache/kokkos/bin
    copy D:/TFS_Sources/Build/kokkos/x64-Release/example/tutorial/Hierarchical_Parallelism/01_thread_teams KokkosExample_tutorial_hierarchicalparallelism_01_thread_teams.exe E:/AssemblyCache/kokkos/bin
    copy D:/TFS_Sources/Build/kokkos/x64-Release/example/tutorial/Hierarchical_Parallelism/01_thread_teams_lambda KokkosExample_tutorial_hierarchical_01_thread_teams_lambda.exe E:/AssemblyCache/kokkos/bin
    copy D:/TFS_Sources/Build/kokkos/x64-Release/example/tutorial/Hierarchical_Parallelism/02_nested_parallel_for KokkosExample_tutorial_hierarchicalparallelism_02_nested_parallel_for.exe E:/AssemblyCache/kokkos/bin
    copy D:/TFS_Sources/Build/kokkos/x64-Release/example/tutorial/Hierarchical_Parallelism/03_vectorization KokkosExample_tutorial_hierarchicalparallelism_03_vectorization.exe E:/AssemblyCache/kokkos/bin
        
fi

if [ "$1" = "test" ]; then
	# rm $buildDir/test.log
	
    shift & ctest --progress -j 1 --output-log $buildDir/test.log    
fi

echo -e "\a"

exit 0




















# make -j 16 $@