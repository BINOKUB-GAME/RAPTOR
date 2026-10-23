# ==============================================================================
# NOM DU SCRIPT   : CMakeLists.txt
# RÉVISION        : Rev 1.5
# PROJET          : RAPTOR v2.0 (Système d'Audit Défensif & Traque Mémoire)
# RESPONSABILITÉ  : Configuration générale du système de construction CMake.
#                   - Cible C99 avec extensions GNU (Linux/POSIX) et standard Windows.
#                   - Définition par défaut du profil de build Release (-O3).
#                   - Neutralisation des avertissements de dépréciation de Raylib.
#                   - Intégration autonome de Raylib via FetchContent.
#                   - Intégration modulaire de l'UI et des 6 greffons système.
#                   - Intégration de Net-Guard et de Net-Intel (plugin_asn_intel.c).
#                   - Assemblage de l'exécutable unifié 'raptor'.
# ==============================================================================

cmake_minimum_required(VERSION 3.16...3.30)

# ------------------------------------------------------------------------------
# 1. DÉFINITION DU PROJET & STANDARDS DE COMPILATION
# ------------------------------------------------------------------------------
project(raptor
    VERSION 2.0.0
    DESCRIPTION "RAPTOR // Système d'Audit Défensif & Traque Mémoire"
    LANGUAGES C
)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Type de build sélectionné" FORCE)
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

if(UNIX)
    add_compile_definitions(_GNU_SOURCE=1)
endif()

# ------------------------------------------------------------------------------
# 2. RÉPERTOIRES D'INCLUSION
# ------------------------------------------------------------------------------
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ------------------------------------------------------------------------------
# 3. GESTION AUTONOME DE RAYLIB (FETCHCONTENT)
# ------------------------------------------------------------------------------
include(FetchContent)

set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.0
    GIT_SHALLOW TRUE
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

set(CMAKE_WARN_DEPRECATED ON CACHE BOOL "" FORCE)

# ------------------------------------------------------------------------------
# 4. COLLECTE DES SOURCES DU SOCLE UNIVERSEL
# ------------------------------------------------------------------------------
set(RAPTOR_CORE_SOURCES
    src/core/raptor_data.c
    src/core/raptor_registry.c
    src/core/raptor_differential.c
    src/core/mem_stealth.c
    src/core/preload_hunter.c
    src/core/raptor_notify.c
)

set(RAPTOR_REPORT_SOURCES
    src/report/report_cli.c
    src/report/report_html.c
    src/report/report_audit_log.c
)

set(RAPTOR_UI_SOURCES
    src/ui/ui_engine.c
    src/ui/ui_resources.c
    src/ui/ui_layout.c
    src/ui/ui_views.c
    src/ui/views/view_global.c
    src/ui/views/view_processes.c
    src/ui/views/view_network.c
    src/ui/views/view_diff.c
    src/ui/views/view_ai.c
    src/ui/modals/modal_guide.c
    src/ui/ui_tooltip.c
)

set(RAPTOR_MAIN_SOURCE
    src/main.c
)

# ------------------------------------------------------------------------------
# 5. GREFFONS (ISOLATION STRICTE SELON L'OS)
# ------------------------------------------------------------------------------
set(RAPTOR_PLUGIN_SOURCES)

if(UNIX)
    list(APPEND RAPTOR_PLUGIN_SOURCES
        src/plugins/plugin_memory_rwx.c
        src/plugins/plugin_network_sockets.c
        src/plugins/plugin_heuristics_expert.c
        src/plugins/plugin_anomaly_engine.c
        src/plugins/plugin_net_guard.c
        src/plugins/plugin_asn_intel.c
    )
    add_compile_definitions(RAPTOR_PLATFORM_LINUX=1)
elseif(WIN32)
    add_compile_definitions(RAPTOR_PLATFORM_WINDOWS=1)
endif()

# ------------------------------------------------------------------------------
# 6. DÉCLARATION DU BINAIRE EXÉCUTABLE & OPTIONS DE COMPILATION
# ------------------------------------------------------------------------------
add_executable(raptor
    ${RAPTOR_MAIN_SOURCE}
    ${RAPTOR_CORE_SOURCES}
    ${RAPTOR_REPORT_SOURCES}
    ${RAPTOR_UI_SOURCES}
    ${RAPTOR_PLUGIN_SOURCES}
)

target_include_directories(raptor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/raptor
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
)

if(MSVC)
    target_compile_options(raptor PRIVATE /W4 /utf-8)
else()
    target_compile_options(raptor PRIVATE -Wall -Wextra -Wformat=2 -Wno-unused-parameter)
endif()

# ------------------------------------------------------------------------------
# 7. LIAISON DES BIBLIOTHÈQUES
# ------------------------------------------------------------------------------
target_link_libraries(raptor PRIVATE raylib)

if(UNIX)
    target_link_libraries(raptor PRIVATE
        m
        pthread
        dl
    )
elseif(WIN32)
    target_link_libraries(raptor PRIVATE
        winmm
        gdi32
        user32
        shell32
    )
endif()
