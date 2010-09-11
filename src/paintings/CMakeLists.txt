SET(PAINTINGS_SRCS painting.cpp rectangle.cpp ellipse.cpp graphicline.cpp arrow.cpp layer.cpp
graphictext.cpp ellipsearc.cpp id_text.cpp portsymbol.cpp styledialog.cpp graphictextdialog.cpp)

SET(PAINTINGS_MOC_HDRS styledialog.h graphictextdialog.h)

SET(PAINTINGS_UIS styledialog.ui)

QT4_WRAP_UI( PAINTINGS_UI_HDRS ${PAINTINGS_UIS})

QT4_WRAP_CPP( PAINTINGS_MOC_SRCS ${PAINTINGS_MOC_HDRS} )

ADD_LIBRARY(paintings ${PAINTINGS_SRCS} ${PAINTINGS_MOC_SRCS} ${PAINTINGS_UI_HDRS})
