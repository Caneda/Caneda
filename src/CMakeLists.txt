INCLUDE_DIRECTORIES(
  ${CMAKE_BINARY_DIR}/src/dialogs
  ${CMAKE_BINARY_DIR}/src/paintings
  ${CMAKE_BINARY_DIR}/src/plugins/attenuator
  ${CMAKE_BINARY_DIR}/src/plugins/filter
  ${CMAKE_BINARY_DIR}/src/plugins/transmission
)

ADD_SUBDIRECTORY( dialogs )
ADD_SUBDIRECTORY( paintings )
ADD_SUBDIRECTORY( plugins )

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
  csimulationscene.cpp csimulationview.cpp
  textedit.cpp webpage.cpp xmlutilities.cpp messagewidget.cpp
)

ADD_EXECUTABLE( caneda ${CANEDA_SRCS} )

TARGET_LINK_LIBRARIES( caneda
  ${Qt5Widgets_LIBRARIES}
  ${Qt5Svg_LIBRARIES}
  ${Qt5PrintSupport_LIBRARIES}
  ${QWT_LIBRARIES}
  dialogs
  paintings
  attenuator
  filter
  transmission
)

INSTALL( TARGETS caneda DESTINATION ${BINARYDIR} )
