INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )

SET(QTERM_MOC_HDRS  ScreenWindow.h Emulation.h Vt102Emulation.h
TerminalDisplay.h Filter.h Pty.h k3process.h k3processcontroller.h Session.h
qtermwidget.h
)

SET(QTERM_SRCS  TerminalCharacterDecoder.cpp KeyboardTranslator.cpp
Screen.cpp History.cpp BlockArray.cpp konsole_wcwidth.cpp ScreenWindow.cpp
Emulation.cpp Vt102Emulation.cpp TerminalDisplay.cpp Filter.cpp Pty.cpp kpty.cpp
k3process.cpp k3processcontroller.cpp Session.cpp ShellCommand.cpp qtermwidget.cpp
)

ADD_DEFINITIONS( -DHAVE_CONFIG_H )

#Library was tested both with HAVE_POSIX_OPENPT and HAVE_GETPT precompiler directives,
#defined here. You should select the correct variant for your system.
ADD_DEFINITIONS( -DHAVE_POSIX_OPENPT )
#or ADD_DEFINITIONS( -DHAVE_GETPT )

QT4_WRAP_CPP( QTERM_MOC_SRCS ${QTERM_MOC_HDRS} )

ADD_LIBRARY( qtermwidget ${QTERM_SRCS} ${QTERM_MOC_SRCS} )

