INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR/qucs-filter} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )

SET(QUCSFILTER_SRCS lc_filter.cpp qf_filter.cpp qf_poly.cpp qucsfilter.cpp helpdialog.cpp qf_cauer.cpp main.cpp)

SET(QUCSFILTER_MOC_HDRS qucsfilter.h)

QT4_WRAP_CPP( QUCSFILTER_MOC_SRCS ${QUCSFILTER_MOC_HDRS} )

LINK_DIRECTORIES( ${Qucs_BINARY_DIR}/qucs-tools )

ADD_EXECUTABLE( qucsfilter ${QUCSFILTER_SRCS} ${QUCSFILTER_MOC_SRCS} )

TARGET_LINK_LIBRARIES( qucsfilter ${QT_LIBRARIES} qucstools )

INSTALL(TARGETS qucsfilter DESTINATION ${BINARYDIR})
