INCLUDE_DIRECTORIES(
   ${CMAKE_BINARY_DIR}/src/dialogs
   ${CMAKE_BINARY_DIR}/src/paintings
   ${CMAKE_BINARY_DIR}/src/plugins/attenuator
   ${CMAKE_BINARY_DIR}/src/plugins/filter
   ${CMAKE_BINARY_DIR}/src/plugins/transmission
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR} )

ADD_SUBDIRECTORY( dialogs )
ADD_SUBDIRECTORY( paintings )
ADD_SUBDIRECTORY( plugins )

SET(CANEDA_SRCS wire.cpp sidebarbrowser.cpp sidebartextbrowser.cpp
  cgraphicsitem.cpp cgraphicsscene.cpp sidebarmodel.cpp
  global.cpp units.cpp main.cpp mainwindow.cpp library.cpp
  cgraphicsview.cpp  undocommands.cpp component.cpp
  property.cpp port.cpp
  formatxmlschematic.cpp formatxmllayout.cpp formatxmlsymbol.cpp
  formatrawsimulation.cpp folderbrowser.cpp settings.cpp
  actionmanager.cpp project.cpp portsymbol.cpp
  statehandler.cpp documentviewmanager.cpp icontext.cpp
  idocument.cpp iview.cpp tabs.cpp syntaxhighlighters.cpp
  clineedit.cpp csimulationscene.cpp csimulationview.cpp
  textedit.cpp webpage.cpp xmlutilities.cpp
)

SET(CANEDA_MOC_HDRS cgraphicsscene.h cgraphicsview.h
  mainwindow.h sidebarbrowser.h sidebartextbrowser.h library.h
  component.h sidebarmodel.h portsymbol.h property.h
  folderbrowser.h project.h actionmanager.h statehandler.h
  documentviewmanager.h icontext.h idocument.h iview.h tabs.h
  syntaxhighlighters.h clineedit.h
  csimulationscene.h csimulationview.h
  textedit.h webpage.h
)

QT4_WRAP_CPP( CANEDA_MOC_SRCS ${CANEDA_MOC_HDRS} )

ADD_EXECUTABLE( caneda ${CANEDA_SRCS} ${CANEDA_MOC_SRCS} )

TARGET_LINK_LIBRARIES( caneda ${QT_LIBRARIES} ${QWT_LIBRARIES}
                       dialogs paintings attenuator filter transmission )

INSTALL(TARGETS caneda DESTINATION ${BINARYDIR})
