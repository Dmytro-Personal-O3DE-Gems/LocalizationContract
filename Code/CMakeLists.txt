
# Currently we are in the Code folder: ${CMAKE_CURRENT_LIST_DIR}
# Get the platform specific folder ${pal_dir} for the current folder: ${CMAKE_CURRENT_LIST_DIR}/Platform/${PAL_PLATFORM_NAME}
# Note: o3de_pal_dir will take care of the details for us, as this may be a restricted platform
#       in which case it will see if that platform is present here or in the restricted folder.
#       i.e. It could here in our gem : Gems/LocalizationContract/Code/Platform/<platorm_name>  or
#            <restricted_folder>/<platform_name>/Gems/LocalizationContract/Code
o3de_pal_dir(pal_dir ${CMAKE_CURRENT_LIST_DIR}/Platform/${PAL_PLATFORM_NAME} "${gem_restricted_path}" "${gem_path}" "${gem_parent_relative_path}")

# Now that we have the platform abstraction layer (PAL) folder for this folder, thats where we will find the
# traits for this platform. Traits for a platform are defines for things like whether or not something in this gem
# is supported by this platform.
include(${pal_dir}/PAL_${PAL_PLATFORM_NAME_LOWERCASE}.cmake)

# Check to see if this gem is supported for the current platform
if(NOT PAL_TRAIT_LOCALIZATIONCONTRACT_SUPPORTED)
    return()
endif()

# This gem is header-only: it only declares the LocalizationContract interface
# (request and notification buses). There is intentionally no module and no system component,
# so there are no ${gem_name}.Clients/.Servers/.Unified/.Tools/.Builders targets and nothing
# is loaded at runtime. When a gem has no target for a variant, O3DE simply skips it
# (see o3de_add_variant_dependencies_for_gem_dependencies and ly_enable_gems in cmake/Gems.cmake).
#
# Implementations (e.g. LocalizationGem) and consumers (e.g. InteractionSystemUI)
# depend on Gem::${gem_name}.API.
ly_add_target(
    NAME ${gem_name}.API INTERFACE
    NAMESPACE Gem
    FILES_CMAKE
        localizationcontract_api_files.cmake
        ${pal_dir}/localizationcontract_api_files.cmake
    INCLUDE_DIRECTORIES
        INTERFACE
            Include
    BUILD_DEPENDENCIES
        INTERFACE
           AZ::AzCore
)

# Per-variant aliases of the API target, so consumers can depend on Gem::${gem_name}.<Variant>.API as well.
ly_create_alias(NAME ${gem_name}.Clients.API NAMESPACE Gem TARGETS Gem::${gem_name}.API)
ly_create_alias(NAME ${gem_name}.Servers.API NAMESPACE Gem TARGETS Gem::${gem_name}.API)
ly_create_alias(NAME ${gem_name}.Unified.API NAMESPACE Gem TARGETS Gem::${gem_name}.API)

if(PAL_TRAIT_BUILD_HOST_TOOLS)
    # The template pointed these to ${gem_name}.Editor.API. There is no editor part anymore,
    # so tools and builders use the same header-only API target.
    ly_create_alias(NAME ${gem_name}.Tools.API NAMESPACE Gem TARGETS Gem::${gem_name}.API)
    ly_create_alias(NAME ${gem_name}.Builders.API NAMESPACE Gem TARGETS Gem::${gem_name}.API)
endif()
