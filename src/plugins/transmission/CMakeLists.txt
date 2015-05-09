SET( TRANSMISSION_SRCS
  transmissiondialog.cpp propertygrid.cpp
  microstrip.cpp coax.cpp transline.cpp c_microstrip.cpp rectwaveguide.cpp
)

ADD_LIBRARY( transmission ${TRANSMISSION_SRCS} )
