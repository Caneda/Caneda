SET( TOOLS_SRCS
  gitmanager.cpp quicklauncher.cpp quickinsert.cpp quickopen.cpp
)

ADD_LIBRARY( tools ${TOOLS_SRCS} )

TARGET_LINK_LIBRARIES( tools
  Qt5::Widgets
)
