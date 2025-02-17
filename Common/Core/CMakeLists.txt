# Core configuration options.
if(CMAKE_SIZEOF_VOID_P GREATER 4)
 set(VTK_USE_64BIT_IDS_DEFAULT ON)
else()
 set(VTK_USE_64BIT_IDS_DEFAULT OFF)
 set(VTK_USE_64BIT_TIMESTAMPS_DEFAULT OFF)
 option(VTK_USE_64BIT_TIMESTAMPS "Build VTK with 64 bit timestamps"
  ${VTK_USE_64BIT_TIMESTAMPS_DEFAULT})
endif()

option(VTK_USE_64BIT_IDS "Build VTK with 64 bit ids"
  ${VTK_USE_64BIT_IDS_DEFAULT})
mark_as_advanced(VTK_USE_64BIT_IDS)

option(VTK_DEBUG_LEAKS "Build leak checking support into VTK." OFF)
mark_as_advanced(VTK_DEBUG_LEAKS)
include(CMakeDependentOption)
# See vtkDataArrayRange.h docs for more info on these:
cmake_dependent_option(VTK_DEBUG_RANGE_ITERATORS
  "Detect usage errors with the for-range iterators (SLOW)." OFF
  "CMAKE_BUILD_TYPE STREQUAL \"Debug\"" OFF)
mark_as_advanced(VTK_DEBUG_RANGE_ITERATORS)
cmake_dependent_option(VTK_ALWAYS_OPTIMIZE_ARRAY_ITERATORS
  "Enable additional optimizations for array iterators in Debug builds." OFF
  "NOT VTK_DEBUG_RANGE_ITERATORS" OFF)
mark_as_advanced(VTK_ALWAYS_OPTIMIZE_ARRAY_ITERATORS)

# This option determines the behavior of the New methods of vtkObject derived
# classes. If this option is off then only classes specifically using the
# vtkObjectFactoryNewMacro and vtkAbstractObjectFactoryNewMacro will allow for
# overrides. If the method is on then vtkStandardNewMacro will use the
# vtkObjectFactoryNewMacro's implementation body.
option(VTK_ALL_NEW_OBJECT_FACTORY
  "Build all vtkObject derived classes with object factory new methods." OFF)
mark_as_advanced(VTK_ALL_NEW_OBJECT_FACTORY)

set(vtk_smp_sources)
set(vtk_smp_headers)
set(vtk_smp_nowrap_headers)
set(vtk_smp_templates)
set(vtk_smp_defines)
set(vtk_smp_libraries)
set(vtk_smp_enable_sequential 0)
set(vtk_smp_enable_stdthread 0)
set(vtk_smp_enable_tbb 0)
set(vtk_smp_enable_openmp 0)
set(VTK_SMP_IMPLEMENTATION_TYPE)
include("${CMAKE_CURRENT_SOURCE_DIR}/vtkSMPSelection.cmake")

# An optional dependency on memkind
set(vtk_memkind_libraries)
if (VTK_USE_MEMKIND)
  vtk_module_find_package(
    PACKAGE MEMKIND)
  list(APPEND vtk_memkind_libraries
    MEMKIND::MEMKIND)
  get_target_property(MEMKIND_VERSION_MINOR MEMKIND::MEMKIND MEMKIND_VERSION_MINOR)
  list(APPEND VTK_OBJECT_BASE_DEFINES "MEMKIND_VERSION_MINOR=${MEMKIND_VERSION_MINOR}")
endif()

# Generate the vtkTypeList_Create macros:
include("${CMAKE_CURRENT_SOURCE_DIR}/vtkCreateTypeListMacros.cmake")
CreateTypeListMacros(
  VTK_TYPELISTMACRO_HEADER_CONTENTS
  99
  vtkTypeList_Create
  "vtkTypeList::TypeList"
  "vtkTypeList::NullType")
configure_file(vtkTypeListMacros.h.in vtkTypeListMacros.h)

# And the ArrayDispatch array list header:
option(VTK_DISPATCH_AOS_ARRAYS "Include array-of-structs vtkDataArray subclasses in dispatcher." ON)
option(VTK_DISPATCH_SOA_ARRAYS "Include struct-of-arrays vtkDataArray subclasses in dispatcher." OFF)
option(VTK_DISPATCH_TYPED_ARRAYS "Include vtkTypedDataArray subclasses (e.g. old mapped arrays) in dispatcher." OFF)
option(VTK_WARN_ON_DISPATCH_FAILURE "If enabled, vtkArrayDispatch will print a warning when a dispatch fails." OFF)
mark_as_advanced(
  VTK_DISPATCH_AOS_ARRAYS
  VTK_DISPATCH_SOA_ARRAYS
  VTK_DISPATCH_TYPED_ARRAYS
  VTK_WARN_ON_DISPATCH_FAILURE)

option(VTK_BUILD_SCALED_SOA_ARRAYS "Include struct-of-arrays with scaled vtkDataArray implementation." OFF)

include("${CMAKE_CURRENT_SOURCE_DIR}/vtkCreateArrayDispatchArrayList.cmake")
vtkArrayDispatch_default_array_setup()
vtkArrayDispatch_generate_array_header(VTK_ARRAYDISPATCH_ARRAY_LIST)
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/vtkArrayDispatchArrayList.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/vtkArrayDispatchArrayList.h"
  @ONLY)

# create a list of source files for explicit array type instantiations
set(instantiation_sources)

# steal the list of array types from vtkCreateArrayDispatchArrayList.cmake
foreach (INSTANTIATION_VALUE_TYPE IN LISTS vtkArrayDispatch_all_types)
  # only instantiate fundamental types, not typedefs
  if (INSTANTIATION_VALUE_TYPE STREQUAL "vtkIdType")
    continue()
  endif ()
  # create a file suffix from the type
  string(REPLACE " " "_" _suffix "${INSTANTIATION_VALUE_TYPE}")

  set(_list "vtkSOADataArrayTemplateInstantiate")
  if (VTK_BUILD_SCALED_SOA_ARRAYS)
    list(APPEND _list "vtkScaledSOADataArrayTemplateInstantiate")
  endif ()
  # see comments in vtkGenericDataArray.h for explanation
  if (INSTANTIATION_VALUE_TYPE MATCHES "(^| )long( |$)")
    list(APPEND _list "vtkGenericDataArrayValueRangeInstantiate")
  endif ()

  # generate cxx file to instantiate template with this type
  foreach (_prefix IN LISTS _list)
    configure_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/${_prefix}.cxx.in"
      "${CMAKE_CURRENT_BINARY_DIR}/${_prefix}_${_suffix}.cxx"
      @ONLY)
    list(APPEND instantiation_sources
      "${CMAKE_CURRENT_BINARY_DIR}/${_prefix}_${_suffix}.cxx")
  endforeach ()
endforeach ()

set(classes
  vtkAbstractArray
  vtkAnimationCue
  vtkArchiver
  vtkArray
  vtkArrayCoordinates
  vtkArrayExtents
  vtkArrayExtentsList
  vtkArrayIterator
  vtkArrayRange
  vtkArraySort
  vtkArrayWeights
  vtkBitArray
  vtkBitArrayIterator
  vtkBoxMuellerRandomSequence
  vtkBreakPoint
  vtkByteSwap
  vtkCallbackCommand
  vtkCharArray
  vtkCollection
  vtkCollectionIterator
  vtkCommand
  vtkCommonInformationKeyManager
  vtkDataArray
  vtkDataArrayCollection
  vtkDataArrayCollectionIterator
  vtkDataArraySelection
  vtkDebugLeaks
  vtkDebugLeaksManager
  vtkDoubleArray
  vtkDynamicLoader
  vtkEventForwarderCommand
  vtkFileOutputWindow
  vtkFloatArray
  vtkFloatingPointExceptions
  vtkGarbageCollector
  vtkGarbageCollectorManager
  vtkGaussianRandomSequence
  vtkIdList
  vtkIdListCollection
  vtkIdTypeArray
  vtkIndent
  vtkInformation
  vtkInformationDataObjectKey
  vtkInformationDoubleKey
  vtkInformationDoubleVectorKey
  vtkInformationIdTypeKey
  vtkInformationInformationKey
  vtkInformationInformationVectorKey
  vtkInformationIntegerKey
  vtkInformationIntegerPointerKey
  vtkInformationIntegerVectorKey
  vtkInformationIterator
  vtkInformationKey
  vtkInformationKeyLookup
  vtkInformationKeyVectorKey
  vtkInformationObjectBaseKey
  vtkInformationObjectBaseVectorKey
  vtkInformationRequestKey
  vtkInformationStringKey
  vtkInformationStringVectorKey
  vtkInformationUnsignedLongKey
  vtkInformationVariantKey
  vtkInformationVariantVectorKey
  vtkInformationVector
  vtkIntArray
  vtkLargeInteger
  vtkLogger
  vtkLongArray
  vtkLongLongArray
  vtkLookupTable
  vtkMath
  vtkMersenneTwister
  vtkMinimalStandardRandomSequence
  vtkMultiThreader
  vtkOStrStreamWrapper
  vtkOStreamWrapper
  vtkObject
  vtkObjectBase
  vtkObjectFactory
  vtkObjectFactoryCollection
  vtkOldStyleCallbackCommand
  vtkOutputWindow
  vtkOverrideInformation
  vtkOverrideInformationCollection
  vtkPoints
  vtkPoints2D
  vtkPriorityQueue
  vtkRandomPool
  vtkRandomSequence
  vtkReferenceCount
  vtkScalarsToColors
  vtkShortArray
  vtkSignedCharArray
  vtkSmartPointerBase
  vtkSortDataArray
  vtkStdString
  vtkStringArray
  vtkStringOutputWindow
  vtkTimePointUtility
  vtkTimeStamp
  vtkUnicodeString
  vtkUnicodeStringArray
  vtkUnsignedCharArray
  vtkUnsignedIntArray
  vtkUnsignedLongArray
  vtkUnsignedLongLongArray
  vtkUnsignedShortArray
  vtkVariant
  vtkVariantArray
  vtkVersion
  vtkVoidArray
  vtkWeakPointerBase
  vtkWeakReference
  vtkWindow
  vtkXMLFileOutputWindow

  # These classes are deprecated.
  vtkConditionVariable
  vtkCriticalSection
  vtkSimpleCriticalSection
  vtkMutexLock)

set(template_classes
  vtkAOSDataArrayTemplate
  vtkArrayDispatch
  vtkArrayInterpolate
  vtkArrayIteratorTemplate
  vtkArrayPrint
  vtkDenseArray
  vtkGenericDataArray
  vtkMappedDataArray
  vtkSOADataArrayTemplate
  vtkSparseArray
  vtkTypedArray
  vtkTypedDataArray
  vtkTypeList)

set(sources
  vtkArrayIteratorTemplateInstantiate.cxx
  vtkGenericDataArray.cxx
  ${instantiation_sources}
  ${vtk_smp_sources})

if (VTK_BUILD_SCALED_SOA_ARRAYS)
  list (APPEND template_classes vtkScaledSOADataArrayTemplate)
  set(VTK_USE_SCALED_SOA_ARRAYS 1)
endif ()
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/vtkVTK_USE_SCALED_SOA_ARRAYS.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/vtkVTK_USE_SCALED_SOA_ARRAYS.h")

set(headers
  vtkABI.h
  vtkArrayIteratorIncludes.h
  vtkAssume.h
  vtkAutoInit.h
  vtkBuffer.h
  vtkCollectionRange.h
  vtkCompiler.h
  vtkDataArrayAccessor.h
  vtkDataArrayIteratorMacro.h
  vtkDataArrayMeta.h
  vtkDataArrayRange.h
  vtkDataArrayTupleRange_AOS.h
  vtkDataArrayTupleRange_Generic.h
  vtkDataArrayValueRange_AOS.h
  vtkDataArrayValueRange_Generic.h
  vtkDeprecation.h
  vtkEventData.h
  vtkGenericDataArrayLookupHelper.h
  vtkIOStream.h
  vtkIOStreamFwd.h
  vtkInformationInternals.h
  vtkMathUtilities.h
  vtkMatrixUtilities.h
  vtkMeta.h
  vtkNew.h
  vtkRange.h
  vtkRangeIterableTraits.h
  vtkSetGet.h
  vtkSmartPointer.h
  vtkSystemIncludes.h
  vtkTemplateAliasMacro.h
  vtkTestDataArray.h
  vtkType.h
  vtkTypeTraits.h
  vtkTypedDataArrayIterator.h
  vtkVariantCast.h
  vtkVariantCreate.h
  vtkVariantExtract.h
  vtkVariantInlineOperators.h
  vtkWeakPointer.h
  vtkWin32Header.h
  vtkWindows.h
  vtkWrappingHints.h
  ${vtk_smp_headers}
  "${CMAKE_CURRENT_BINARY_DIR}/vtkArrayDispatchArrayList.h"
  "${CMAKE_CURRENT_BINARY_DIR}/vtkMathConfigure.h"
  "${CMAKE_CURRENT_BINARY_DIR}/vtkTypeListMacros.h"
  "${CMAKE_CURRENT_BINARY_DIR}/vtkVTK_USE_SCALED_SOA_ARRAYS.h")

set(private_headers
  "${CMAKE_CURRENT_BINARY_DIR}/vtkFloatingPointExceptionsConfigure.h")

set(templates
  vtkArrayIteratorTemplateImplicit.txx
  ${vtk_smp_templates})

set(private_templates
  vtkDataArrayPrivate.txx)

set(vtk_include_dirs)

vtk_module_find_package(
  PACKAGE Threads)

set(VTK_MAX_THREADS "64" CACHE STRING
  "Max number of threads vtkMultiThreader will allocate.")
mark_as_advanced(VTK_MAX_THREADS)

if (CMAKE_USE_PTHREADS_INIT AND CMAKE_USE_WIN32_THREADS_INIT)
  if (DEFINED _vtk_thread_impl_output)
    set(thread_impl_output "${_vtk_thread_impl_output}")
  else ()
    execute_process(
      COMMAND "${CMAKE_C_COMPILER}" -v
      OUTPUT_VARIABLE thread_impl_output
      ERROR_VARIABLE  thread_impl_output
      RESULT_VARIABLE result
      TIMEOUT         10)
    if (result)
      message(FATAL_ERROR
        "Failed to determine the thread implementation in use by the "
        "compiler.")
    endif ()
    string (REGEX REPLACE ".*Thread model: \((posix|win32)\).*" "\\1" thread_impl_output "${thread_impl_output}")
  endif ()
  set(_vtk_thread_impl_output "${thread_impl_output}"
    CACHE INTERNAL "thread model detection")
  if (thread_impl_output MATCHES "posix")
    set(VTK_USE_PTHREADS 1)
  elseif (thread_impl_output MATCHES "win32")
    set(VTK_USE_WIN32_THREADS 1)
  else ()
    set(VTK_USE_PTHREADS 1)
  endif ()
elseif (CMAKE_USE_PTHREADS_INIT)
  set(VTK_USE_PTHREADS 1)
elseif (CMAKE_USE_WIN32_THREADS_INIT)
  set(VTK_USE_WIN32_THREADS 1)
endif ()

# We can't check for the target since we're before VTK::CommonDataModel in the
# dependency graph.
if (VTK_MODULE_ENABLE_VTK_CommonDataModel)
  list(APPEND vtk_include_dirs
    "${CMAKE_CURRENT_BINARY_DIR}/../DataModel")
  set_property(SOURCE vtkInformationDataObjectKey.cxx
    PROPERTY
      COMPILE_DEFINITIONS vtkCommonDataModel_ENABLED)
endif ()

if(WIN32)
  list(APPEND classes
    vtkWin32OutputWindow
    vtkWin32ProcessOutputWindow)
  list(APPEND sources
    vtkWin32OutputWindowProcessEncoded.c)
endif()

if(ANDROID)
  list(APPEND classes
    vtkAndroidOutputWindow)
endif()

# Platform configuration tests.
include(TestBigEndian)
test_big_endian(VTK_WORDS_BIGENDIAN)

set(nowrap_headers
  vtkMathPrivate.hxx
  ${vtk_smp_nowrap_headers})
set(generated_headers
  vtkBuild.h
  vtkDebug.h
  vtkDebugRangeIterators.h
  vtkEndian.h
  vtkFeatures.h
  vtkLegacy.h
  vtkOptions.h
  vtkPlatform.h
  vtkSMP.h
  vtkThreads.h
  vtkToolkits.h
  vtkVersionFull.h
  vtkVersionMacros.h)
if (NOT VTK_LEGACY_REMOVE)
  list(APPEND generated_headers
    vtkConfigureDeprecated.h)
endif ()
foreach (generated_header IN LISTS generated_headers)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/${generated_header}.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${generated_header}"
    @ONLY)
  list(APPEND headers
    "${CMAKE_CURRENT_BINARY_DIR}/${generated_header}")
endforeach ()

if (NOT VTK_LEGACY_REMOVE)
  list(APPEND nowrap_headers
    vtkConfigure.h)
endif ()

add_custom_command(
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/CaseFolding.txt"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/vtkUnicodeString.cmake"
  COMMAND "${CMAKE_COMMAND}"
          "-Dsource_dir=${CMAKE_CURRENT_SOURCE_DIR}"
          "-Dbinary_dir=${CMAKE_CURRENT_BINARY_DIR}"
          -P "${CMAKE_CURRENT_SOURCE_DIR}/vtkUnicodeString.cmake"
  OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/vtkUnicodeCaseFoldData.h")

set_source_files_properties(vtkUnicodeString.cxx
  PROPERTIES
    OBJECT_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/vtkUnicodeCaseFoldData.h")

include("${CMAKE_CURRENT_SOURCE_DIR}/vtkTypeArrays.cmake")

include(CheckCXXSourceCompiles)

macro(CHECK_CXX_EXPRESSION_COMPILES EXPRESSION FILES VARIABLE)
  set(SOURCE "/* CHECK_CXX_EXPRESSION_COMPILES */\n")
  foreach(FILE ${FILES})
    set(SOURCE "${SOURCE}#include <${FILE}>\n")
  endforeach()
  set(SOURCE "${SOURCE}\nint main()\n{\n")
  set(SOURCE "${SOURCE}  static_cast<void>(${EXPRESSION});\n\n")
  set(SOURCE "${SOURCE}  return 0;\n}\n")
  check_cxx_source_compiles("${SOURCE}" "${VARIABLE}")
endmacro()

# Check C++ <cmath> first, where the C++11 standard says these must be.
check_cxx_expression_compiles("std::isnan(0.0)" "cmath" VTK_HAS_STD_ISNAN)
check_cxx_expression_compiles("std::isinf(0.0)" "cmath" VTK_HAS_STD_ISINF)
check_cxx_expression_compiles("std::isfinite(0.0)" "cmath" VTK_HAS_STD_ISFINITE)

include(CheckSymbolExists)

# Check C99 <math.h> next, where the C99 standard says these must be.
# (they will be found even if they are defined as macros)
check_symbol_exists(isnan "math.h" VTK_HAS_ISNAN)
check_symbol_exists(isinf "math.h" VTK_HAS_ISINF)
check_symbol_exists(isfinite "math.h" VTK_HAS_ISFINITE)

# Check for finite() which is widespread, though non-standard.
check_symbol_exists(finite "math.h" VTK_HAS_FINITE)

# Check float.h last, where Windows keeps its underscore versions.
if(NOT VTK_HAS_ISNAN AND NOT VTK_HAS_STD_ISNAN)
  check_symbol_exists(_isnan "float.h" VTK_HAS__ISNAN)
endif()
if(NOT VTK_HAS_FINITE AND NOT VTK_HAS_ISFINITE AND NOT VTK_HAS_STD_ISFINITE)
  check_symbol_exists(_finite "float.h" VTK_HAS__FINITE)
endif()

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/vtkMathConfigure.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/vtkMathConfigure.h")

# Figure out if feenableexcept and fedisableexcept functions are present
include(CheckIncludeFile)
check_include_file("fenv.h" HAVE_FENV_H)
if(HAVE_FENV_H)
  set(CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
  if(UNIX)
    set(CMAKE_REQUIRED_LIBRARIES m)
  endif()
  check_symbol_exists(feenableexcept "fenv.h" VTK_HAS_FEENABLEEXCEPT)
  set(CMAKE_REQUIRED_LIBRARIES)
endif()

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/vtkFloatingPointExceptionsConfigure.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/vtkFloatingPointExceptionsConfigure.h")

# Allow work arounds for lack of thread_local on odd compilers
if (NOT (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0) )
  list(APPEND VTK_OBJECT_BASE_DEFINES "VTK_HAS_THREADLOCAL")
endif ()

set_property(SOURCE vtkObjectBase.cxx
  PROPERTY
     COMPILE_DEFINITIONS ${VTK_OBJECT_BASE_DEFINES})

if(MSVC)
  set_source_files_properties(
    vtkDataArray.cxx
    PROPERTIES
      COMPILE_FLAGS /bigobj)
endif()

vtk_module_add_module(VTK::CommonCore
  HEADER_DIRECTORIES
  CLASSES           ${classes}
  TEMPLATE_CLASSES  ${template_classes}
  SOURCES           ${sources}
  TEMPLATES         ${templates}
  HEADERS           ${headers}
  NOWRAP_HEADERS    ${nowrap_headers}
  PRIVATE_HEADERS   ${private_headers}
  PRIVATE_TEMPLATES ${private_templates})
if (vtk_include_dirs)
  vtk_module_include(VTK::CommonCore
    PRIVATE
    ${vtk_include_dirs})
endif ()

vtk_module_link(VTK::CommonCore
  PUBLIC
    Threads::Threads
    ${vtk_smp_libraries}
    ${vtk_memkind_libraries}
  PRIVATE
    # Need nsl to resolve gethostbyname on SunOS-5.8 and socket also
    $<$<PLATFORM_ID:SunOS>:socket>
    $<$<PLATFORM_ID:SunOS>:nsl>
    $<$<PLATFORM_ID:Android>:log>
    )
vtk_module_compile_features(VTK::CommonCore
  PUBLIC
    cxx_std_11)

vtk_module_definitions(VTK::CommonCore
  PUBLIC
    "VTK_SMP_ENABLE_SEQUENTIAL=${vtk_smp_enable_sequential}"
    "VTK_SMP_ENABLE_STDTHREAD=${vtk_smp_enable_stdthread}"
    "VTK_SMP_ENABLE_TBB=${vtk_smp_enable_tbb}"
    "VTK_SMP_ENABLE_OPENMP=${vtk_smp_enable_openmp}"
    "VTK_SMP_DEFAULT_IMPLEMENTATION_SEQUENTIAL=$<STREQUAL:${VTK_SMP_IMPLEMENTATION_TYPE},Sequential>"
    "VTK_SMP_DEFAULT_IMPLEMENTATION_STDTHREAD=$<STREQUAL:${VTK_SMP_IMPLEMENTATION_TYPE},STDThread>"
    "VTK_SMP_DEFAULT_IMPLEMENTATION_TBB=$<STREQUAL:${VTK_SMP_IMPLEMENTATION_TYPE},TBB>"
    "VTK_SMP_DEFAULT_IMPLEMENTATION_OPENMP=$<STREQUAL:${VTK_SMP_IMPLEMENTATION_TYPE},OpenMP>")
