FetchContent_Declare(
        bullet
        GIT_REPOSITORY https://github.com/bulletphysics/bullet3.git
        GIT_TAG        master
)

# Disable Bullet tests and demos
set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_CPU_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_OPENGL3_DEMOS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(bullet)
