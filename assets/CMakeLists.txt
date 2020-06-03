file(GLOB ARYIBI_SHADER_ASSETS *.frag *.vert)
set(ARYIBI_BINARY_ASSETS "")
foreach(ARYIBI_SHADER ${ARYIBI_SHADER_ASSETS})
    file(RELATIVE_PATH ASSET_RELATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${ARYIBI_SHADER})
    message(STATUS "Found aryibi shader asset: ${ASSET_RELATIVE_PATH}")
    # TODO: This won't work with anything that isn't the example
    set(ASSET_IN_BINARY_PATH ${PROJECT_BINARY_DIR}/example/assets/${ASSET_RELATIVE_PATH})
    set(ARYIBI_BINARY_ASSETS ${ARYIBI_BINARY_ASSETS} ${ASSET_IN_BINARY_PATH})

    add_custom_command(OUTPUT ${ASSET_IN_BINARY_PATH}
            DEPENDS ${ARYIBI_SHADER}
            COMMAND cmake -E copy ${ARYIBI_SHADER} ${ASSET_IN_BINARY_PATH}
            COMMENT "Copying ${ASSET_RELATIVE_PATH} to binary directory." VERBATIM)
endforeach()

add_custom_target(copy-aryibi-assets DEPENDS ${ARYIBI_BINARY_ASSETS})
add_dependencies(aryibi copy-aryibi-assets)