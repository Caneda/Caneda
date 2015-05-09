SET( PAINTINGS_SRCS
  painting.cpp rectangle.cpp ellipse.cpp graphicline.cpp arrow.cpp layer.cpp
  graphictext.cpp ellipsearc.cpp styledialog.cpp graphictextdialog.cpp
)

ADD_LIBRARY( paintings ${PAINTINGS_SRCS} )
