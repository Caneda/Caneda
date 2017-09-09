SET( TOOLS_SRCS
  gitmanager.cpp quicklauncher.cpp quickinsert.cpp quickopen.cpp
)

qt5_wrap_ui( TOOLS_UIC
  gitmanager.ui
)

ADD_LIBRARY( tools ${TOOLS_SRCS} ${TOOLS_UIC} )

TARGET_LINK_LIBRARIES( tools
  Qt5::Widgets
)
