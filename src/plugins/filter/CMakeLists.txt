INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )

SET(FILTER_SRCS filterdialog.cpp lc_filter.cpp  qf_cauer.cpp  qf_filter.cpp
                filterdialog.cpp qf_poly.cpp
)

SET(FILTER_MOC_HDRS filterdialog.h
)

SET(FILTER_UIS filterdialog.ui
)

QT5_WRAP_UI( FILTER_UI_HDRS ${FILTER_UIS})

QT5_WRAP_CPP( FILTER_MOC_SRCS ${FILTER_MOC_HDRS} )

ADD_LIBRARY(filter ${FILTER_SRCS} ${FILTER_MOC_SRCS} ${FILTER_UI_HDRS})
