cmake_minimum_required(VERSION 3.16)
project(DicomReader)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(VTK REQUIRED)

set(DCMTK_ROOT "/opt/homebrew/Cellar/dcmtk/3.6.9")
set(DCMTK_INCLUDE_DIRS "${DCMTK_ROOT}/include")
set(DCMTK_LIBRARY_DIRS "${DCMTK_ROOT}/lib")

set(DCMTK_LIBRARIES
    dcmdata
    dcmimgle
    ofstd
    oflog
    oficonv
)

add_executable(main
    main.cpp
    dicom_utils.cpp
    Volume4D.cpp
)

target_include_directories(main PRIVATE
    ${DCMTK_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_directories(main PRIVATE ${DCMTK_LIBRARY_DIRS})
target_link_libraries(main ${DCMTK_LIBRARIES} ${VTK_LIBRARIES})

add_executable(vtk_test
    vtk_test.cpp
    dicom_utils.cpp
    Volume4D.cpp
)

target_include_directories(vtk_test PRIVATE
    ${DCMTK_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_directories(vtk_test PRIVATE ${DCMTK_LIBRARY_DIRS})
target_link_libraries(vtk_test ${DCMTK_LIBRARIES} ${VTK_LIBRARIES})

