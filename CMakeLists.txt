file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS src/*.cpp include/*)

add_library(mylibc STATIC ${SRCS})

target_include_directories(mylibc PUBLIC include)

target_link_libraries(mylibc INTERFACE myos_flags)

target_compile_options(mylibc PRIVATE
    -nostdlib
    -nostartfiles
)
target_compile_options(mylibc PUBLIC
    -m64
    -ffreestanding
    -fno-rtti
    -fno-exceptions
    -fno-zero-initialized-in-bss
    -fno-common
    -masm=intel
    -mno-sse -mno-sse2 -mno-mmx -mno-3dnow -mno-80387
    -msoft-float
    -mno-red-zone
    $<$<CONFIG:Debug>:-g;-O0>
    $<$<CONFIG:Release>:-O2>
)
