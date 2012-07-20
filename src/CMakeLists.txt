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
ADD_SUBDIRECTORY( xmlutilities )

SET(CANEDA_SRCS wire.cpp sidebarbrowser.cpp sidebartextbrowser.cpp
  cgraphicsitem.cpp cgraphicsscene.cpp sidebarmodel.cpp
  global.cpp units.cpp main.cpp mainwindow.cpp library.cpp
  cgraphicsview.cpp  undocommands.cpp svgitem.cpp property.cpp
  propertyitem.cpp propertygroup.cpp component.cpp port.cpp node.cpp
  xmlschematic.cpp xmllayout.cpp xmlsymbol.cpp xmlsimulation.cpp
  folderbrowser.cpp settings.cpp
  actionmanager.cpp singletonowner.cpp project.cpp
  statehandler.cpp documentviewmanager.cpp icontext.cpp
  idocument.cpp iview.cpp tabs.cpp
  syntaxhighlighters.cpp clineedit.cpp
  schematicdocument.cpp schematiccontext.cpp schematicview.cpp
  simulationdocument.cpp simulationcontext.cpp simulationview.cpp simulationscene.cpp
  symboldocument.cpp symbolcontext.cpp symbolview.cpp
  layoutdocument.cpp layoutcontext.cpp layoutview.cpp
  textdocument.cpp textcontext.cpp textview.cpp textedit.cpp
  webdocument.cpp webcontext.cpp webview.cpp webpage.cpp

)

SET(CANEDA_MOC_HDRS cgraphicsscene.h cgraphicsview.h
  mainwindow.h sidebarbrowser.h sidebartextbrowser.h library.h
  propertyitem.h propertygroup.h component.h sidebarmodel.h
  folderbrowser.h project.h actionmanager.h singletonowner.h statehandler.h
  documentviewmanager.h icontext.h idocument.h iview.h tabs.h
  syntaxhighlighters.h clineedit.h
  schematicdocument.h schematiccontext.h schematicview.h
  simulationdocument.h simulationcontext.h simulationview.h simulationscene.h
  symboldocument.h symbolcontext.h symbolview.h
  layoutdocument.h layoutcontext.h layoutview.h
  textdocument.h textcontext.h textview.h textedit.h
  webdocument.h webcontext.h webview.h webpage.h
)

QT4_WRAP_CPP( CANEDA_MOC_SRCS ${CANEDA_MOC_HDRS} )

ADD_EXECUTABLE( caneda ${CANEDA_SRCS} ${CANEDA_MOC_SRCS} )

TARGET_LINK_LIBRARIES( caneda ${QT_LIBRARIES} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES} ${QWT_LIBRARIES}
                       dialogs paintings xmlutilities
                       attenuator filter transmission)

INSTALL(TARGETS caneda DESTINATION ${BINARYDIR})
