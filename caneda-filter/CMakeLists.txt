INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR/caneda-filter} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )

SET(CANEDAFILTER_SRCS lc_filter.cpp qf_filter.cpp qf_poly.cpp canedafilter.cpp helpdialog.cpp qf_cauer.cpp main.cpp)

SET(CANEDAFILTER_MOC_HDRS canedafilter.h)

QT4_WRAP_CPP( CANEDAFILTER_MOC_SRCS ${CANEDAFILTER_MOC_HDRS} )

LINK_DIRECTORIES( ${Caneda_BINARY_DIR}/caneda-tools )

ADD_EXECUTABLE( canedafilter ${CANEDAFILTER_SRCS} ${CANEDAFILTER_MOC_SRCS} )

TARGET_LINK_LIBRARIES( canedafilter ${QT_LIBRARIES} canedatools )

INSTALL(TARGETS canedafilter DESTINATION ${BINARYDIR})
