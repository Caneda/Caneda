INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )

SET(TRANSMISSION_SRCS transmissiondialog.cpp
microstrip.cpp coax.cpp transline.cpp c_microstrip.cpp rectwaveguide.cpp
propertygrid.cpp
)

SET(TRANSMISSION_MOC_HDRS transmissiondialog.h propertygrid.h
)

SET(TRANSMISSION_UIS transmissiondialog.ui
)

QT4_WRAP_UI( TRANSMISSION_UI_HDRS ${TRANSMISSION_UIS})

QT4_WRAP_CPP( TRANSMISSION_MOC_SRCS ${TRANSMISSION_MOC_HDRS} )

ADD_LIBRARY(transmission ${TRANSMISSION_SRCS} ${TRANSMISSION_MOC_SRCS} ${TRANSMISSION_UI_HDRS})
