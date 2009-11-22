SET(LIBS  Bridges.lib Diodes.lib LEDs.lib OpAmps.lib Transistors.lib JFETs.lib MOSFETs.lib Substrates.lib Z-Diodes.lib)

INSTALL(FILES ${LIBS} DESTINATION ${LIBRARYDIR})
