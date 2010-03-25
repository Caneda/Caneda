INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR/caneda-lib} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )

ADD_SUBDIRECTORY( library )

SET(CANEDALIB_SRCS searchdialog.cpp main.cpp canedalib.cpp displaydialog.cpp librarydialog.cpp symbolwidget.cpp)

SET(CANEDALIB_MOC_HDRS displaydialog.h librarydialog.h symbolwidget.h searchdialog.h canedalib.h)

QT4_WRAP_CPP( CANEDALIB_MOC_SRCS ${CANEDALIB_MOC_HDRS} )

LINK_DIRECTORIES( ${Caneda_BINARY_DIR}/caneda-tools )

ADD_EXECUTABLE( canedalib ${CANEDALIB_SRCS} ${CANEDALIB_MOC_SRCS} )

TARGET_LINK_LIBRARIES( canedalib ${QT_LIBRARIES} canedatools )

INSTALL(TARGETS canedalib DESTINATION ${BINARYDIR})
