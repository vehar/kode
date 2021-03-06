########### next target ###############
set( kxmlcommon_LIB_SRCS
   fileprovider.cpp
   messagehandler.cpp
   nsmanager.cpp
   parsercontext.cpp
   qname.cpp
)

find_package(Qt5Core REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Network REQUIRED)

add_library( kxmlcommon SHARED ${kxmlcommon_LIB_SRCS} )
qt5_use_modules(kxmlcommon Core)
qt5_use_modules(kxmlcommon Xml)
qt5_use_modules(kxmlcommon Network)

target_link_libraries(kxmlcommon Qt5::Core)
target_link_libraries(kxmlcommon Qt5::Xml)
target_link_libraries(kxmlcommon Qt5::Network)

target_link_libraries( kxmlcommon  ${QT_QTXML_LIBRARY} )
target_link_libraries( kxmlcommon  LINK_INTERFACE_LIBRARIES ${QT_QTXML_LIBRARY} )

set_target_properties( kxmlcommon PROPERTIES VERSION ${GENERIC_LIB_VERSION}
                                            SOVERSION ${GENERIC_LIB_SOVERSION} )

install( 
    TARGETS kxmlcommon  ${INSTALL_TARGETS_DEFAULT_ARGS} 
    LIBRARY DESTINATION lib  # <-- Add this line
)
