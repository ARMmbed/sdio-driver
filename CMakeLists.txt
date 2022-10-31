add_library(component_sdio INTERFACE)

target_include_directories(component_sdio INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

list(APPEND SOURCEFILES 
    SDIOBlockDevice.cpp
)

if("STM32F4" IN_LIST MBED_TARGET_LABELS)
    target_include_directories(component_sdio INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/TARGET_STM/TARGET_STM32F4)
    list(APPEND SOURCEFILES
        TARGET_STM/TARGET_STM32F4/sdio_device.c
    )
endif()

if("STM32F7" IN_LIST MBED_TARGET_LABELS)
    target_include_directories(component_sdio INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/TARGET_STM/TARGET_STM32F7)
    list(APPEND SOURCEFILES
        TARGET_STM/TARGET_STM32F7/sdio_device.c
    )
endif()


target_sources(component_sdio
    INTERFACE
        ${SOURCEFILES}
)
