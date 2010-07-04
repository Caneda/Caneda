# toolbar pictures
SET(PNGS  align-horizontal-center.png chronometer.png dialog-close.png draw-eraser.png forward.png object-flip-horizontal.png qt.png text-plain.png
align-horizontal-left.png distribute-horizontal-center.png draw-freehand.png go-bottom.png object-flip-vertical.png select-rectangular.png textright.png
align-horizontal-right.png distribute-vertical-center.png edit-copy.png go-top.png object-locked.png stop.png textunder.png
align-vertical-bottom.png document-close.png edit-delete.png ground.png object-rotate-left.png sub.png tools-wizard.png
align-vertical-center.png document-new.png edit-find.png help-contents.png object-unlocked.png super.png wire.png
align-vertical-top.png document-open.png edit-redo.png home.png pause.png system-switch-user.png zoom-fit-best.png
application-exit.png document-preview.png edit-select.png image-x-generic.png pdf.png terminal.png zoom-in.png
back.png document-print.png edit-undo.png edit-cut.png edit-paste.png library.png port.png textbold.png zoom-original.png
caneda.png document-properties.png eps.png marker.png preferences-other.png textcenter.png zoom-out.png
clearFilterText.png document-save-all.png filedelete.png media-playback-start.png previous.png textitalic.png
code-context.png document-save-as.png foldernew.png next.png printer.png textjustify.png
deactiv.png document-save.png formula.png nodename.png project-new.png textleft.png
modified.png unmodified.png view-grid.png view-left-close.png view-split-left-right.png view-split-top-bottom.png)

SET(SVGS arrow.svg ellipsearc.svg ellipse.svg line.svg rectangle.svg text.svg splash.jpg)

# application pictures
SET(ICONS  big.button.caneda.xpm big.caneda.xpm tiny.button.caneda.xpm tiny.caneda.xpm )

INSTALL(FILES ${PNGS} DESTINATION ${BITMAPDIR})
INSTALL(FILES ${ICONS} DESTINATION ${BITMAPDIR})
