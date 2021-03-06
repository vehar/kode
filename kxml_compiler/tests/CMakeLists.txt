MACRO (KODE_ADD_LOCAL_XML_PARSER _sources _current_file)
  get_filename_component( _schema ${_current_file} ABSOLUTE )
  get_filename_component( _basename ${_schema} NAME_WE )

  set( _source_cpp ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp )
  set( _source_h   ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h )

  add_custom_command(
    OUTPUT ${_source_cpp} ${_source_h}
    COMMAND "${CMAKE_CURRENT_BINARY_DIR}/../kxml_compiler"
    ARGS ${_schema} ${ARGN}
    MAIN_DEPENDENCY ${_schema}
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/../kxml_compiler"
  )

  set( ${_sources} ${${_sources}} ${_source_cpp} )
ENDMACRO (KODE_ADD_LOCAL_XML_PARSER)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

########### next target ###############

set( testfeatures_SRCS testfeatures.cpp )
kode_add_local_xml_parser( testfeatures_SRCS data/kde-features.rng )


#add_executable( testfeatures ${testfeatures_SRCS} )

#qt5_use_modules(testfeatures Core)
#qt5_use_modules(testfeatures Xml)
#qt5_use_modules(testfeatures Network)

#target_link_libraries(testfeatures Qt5::Core)
#target_link_libraries(testfeatures Qt5::Xml)
#target_link_libraries(testfeatures Qt5::Network)

#target_link_libraries( testfeatures  ${QT_QTXML_LIBRARY} )

########### next target ###############

set( testholidays_SRCS testholidays.cpp )
kode_add_local_xml_parser( testholidays_SRCS data/kde-holidays.rng )

add_executable( testholidays ${testholidays_SRCS} )
qt5_use_modules(testholidays Core)
qt5_use_modules(testholidays Xml)
qt5_use_modules(testholidays Network)

target_link_libraries(testholidays Qt5::Core)
target_link_libraries(testholidays Qt5::Xml)
target_link_libraries(testholidays Qt5::Network)

target_link_libraries( testholidays ${QT_QTXML_LIBRARY} )


########### next target ###############

set( parsertest_SRCS parserxsdtest.cpp )

add_executable( parserxsdtest ${parsertest_SRCS} )
qt5_use_modules(parserxsdtest Core)
qt5_use_modules(parserxsdtest Xml)
qt5_use_modules(parserxsdtest Network)
qt5_use_modules(parserxsdtest Test)

target_link_libraries(parserxsdtest Qt5::Core)
target_link_libraries(parserxsdtest Qt5::Xml)
target_link_libraries(parserxsdtest Qt5::Network)
target_link_libraries(parserxsdtest Qt5::Test)
target_link_libraries( parserxsdtest kschema ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} )

# next target

set( testaccounts_SRCS testaccounts.cpp )
kode_add_local_xml_parser( testaccounts_SRCS data/account.xml
  --create-crud-functions )
add_executable( testaccounts ${testaccounts_SRCS} )
qt5_use_modules(testaccounts Core)
qt5_use_modules(testaccounts Xml)
qt5_use_modules(testaccounts Network)

target_link_libraries(testaccounts Qt5::Core)
target_link_libraries(testaccounts Qt5::Xml)
target_link_libraries(testaccounts Qt5::Network)
target_link_libraries( testaccounts ${QT_QTXML_LIBRARY} )

# tests
#set( testparser_SRCS testparser.cpp )
#add_executable( testparser ${testparser_SRCS} )
#qt5_use_modules(testparser Core)
#qt5_use_modules(testparser Xml)
#qt5_use_modules(testparser Network)
#qt5_use_modules(testparser Test)

#target_link_libraries(testparser Qt5::Core)
#target_link_libraries(testparser Qt5::Xml)
#target_link_libraries(testparser Qt5::Network)
#target_link_libraries(testparser Qt5::Test)
#target_link_libraries( testparser ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} )

# tests

ADD_TEST(RunKXmlCompiler ${EXECUTABLE_OUTPUT_PATH}/../kxml_compiler --help)
#ADD_TEST(RunTestFeatures ${EXECUTABLE_OUTPUT_PATH}/testfeatures)
ADD_TEST(RunTestHolidays ${EXECUTABLE_OUTPUT_PATH}/testholidays)
ADD_TEST(RunTestAccount ${EXECUTABLE_OUTPUT_PATH}/testaccounts
  ${CMAKE_CURRENT_SOURCE_DIR}/data/account.xml)
ADD_TEST(RunTestAccount2 ${EXECUTABLE_OUTPUT_PATH}/testaccounts
  ${CMAKE_CURRENT_SOURCE_DIR}/data/account2.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/data/out_account2.xml)
#ADD_TEST(RunTestParser ${EXECUTABLE_OUTPUT_PATH}/testparser)

