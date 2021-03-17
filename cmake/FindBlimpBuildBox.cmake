find_path(BLIMP_BUILDBOX_DIRECTORY
    NAMES blimp_buildbox.marker
    HINTS ${PROJECT_SOURCE_DIR}/../buildbox ${PROJECT_SOURCE_DIR}/../blimp-buildbox
    PATH_SUFFIXES install
    DOC "Blimp Buildbox Directory"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BlimpBuildBox DEFAULT_MSG BLIMP_BUILDBOX_DIRECTORY)
