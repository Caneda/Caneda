SET( FILTER_SRCS
  filterdialog.cpp lc_filter.cpp qf_cauer.cpp qf_filter.cpp
  filterdialog.cpp qf_poly.cpp
)

ADD_LIBRARY( filter ${FILTER_SRCS} )
