cmake_minimum_required(VERSION 3.16)
project(DicomReader)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# DCMTK paths for Homebrew on macOS
set(DCMTK_ROOT "/opt/homebrew/Cellar/dcmtk/3.6.9")
set(DCMTK_INCLUDE_DIRS "${DCMTK_ROOT}/include")
set(DCMTK_LIBRARY_DIRS "${DCMTK_ROOT}/lib")

# DCMTK libraries needed for basic DICOM operations
set(DCMTK_LIBRARIES
    dcmdata
    ofstd
    oflog
    oficonv
)

# Add executable
add_executable(main main.cpp)

# Include DCMTK headers
target_include_directories(main PRIVATE ${DCMTK_INCLUDE_DIRS})

# Link DCMTK libraries
target_link_directories(main PRIVATE ${DCMTK_LIBRARY_DIRS})
target_link_libraries(main ${DCMTK_LIBRARIES}) 