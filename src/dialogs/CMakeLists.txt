SET(DIALOGS_SRCS aboutdialog.cpp settingsdialog.cpp settingspages.cpp exportdialog.cpp
    printdialog.cpp projectfilenewdialog.cpp savedocumentsdialog.cpp
    projectfileopendialog.cpp gitmanager.cpp propertydialog.cpp filenewdialog.cpp
    portsymboldialog.cpp
)

SET(DIALOGS_MOC_HDRS aboutdialog.h settingsdialog.h settingspages.h exportdialog.h
    printdialog.h projectfilenewdialog.h savedocumentsdialog.h projectfileopendialog.h
    gitmanager.h propertydialog.h filenewdialog.h portsymboldialog.h
)

SET(DIALOGS_UIS savedocumentsdialog.ui gitmanager.ui printdialog.ui
    projectfilenewdialog.ui projectfileopendialog.ui propertydialog.ui
    exportdialog.ui filenewdialog.ui portsymboldialog.ui
)

QT4_WRAP_UI( DIALOGS_UI_HDRS ${DIALOGS_UIS})

QT4_WRAP_CPP( DIALOGS_MOC_SRCS ${DIALOGS_MOC_HDRS} )

ADD_LIBRARY(dialogs ${DIALOGS_SRCS} ${DIALOGS_MOC_SRCS} ${DIALOGS_UI_HDRS})
