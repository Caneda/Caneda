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
  wire.cpp sidebarbrowser.cpp sidebartextbrowser.cpp
  cgraphicsitem.cpp cgraphicsscene.cpp sidebarwebbrowser.cpp
  global.cpp units.cpp main.cpp mainwindow.cpp library.cpp
  cgraphicsview.cpp  undocommands.cpp component.cpp
  property.cpp port.cpp fileformats.cpp fileimport.cpp
  fileexport.cpp folderbrowser.cpp settings.cpp
  actionmanager.cpp project.cpp portsymbol.cpp
  statehandler.cpp documentviewmanager.cpp icontext.cpp
  idocument.cpp iview.cpp tabs.cpp syntaxhighlighters.cpp
  csimulationscene.cpp csimulationview.cpp csimulationitem.cpp
  textedit.cpp webpage.cpp xmlutilities.cpp messagewidget.cpp
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
