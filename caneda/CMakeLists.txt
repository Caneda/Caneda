INCLUDE_DIRECTORIES(
   ${CMAKE_SOURCE_DIR}/qucs}
   ${CMAKE_BINARY_DIR}/qucs/dialogs
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR} )

ADD_SUBDIRECTORY( xmlutilities )
ADD_SUBDIRECTORY( dialogs )
ADD_SUBDIRECTORY( bitmaps )
ADD_SUBDIRECTORY( diagrams )
ADD_SUBDIRECTORY( paintings )

SET(QUCS_SRCS wire.cpp componentssidebar.cpp item.cpp
  schematicscene.cpp  sidebarmodel.cpp
  main.cpp mainwindowbase.cpp qucsmainwindow.cpp qucsview.cpp
  schematicview.cpp  undocommands.cpp fileformathandler.cpp
  svgitem.cpp property.cpp propertyitem.cpp
  propertygroup.cpp component.cpp port.cpp library.cpp
  xmlformat.cpp mnemo.cpp propertydialog.cpp
  wireline.cpp folderbrowser.cpp xmlsymbolformat.cpp settings.cpp
  actionmanager.cpp singletonmanager.cpp project.cpp
  schematicstatehandler.cpp
)

SET(QUCS_MOC_HDRS schematicscene.h schematicview.h mainwindowbase.h
  qucsmainwindow.h componentssidebar.h svgitem.h library.h
  propertyitem.h propertygroup.h component.h sidebarmodel.h
  propertydialog.h folderbrowser.h project.h
  actionmanager.h singletonmanager.h schematicstatehandler.h
)

QT4_WRAP_CPP( QUCS_MOC_SRCS ${QUCS_MOC_HDRS} )

LINK_DIRECTORIES( ${Qucs_BINARY_DIR}/qucs-tools ${Qucs_BINARY_DIR}/qucs-qterm )

ADD_EXECUTABLE( qucs ${QUCS_SRCS} ${QUCS_MOC_SRCS} )

TARGET_LINK_LIBRARIES( qucs ${QT_LIBRARIES} qucstools paintings dialogs
                       ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES}
                       diagrams xmlutilities qtermwidget)

INSTALL(TARGETS qucs DESTINATION ${BINARYDIR})
