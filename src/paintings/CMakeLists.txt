SET( PAINTINGS_SRCS
  painting.cpp rectangle.cpp ellipse.cpp graphicline.cpp arrow.cpp
  graphictext.cpp ellipsearc.cpp styledialog.cpp graphictextdialog.cpp
)

qt6_wrap_ui( PAINTINGS_UIC
  styledialog.ui
)

ADD_LIBRARY( paintings ${PAINTINGS_SRCS} ${PAINTINGS_UIC} )

TARGET_LINK_LIBRARIES( paintings
  Qt6::Widgets
)
