SET( DIALOGS_SRCS
  aboutdialog.cpp chartsdialog.cpp exportdialog.cpp filenewdialog.cpp
  messagewidget.cpp portsymboldialog.cpp printdialog.cpp
  projectfilenewdialog.cpp projectfileopendialog.cpp propertydialog.cpp
  savedocumentsdialog.cpp settingsdialog.cpp shortcutsdialog.cpp
)

qt5_wrap_ui( DIALOGS_UIC
  aboutdialog.ui chartsdialog.ui exportdialog.ui filenewdialog.ui
  portsymboldialog.ui printdialog.ui projectfilenewdialog.ui
  projectfileopendialog.ui propertydialog.ui savedocumentsdialog.ui
  settingsdialog.ui shortcutsdialog.ui
)

ADD_LIBRARY( dialogs ${DIALOGS_SRCS} ${DIALOGS_UIC} )

TARGET_LINK_LIBRARIES( dialogs
  Qt5::PrintSupport
  Qt5::Svg
  ${QWT_LIBRARIES}
)
