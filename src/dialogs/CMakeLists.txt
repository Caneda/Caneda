SET( DIALOGS_SRCS
  aboutdialog.cpp chartsdialog.cpp exportdialog.cpp filenewdialog.cpp
  gitmanager.cpp portsymboldialog.cpp printdialog.cpp projectfilenewdialog.cpp
  projectfileopendialog.cpp propertydialog.cpp savedocumentsdialog.cpp
  settingsdialog.cpp settingspages.cpp
)

ADD_LIBRARY( dialogs ${DIALOGS_SRCS} )

TARGET_LINK_LIBRARIES( dialogs
  Qt5::PrintSupport
  Qt5::Svg
  ${QWT_LIBRARIES}
)
