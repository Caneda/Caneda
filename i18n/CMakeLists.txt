# Qt translations
#
# Caneda uses the translation interface of Qt in order to internationalize
# the software.  The translation files are automatically generated.
#
# The actual Caneda translation is named 'caneda_xx.ts' where 'xx'
# specifies the locale abbreviation (e.g. 'de' for German, or 'es' for
# Spanish).  This type translation files may change as the source code
# of Caneda changes.
#
# To add a new language translation, simply add a new file name to the
# following TS_FILES rule.

SET( TS_FILES
caneda_de.ts caneda_es.ts caneda_fr.ts caneda_he.ts caneda_hu.ts
caneda_it.ts caneda_jp.ts caneda_pl.ts caneda_pt.ts caneda_ro.ts
caneda_ru.ts caneda_sv.ts caneda_tr.ts caneda_uk.ts caneda_es_AR.ts
)

QT5_ADD_TRANSLATION( QM_FILES ${TS_FILES} )
QT5_CREATE_TRANSLATION( QM_FILES ${CMAKE_SOURCE_DIR}/src/ ${TS_FILES} OPTIONS -I ${CMAKE_SOURCE_DIR}/src )

ADD_CUSTOM_TARGET( translations ALL DEPENDS ${QM_FILES} )
INSTALL( FILES ${QM_FILES} DESTINATION ${LANGUAGEDIR} )

# Prevent the generated files from being deleted during make clean
SET_DIRECTORY_PROPERTIES( PROPERTIES CLEAN_NO_CUSTOM true )
