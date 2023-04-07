add_executable(
        archiver
        archiver.cpp
        archiver.h IOHelper.h archiver_implementation.cpp)

add_executable(debug debug.cpp archiver_implementation.cpp)
