INCLUDE_DIRECTORIES(
   ${CMAKE_BINARY_DIR}/src/dialogs
   ${CMAKE_BINARY_DIR}/src/paintings
   ${CMAKE_BINARY_DIR}/src/tools/attenuator
   ${CMAKE_BINARY_DIR}/src/tools/filter
   ${CMAKE_BINARY_DIR}/src/tools/transmission
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR} )

ADD_SUBDIRECTORY( diagrams )
ADD_SUBDIRECTORY( dialogs )
ADD_SUBDIRECTORY( paintings )
ADD_SUBDIRECTORY( tools )
ADD_SUBDIRECTORY( xmlutilities )

SET(CANEDA_SRCS wire.cpp componentssidebar.cpp item.cpp
  schematicscene.cpp sidebarmodel.cpp
  global.cpp units.cpp main.cpp mainwindow.cpp
  schematicwidget.cpp  undocommands.cpp fileformathandler.cpp
  svgitem.cpp property.cpp propertyitem.cpp
  propertygroup.cpp component.cpp port.cpp library.cpp
  xmlformat.cpp
  wireline.cpp folderbrowser.cpp xmlsymbolformat.cpp settings.cpp
  actionmanager.cpp singletonowner.cpp project.cpp
  schematicstatehandler.cpp documentviewmanager.cpp icontext.cpp
  idocument.cpp iview.cpp tabs.cpp
  schematicdocument.cpp schematiccontext.cpp schematicview.cpp
  syntaxhighlighters.cpp
  textdocument.cpp textcontext.cpp textview.cpp textedit.cpp
  webdocument.cpp webcontext.cpp webview.cpp webpage.cpp
)

SET(CANEDA_MOC_HDRS schematicscene.h schematicwidget.h
  mainwindow.h componentssidebar.h svgitem.h library.h
  propertyitem.h propertygroup.h component.h sidebarmodel.h
  folderbrowser.h project.h
  actionmanager.h singletonowner.h schematicstatehandler.h
  documentviewmanager.h icontext.h idocument.h iview.h tabs.h
  schematicdocument.h schematiccontext.h schematicview.h
  syntaxhighlighters.h
  textdocument.h textcontext.h textview.h textedit.h
  webdocument.h webcontext.h webview.h webpage.h
)

QT4_WRAP_CPP( CANEDA_MOC_SRCS ${CANEDA_MOC_HDRS} )

LINK_DIRECTORIES( ${Caneda_BINARY_DIR}/src/tools/qtermwidget )

ADD_EXECUTABLE( caneda ${CANEDA_SRCS} ${CANEDA_MOC_SRCS} )

TARGET_LINK_LIBRARIES( caneda ${QT_LIBRARIES} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES}
                       diagrams dialogs paintings xmlutilities
                       attenuator filter qtermwidget transmission)

INSTALL(TARGETS caneda DESTINATION ${BINARYDIR})
