INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR/qucs-qterm} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )

SET(QTermWidget_MOC_HEADERS  ScreenWindow.h Emulation.h Vt102Emulation.h TerminalDisplay.h Filter.h Pty.h k3process.h k3processcontroller.h Session.h qtermwidget.h)

SET(QTermWidget_SOURCES  TerminalCharacterDecoder.cpp KeyboardTranslator.cpp Screen.cpp History.cpp BlockArray.cpp konsole_wcwidth.cpp ScreenWindow.cpp Emulation.cpp Vt102Emulation.cpp TerminalDisplay.cpp Filter.cpp Pty.cpp kpty.cpp k3process.cpp k3processcontroller.cpp Session.cpp ShellCommand.cpp qtermwidget.cpp)

ADD_DEFINITIONS( -DHAVE_CONFIG_H )

#Library was tested both with HAVE_POSIX_OPENPT and HAVE_GETPT precompiler directives, defined here. You should select the correct variant for your system.
ADD_DEFINITIONS( -DHAVE_POSIX_OPENPT )
#or ADD_DEFINITIONS( -DHAVE_GETPT )

QT4_WRAP_CPP( QTermWidget_MOC_SOURCES ${QTermWidget_MOC_HEADERS} )

ADD_LIBRARY( qtermwidget ${QTermWidget_SOURCES} ${QTermWidget_MOC_SOURCES} )

