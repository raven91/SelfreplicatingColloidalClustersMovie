cmake_minimum_required(VERSION 3.15)
project(SelfreplicatingColloidalClustersMovie)

set(CMAKE_CXX_STANDARD 17)

include_directories(/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/ /usr/local/include/ /usr/local/include/freetype2/)
link_directories(/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/lib/ /usr/local/lib/)
set(CMAKE_OSX_SYSROOT /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk)

add_executable(SelfreplicatingColloidalClustersMovie main.cpp Definitions.hpp ColorMapConversion.cpp Model.cpp Model.hpp Renderer.cpp Renderer.hpp RendererForStaticTetrahedrons.cpp RendererForStaticTetrahedrons.hpp ScreenshotHandler.cpp ScreenshotHandler.hpp)

file(GLOB LIBRARIES "/usr/local/lib/libGLEW.1.13.0.dylib" "/usr/local/lib/libglfw.3.1.dylib" "/usr/local/lib/libpng16.16.dylib" "/usr/local/lib/libfreetype.a" "/usr/lib/libbz2.1.0.dylib" "/usr/lib/libz.1.dylib")
message("LIBRARIES = ${LIBRARIES}")
target_link_libraries(SelfreplicatingColloidalClustersMovie  ${LIBRARIES})

#set(GCC_COVERAGE_COMPILE_FLAGS "-march=native -flto -O3")
#set(GCC_COVERAGE_LINK_FLAGS "")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-framework OpenGL" COMPILER_SUPPORTS_FRAMEWORK_OPENGL)
if(COMPILER_SUPPORTS_FRAMEWORK_OPENGL)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -framework OpenGL")
endif()