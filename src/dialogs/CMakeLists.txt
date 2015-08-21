SET( DIALOGS_SRCS
  aboutdialog.cpp settingsdialog.cpp settingspages.cpp exportdialog.cpp
  printdialog.cpp projectfilenewdialog.cpp savedocumentsdialog.cpp
  projectfileopendialog.cpp gitmanager.cpp propertydialog.cpp filenewdialog.cpp
  portsymboldialog.cpp simulationdialog.cpp
)

ADD_LIBRARY( dialogs ${DIALOGS_SRCS} )
