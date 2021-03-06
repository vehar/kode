set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${kode_SOURCE_DIR}/kxforms )

########### next target ###############

set(kxformstest_SRCS
   kxformstest.cpp
   ../manager.cpp
   ../form.cpp
   ../formcreator.cpp
   ../hints.cpp
   ../list.cpp
   ../listmodel.cpp
   ../input.cpp
   ../formgui.cpp
   ../remotefile.cpp
   ../xformscommon.cpp
   ../xmlbuilder.cpp
   ../guielement.cpp
   ../kresult.cpp
   ../guihandler.cpp
   ../guihandlerflat.cpp
   ../guihandlerdialogs.cpp
   ../reference.cpp
   ../textarea.cpp
   ../select1.cpp
   ../section.cpp
   ../dispatcher.cpp
   ../select.cpp
   ../listproxymodel.cpp
   ../layout.cpp
   ../editor/editorwidget.cpp
   ../editor/editor.cpp
   ../editor/editoraction.cpp
   ../editor/appearanceaction.cpp
   ../editor/listaction.cpp
   ../editor/positionaction.cpp
   ../editor/layoutstyleaction.cpp
   ../editor/groupaction.cpp
   ../editor/changelabelaction.cpp
   ../editor/readonlyaction.cpp
   ../editor/inputtypeaction.cpp
   ../editor/globalsettingsdlg.cpp )


kde4_add_kcfg_files( kxformstest_SRCS ../prefs.kcfgc )


kde4_add_executable( kxformstest TEST ${kxformstest_SRCS} )

target_link_libraries( kxformstest kschema ${QT_QTTEST_LIBRARY} ${KDE4_KDE3SUPPORT_LIBS} )

########### next target ###############

set( testreferences_SRCS testreferences.cpp ../reference.cpp )


kde4_add_executable( testreferences TEST ${testreferences_SRCS} )

target_link_libraries( testreferences ${KDE4_KDECORE_LIBS} ${QT_QTXML_LIBRARY} )
