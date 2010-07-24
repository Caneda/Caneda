INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR/caneda-transcalc} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )

ADD_SUBDIRECTORY( bitmaps )
ADD_SUBDIRECTORY( examples )

SET(CANEDATRANSCALC_SRCS helpdialog.cpp microstrip.cpp coax.cpp canedatrans.cpp
transline.cpp main.cpp optionsdialog.cpp c_microstrip.cpp rectwaveguide.cpp
propertygrid.cpp units.cpp
)

SET(CANEDATRANSCALC_MOC_HDRS helpdialog.h canedatrans.h optionsdialog.h
propertygrid.h
)

QT4_WRAP_CPP( CANEDATRANSCALC_MOC_SRCS ${CANEDATRANSCALC_MOC_HDRS} )

LINK_DIRECTORIES( ${Caneda_BINARY_DIR}/caneda-tools )

ADD_EXECUTABLE( canedatrans ${CANEDATRANSCALC_SRCS} ${CANEDATRANSCALC_MOC_SRCS} )

TARGET_LINK_LIBRARIES( canedatrans ${QT_LIBRARIES} canedatools )

INSTALL(TARGETS canedatrans DESTINATION ${BINARYDIR})
