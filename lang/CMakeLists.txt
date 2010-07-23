# Language files. In future, these must generate correct translation files through linguist tools
# This is just a dummy file, to test langs instalation.
SET(LANG caneda_de.ts caneda_it.ts caneda_ru.ts qtgeneric_de.ts qtgeneric_it.ts qtgeneric_sv.ts
caneda_es.ts caneda_jp.ts caneda_sv.ts qtgeneric_es.ts qtgeneric_jp.ts qtgeneric_tr.ts
caneda_fr.ts caneda_pl.ts caneda_tr.ts qtgeneric_fr.ts qtgeneric_pl.ts qtgeneric_uk.ts
caneda_he.ts caneda_pt.ts caneda_uk.ts qtgeneric_he.ts qtgeneric_ro.ts qtgeneric_untranslated.ts
caneda_hu.ts caneda_ro.ts qtgeneric_hu.ts qtgeneric_ru.ts README.translations
)

INSTALL(FILES ${LANG} DESTINATION ${LANGUAGEDIR})

