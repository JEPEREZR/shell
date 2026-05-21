find_package(Qt6 REQUIRED COMPONENTS LinguistTools)

set(TS_FILES
    caelestia_es.ts
    caelestia_es_CL.ts
)

set(qm_files "")
foreach(ts_file IN LISTS TS_FILES)
    get_filename_component(qm_name "${ts_file}" NAME_WE)
    set(qm_path "${CMAKE_CURRENT_BINARY_DIR}/${qm_name}.qm")
    add_custom_command(
        OUTPUT "${qm_path}"
        COMMAND Qt6::lrelease "${CMAKE_CURRENT_SOURCE_DIR}/${ts_file}" -qm "${qm_path}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${ts_file}"
        VERBATIM
    )
    list(APPEND qm_files "${qm_path}")
endforeach()

add_custom_target(caelestia-translations ALL DEPENDS ${qm_files})

install(FILES ${qm_files}
    DESTINATION "usr/share/caelestia-shell/translations"
)
