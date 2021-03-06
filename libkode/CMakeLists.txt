find_package(Qt5Core REQUIRED
    Xml
    Network
)

add_definitions( ${QT_DEFINITIONS} )
include_directories( ${CMAKE_BINARY_DIR} )

add_subdirectory( tests )

set(kode_LIB_SRCS
   code.cpp
   enum.cpp
   style.cpp
   printer.cpp
   license.cpp
   file.cpp
   class.cpp
   function.cpp
   variable.cpp
   membervariable.cpp
   typedef.cpp
   statemachine.cpp
   automakefile.cpp )
   
add_library( kode SHARED ${kode_LIB_SRCS} )

target_link_libraries( kode ${QT_QTCORE_LIBRARY} )

set_target_properties( kode PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install( 
    TARGETS kode ${INSTALL_TARGETS_DEFAULT_ARGS} 
    LIBRARY DESTINATION lib  # <-- Add this line
)
