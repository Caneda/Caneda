qqfile(GLOB TS_FILES ${CMAKE_SOURCE_DIR}/i18n/*.ts)
list(REMOVE_ITEM TS_FILES "${CMAKE_SOURCE_DIR}/i18n/qtgeneric_untranslated.ts")

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR}/i18n/ ${TS_FILES} OPTIONS -I ${CMAKE_SOURCE_DIR}/src)

add_custom_target(translations ALL DEPENDS ${QM_FILES})
install(FILES ${QM_FILES} DESTINATION ${LANGUAGEDIR})
