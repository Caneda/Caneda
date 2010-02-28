INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )

SET(DIALOGS_SRCS aboutqucs.cpp settingsdialog.cpp settingspages.cpp exportdialog.cpp printdialog.cpp addtoprojectdialog.cpp savedocumentsdialog.cpp)
SET(DIALOGS_MOC_HDRS aboutqucs.h settingsdialog.h settingspages.h exportdialog.h printdialog.h addtoprojectdialog.h savedocumentsdialog.h)

SET(DIALOGS_UIS savedocumentsdialog.ui)
QT4_WRAP_UI( DIALOGS_UI_HDRS ${DIALOGS_UIS})

QT4_WRAP_CPP( DIALOGS_MOC_SRCS ${DIALOGS_MOC_HDRS} )

ADD_LIBRARY(dialogs ${DIALOGS_SRCS} ${DIALOGS_MOC_SRCS} ${DIALOGS_UI_HDRS})
