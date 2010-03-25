# toolbar pictures
SET(PNGS  align-horizontal-center.png configure.png filenew.png mirror.png select-all.png textunder.png align-horizontal-left.png deactiv.png fileopen.png mirrory.png sub.png align-horizontal-right.png document-edit.png fileprint.png next.png super.png align-vertical-bottom.png document-preview.png filesaveall.png nodename.png switch-view.png tools-wizard.png align-vertical-center.png editcopy.png filesaveas.png pointer.png symbol-edit.png top.png align-vertical-top.png editcut.png filesave.png port.png textbold.png undo.png application-exit.png editdelete.png forward.png previous.png textcenter.png vhdl-code.png arrow.svg editfind.png ground.png       project-new.png viewmag1.png back.png editpaste.png help.png qt.png textitalic.png viewmagfit.png ellipsearc.svg home.png qucs.png textjustify.png viewmag-.png ellipse.svg library.png textleft.png viewmag+.png bottom.png equation.png line.svg rectangle.svg textnew.png wire.png clearFilterText.png export-image.png redo.png textright.png fileclose.png marker.png rotate_ccw.png text.svg pause.png start.png stop.png object-locked.png object-unlocked.png printer.png eps.png pdf.png foldernew.png filedelete.png project-close.png console.png cursordelete.png splash.jpg)

# application pictures
SET(ICONS  big.button.qucs.xpm big.qucs.xpm tiny.button.qucs.xpm tiny.qucs.xpm )

INSTALL(FILES ${PNGS} DESTINATION ${BITMAPDIR})
INSTALL(FILES ${ICONS} DESTINATION ${BITMAPDIR})
