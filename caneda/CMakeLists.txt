INCLUDE_DIRECTORIES(
   ${CMAKE_SOURCE_DIR}/caneda}
   ${CMAKE_BINARY_DIR}/caneda/dialogs
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR} )

ADD_SUBDIRECTORY( xmlutilities )
ADD_SUBDIRECTORY( dialogs )
ADD_SUBDIRECTORY( bitmaps )
ADD_SUBDIRECTORY( diagrams )
ADD_SUBDIRECTORY( paintings )

SET(CANEDA_SRCS wire.cpp componentssidebar.cpp item.cpp
  schematicscene.cpp sidebarmodel.cpp
  main.cpp mainwindow.cpp
  schematicwidget.cpp  undocommands.cpp fileformathandler.cpp
  svgitem.cpp property.cpp propertyitem.cpp
  propertygroup.cpp component.cpp port.cpp library.cpp
  xmlformat.cpp mnemo.cpp propertydialog.cpp
  wireline.cpp folderbrowser.cpp xmlsymbolformat.cpp settings.cpp
  actionmanager.cpp singletonowner.cpp project.cpp
  schematicstatehandler.cpp documentviewmanager.cpp icontext.cpp
  idocument.cpp iview.cpp tabs.cpp
  schematicdocument.cpp schematiccontext.cpp schematicview.cpp
  textdocument.cpp textcontext.cpp textview.cpp textedit.cpp
)

SET(CANEDA_MOC_HDRS schematicscene.h schematicwidget.h
  mainwindow.h componentssidebar.h svgitem.h library.h
  propertyitem.h propertygroup.h component.h sidebarmodel.h
  propertydialog.h folderbrowser.h project.h
  actionmanager.h singletonowner.h schematicstatehandler.h
  documentviewmanager.h icontext.h idocument.h iview.h tabs.h
  schematicdocument.h schematiccontext.h schematicview.h
  textdocument.h textcontext.h textview.h textedit.h
)

QT4_WRAP_CPP( CANEDA_MOC_SRCS ${CANEDA_MOC_HDRS} )

LINK_DIRECTORIES( ${Caneda_BINARY_DIR}/caneda-tools ${Caneda_BINARY_DIR}/caneda-qterm )

ADD_EXECUTABLE( caneda ${CANEDA_SRCS} ${CANEDA_MOC_SRCS} )

TARGET_LINK_LIBRARIES( caneda ${QT_LIBRARIES} canedatools paintings dialogs
                       ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES}
                       diagrams xmlutilities qtermwidget)

INSTALL(TARGETS caneda DESTINATION ${BINARYDIR})
