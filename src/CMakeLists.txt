INCLUDE_DIRECTORIES(
  ${CMAKE_BINARY_DIR}

  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/dialogs
  ${CMAKE_SOURCE_DIR}/src/paintings

  ${CMAKE_BINARY_DIR}/src/dialogs
  ${CMAKE_BINARY_DIR}/src/paintings

  ${QWT_INCLUDE_DIR}
)

ADD_SUBDIRECTORY( dialogs )
ADD_SUBDIRECTORY( paintings )

SET( CANEDA_SRCS
  actionmanager.cpp chartitem.cpp chartscene.cpp chartview.cpp component.cpp
  documentviewmanager.cpp fileformats.cpp folderbrowser.cpp global.cpp
  graphicsitem.cpp graphicsscene.cpp graphicsview.cpp icontext.cpp
  idocument.cpp iview.cpp library.cpp main.cpp mainwindow.cpp messagewidget.cpp
  port.cpp portsymbol.cpp project.cpp property.cpp quicklauncher.cpp
  quickinsert.cpp settings.cpp sidebarchartsbrowser.cpp sidebaritemsbrowser.cpp
  sidebartextbrowser.cpp statehandler.cpp syntaxhighlighters.cpp tabs.cpp
  textedit.cpp undocommands.cpp wire.cpp xmlutilities.cpp
)

ADD_EXECUTABLE( caneda ${CANEDA_SRCS} )

TARGET_LINK_LIBRARIES( caneda
  Qt5::Widgets
  Qt5::Svg
  Qt5::PrintSupport
  ${QWT_LIBRARIES}
  dialogs
  paintings
)

INSTALL( TARGETS caneda DESTINATION ${BINARYDIR} )
