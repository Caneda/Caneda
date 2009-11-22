INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR/qucs-lib} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )

ADD_SUBDIRECTORY( library )

SET(QUCSLIB_SRCS searchdialog.cpp main.cpp qucslib.cpp displaydialog.cpp librarydialog.cpp symbolwidget.cpp)

SET(QUCSLIB_MOC_HDRS displaydialog.h librarydialog.h symbolwidget.h searchdialog.h qucslib.h)

QT4_WRAP_CPP( QUCSLIB_MOC_SRCS ${QUCSLIB_MOC_HDRS} )

LINK_DIRECTORIES( ${Qucs_BINARY_DIR}/qucs-tools )

ADD_EXECUTABLE( qucslib ${QUCSLIB_SRCS} ${QUCSLIB_MOC_SRCS} )

TARGET_LINK_LIBRARIES( qucslib ${QT_LIBRARIES} qucstools )

INSTALL(TARGETS qucslib DESTINATION ${BINARYDIR})
