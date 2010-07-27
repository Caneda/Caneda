INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )

SET(DIALOGS_SRCS aboutdialog.cpp settingsdialog.cpp settingspages.cpp exportdialog.cpp
    printdialog.cpp projectfilenewdialog.cpp savedocumentsdialog.cpp
    projectfileopendialog.cpp gitmanager.cpp propertydialog.cpp librarymanager.cpp
)

SET(DIALOGS_MOC_HDRS aboutdialog.h settingsdialog.h settingspages.h exportdialog.h
    printdialog.h projectfilenewdialog.h savedocumentsdialog.h projectfileopendialog.h
    gitmanager.h propertydialog.h librarymanager.h
)

SET(DIALOGS_UIS savedocumentsdialog.ui gitmanager.ui printdialog.ui
    projectfilenewdialog.ui projectfileopendialog.ui propertydialog.ui
    librarymanager.ui
)

QT4_WRAP_UI( DIALOGS_UI_HDRS ${DIALOGS_UIS})

QT4_WRAP_CPP( DIALOGS_MOC_SRCS ${DIALOGS_MOC_HDRS} )

ADD_LIBRARY(dialogs ${DIALOGS_SRCS} ${DIALOGS_MOC_SRCS} ${DIALOGS_UI_HDRS})
