cmake_minimum_required(VERSION 3.16)

# Ustaw ścieżki do dodatkowych komponentów
list(APPEND EXTRA_COMPONENT_DIRS 
    "${CMAKE_CURRENT_LIST_DIR}/components_ssd1306"
    "${CMAKE_CURRENT_LIST_DIR}/esp-idf-lib/components"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(wifi_prov_mgr)
