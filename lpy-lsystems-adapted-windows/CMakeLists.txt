project(lpy)
cmake_minimum_required(VERSION 2.8.12)

option(BUILDPYTHON2 "Build for Python 2" OFF)

# add CMakeLists.txt source dir and cmake module dir to the cmake module path
# cmake modules are cmake files used e.g. to find library files
set(CMAKE_MODULE_PATH 
    ${CMAKE_MODULE_PATH} 
    "${CMAKE_SOURCE_DIR}"
    ) 

# specify build dir for out-of-source build (aka shadow build)
if(BUILDPYTHON2)
    set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
else(BUILDPYTHON2)
    set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/buildpy2)
endif(BUILDPYTHON2)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


### GCC COMPILER FLAGS ###
#set(CMAKE_CXX_FLAGS "-g -std=c++0x -w") # no warnings
#set(CMAKE_CXX_FLAGS "-g -std=c++0x -Wall") # all warnings
set(CMAKE_CXX_FLAGS "-g -std=c++0x -Wl,--no-undefined") # tell linker to report undefined symbols


### EXTERNAL LIBRARIES ###

if(BUILDPYTHON2)
    find_package(PythonLibs 2.6 REQUIRED)
    find_package(Boost 1.49 COMPONENTS python REQUIRED)
else(BUILDPYTHON2)
    find_package(PythonLibs 3.5 REQUIRED)
    find_package(Boost 1.49 COMPONENTS python3 REQUIRED)
endif(BUILDPYTHON2)

find_package(Qt5Core REQUIRED)


### SOURCE FILES ###

# relative path to lpy core source files
set(SRC_CORE
    src/cpp/abstractlstring.h
    src/cpp/argcollector.h
    src/cpp/argcollector_core.h
    src/cpp/axialtree.cpp
    src/cpp/axialtree.h
    src/cpp/axialtree_iter.h
    src/cpp/axialtree_manip.h
    src/cpp/compilation.cpp
    src/cpp/compilation.h
    src/cpp/consider.cpp
    src/cpp/consider.h
    src/cpp/error.cpp
    src/cpp/error.h
    src/cpp/global.h
#   src/cpp/interpretation.cpp
#   src/cpp/interpretation.h
    src/cpp/lpy_config.h
    src/cpp/lpy_parser.cpp
    src/cpp/lpy_parser.h
    src/cpp/lstringmatcher.cpp
    src/cpp/lstringmatcher.h
    src/cpp/lsyscontext.cpp
    src/cpp/lsyscontext.h
    src/cpp/lsysoptions.cpp
    src/cpp/lsysoptions.h
    src/cpp/lsysrule.cpp
    src/cpp/lsysrule.h
    src/cpp/lsystem.cpp
    src/cpp/lsystem.h
    src/cpp/matching.cpp
    src/cpp/matching.h
    src/cpp/matching_tmpl.h
    src/cpp/module.cpp
    src/cpp/module.h
    src/cpp/moduleclass.cpp
    src/cpp/moduleclass.h
    src/cpp/modulevtable.cpp
    src/cpp/modulevtable.h
    src/cpp/nodemodule.cpp
    src/cpp/nodemodule.h
    src/cpp/packedargs.h
    src/cpp/paramproduction.cpp
    src/cpp/paramproduction.h
    src/cpp/patternmodule.cpp
    src/cpp/patternmodule.h
    src/cpp/patternstring.cpp
    src/cpp/patternstring.h
#   src/cpp/plot.cpp
#   src/cpp/plot.h
    src/cpp/predefinedmodules.cpp
    src/cpp/stringmatching.cpp
    src/cpp/stringmatching.h
    src/cpp/tracker.cpp
    src/cpp/tracker.h
    )
	
# relative path to boost python wrapper source files
set(SRC_WRAPPER
    src/wrapper/export_axialtree.cpp
    src/wrapper/export_consider.cpp
    src/wrapper/export_debugger.cpp
#   src/wrapper/export_interpretation.cpp
    src/wrapper/export_lstring.h
    src/wrapper/export_lsyscontext.cpp
    src/wrapper/export_lsysoptions.cpp
    src/wrapper/export_lsysrule.cpp
    src/wrapper/export_lsystem.cpp
    src/wrapper/export_lsystem.h
    src/wrapper/export_module.cpp
    src/wrapper/export_module.h
    src/wrapper/export_moduleclass.cpp
    src/wrapper/export_nodemodule.cpp
    src/wrapper/export_parser.cpp
    src/wrapper/export_patternmodule.cpp
    src/wrapper/export_patternstring.cpp
#   src/wrapper/export_plot.cpp
    src/wrapper/export_stringmatching.cpp
    src/wrapper/lsystems_wrapper.cpp
    )
    
# relative path to plantgl source files
set(SRC_PLANTGL
    src/plantgl/math/math_config.h
    src/plantgl/math/util_math.cpp
    src/plantgl/math/util_math.h
    src/plantgl/math/util_polymath.h
    src/plantgl/math/util_vector.cpp
    src/plantgl/math/util_vector.h
    src/plantgl/python/exception.h
    src/plantgl/python/exception_core.h
    src/plantgl/python/export_list.h
    src/plantgl/python/extract_list.h
    src/plantgl/python/pyinterpreter.h
    src/plantgl/python/intrusive_ptr_python_helper.h
    src/plantgl/python/export_refcountptr.h
    src/plantgl/python/pyobj_reference.h
    src/plantgl/tool/rcobject.h
    src/plantgl/tool/sequencer.cpp
    src/plantgl/tool/sequencer.h
    src/plantgl/tool/tools_config.h
    src/plantgl/tool/util_assert.h
    src/plantgl/tool/util_hash.h
    src/plantgl/tool/util_hashmap.h
    src/plantgl/tool/util_hashset.h
    src/plantgl/tool/util_string.h
    src/plantgl/tool/util_tuple.h
    src/plantgl/tool/util_types.h
    src/plantgl/pgl_config.h
    )
		
# adds an library target with given name to be built from the source files listed afterwards
add_library(${PROJECT_NAME} SHARED ${SRC_CORE} ${SRC_WRAPPER} ${SRC_PLANTGL})

### INCLUDE HEADER FILES ###
include_directories(${PYTHON_INCLUDE_DIRS}
                    ${Boost_INCLUDE_DIR}
                    )
                    
SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_STATIC_RUNTIME OFF)

### LINK LIBRARIES ###
target_link_libraries(${PROJECT_NAME}
                      ${PYTHON_LIBRARIES}
                      ${Boost_LIBRARIES}
                      Qt5::Core
                      )
