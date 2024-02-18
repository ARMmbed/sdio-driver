
add_library(mbed-storage-sdio STATIC)

list(APPEND SOURCEFILES 
    SDIOBlockDevice.cpp
)

target_include_directories(mbed-storage-sdio INTERFACE .)

if("STM32F4" IN_LIST MBED_TARGET_LABELS)
    target_include_directories(mbed-storage-sdio PUBLIC ./TARGET_STM/TARGET_STM32F4)
    list(APPEND SOURCEFILES
        TARGET_STM/TARGET_STM32F4/sdio_device.c
    )
endif()

if("STM32F7" IN_LIST MBED_TARGET_LABELS)
    target_include_directories(mbed-storage-sdio PUBLIC ./TARGET_STM/TARGET_STM32F7)
    list(APPEND SOURCEFILES
        TARGET_STM/TARGET_STM32F7/sdio_device.c
    )
endif()

if("STM32H7" IN_LIST MBED_TARGET_LABELS)
    target_include_directories(mbed-storage-sdio PUBLIC ./TARGET_STM/TARGET_STM32H7)
    list(APPEND SOURCEFILES
        TARGET_STM/TARGET_STM32H7/sdio_device.c
    )
endif()

target_link_libraries(mbed-storage-sdio
    PUBLIC
        mbed-rtos-flags
        mbed-storage-blockdevice
    )

target_sources(mbed-storage-sdio
    PRIVATE
        ${SOURCEFILES}
    )
