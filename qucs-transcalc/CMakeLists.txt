INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR/qucs-transcalc} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )

ADD_SUBDIRECTORY( bitmaps )
ADD_SUBDIRECTORY( examples )

SET(QUCSTRANSCALC_SRCS helpdialog.cpp microstrip.cpp coax.cpp qucstrans.cpp transline.cpp main.cpp optionsdialog.cpp c_microstrip.cpp rectwaveguide.cpp)

SET(QUCSTRANSCALC_MOC_HDRS helpdialog.h qucstrans.h optionsdialog.h)

QT4_WRAP_CPP( QUCSTRANSCALC_MOC_SRCS ${QUCSTRANSCALC_MOC_HDRS} )

LINK_DIRECTORIES( ${Qucs_BINARY_DIR}/qucs-tools )

ADD_EXECUTABLE( qucstrans ${QUCSTRANSCALC_SRCS} ${QUCSTRANSCALC_MOC_SRCS} )

TARGET_LINK_LIBRARIES( qucstrans ${QT_LIBRARIES} qucstools )

INSTALL(TARGETS qucstrans DESTINATION ${BINARYDIR})
