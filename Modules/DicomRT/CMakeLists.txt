if(NOT DEFINED DCMTK_dcmrt_LIBRARY OR DCMTK_dcmrt_LIBRARY)
  mitk_create_module(
    INCLUDE_DIRS PRIVATE src/Rendering
    DEPENDS MitkSegmentation MitkSceneSerializationBase
    PACKAGE_DEPENDS PUBLIC DCMTK
  )
  add_subdirectory(test)
else()
  message("MITK DicomRT Support disabled because the DCMTK dcmrt library not found")
endif()
