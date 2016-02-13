# toolbar pictures
SET(PNGS align-horizontal-center.png c_microstrip.png document-save-as.png
         format-justify-center.png go-top.png object-locked.png terminal.png
         align-horizontal-left.png coax.png document-save.png format-justify-fill.png
         go-up.png object-rotate-left.png object-rotate-right.png text-plain.png
         align-horizontal-right.png code-context.png draw-eraser.png
         format-justify-left.png object-unlocked.png tools-wizard.png
         align-vertical-bottom.png deactiv.png draw-freehand.png format-justify-right.png
         help-contents.png pause.png transform-scale.png align-vertical-center.png
         dialog-close.png edit-copy.png format-text-bold.png image-x-generic.png pdf.png
         unmodified.png align-vertical-top.png distribute-horizontal-center.png edit-cut.png
         format-text-italic.png library.png port.png view-grid.png application-exit.png
         distribute-vertical-center.png edit-delete.png format-text-subscript.png marker.png
         preferences-other.png view-left-close.png archive-remove.png document-close.png
         edit-find.png format-text-superscript.png media-playback-start.png printer.png
         view-split-left-right.png att_bridge.png document-new.png edit-paste.png
         format-text-underline.png microstrip.png project-new.png view-split-top-bottom.png
         att_pi.png document-open.png edit-redo.png formula.png qt.png wire.png att_tee.png
         document-preview.png edit-select.png go-bottom.png next.png rectwaveguide.png
         zoom-fit-best.png caneda.png document-print.png edit-undo.png go-home.png
         select-rectangular.png zoom-in.png chronometer.png document-properties.png eps.png
         go-next.png object-flip-horizontal.png stop.png zoom-original.png document-save-all.png
         folder-new.png go-previous.png object-flip-vertical.png system-switch-user.png
         zoom-out.png list-remove.png list-add.png edit-clear-locationbar-rtl.png
)

SET(SVGS arrow.svg ellipsearc.svg ellipse.svg line.svg portsymbol.svg rectangle.svg text.svg ground.svg)

# application pictures
SET(ICONS big.button.caneda.xpm big.caneda.xpm tiny.button.caneda.xpm tiny.caneda.xpm canedaicon.ico )

INSTALL(FILES ${PNGS} DESTINATION ${BITMAPDIR})
INSTALL(FILES ${SVGS} DESTINATION ${BITMAPDIR})
INSTALL(FILES ${ICONS} DESTINATION ${BITMAPDIR})
