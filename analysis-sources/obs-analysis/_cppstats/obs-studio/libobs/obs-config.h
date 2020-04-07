#pragma once
#define LIBOBS_API_MAJOR_VER 25
#define LIBOBS_API_MINOR_VER 0
#define LIBOBS_API_PATCH_VER 4
#define MAKE_SEMANTIC_VERSION(major, minor, patch) ((major << 24) | (minor << 16) | patch)
#define LIBOBS_API_VER MAKE_SEMANTIC_VERSION(LIBOBS_API_MAJOR_VER, LIBOBS_API_MINOR_VER, LIBOBS_API_PATCH_VER)
#if defined(HAVE_OBSCONFIG_H)
#include "obsconfig.h"
#else
#define OBS_VERSION "unknown"
#define OBS_DATA_PATH "../../data"
#define OBS_INSTALL_PREFIX ""
#define OBS_PLUGIN_DESTINATION "obs-plugins"
#define OBS_RELATIVE_PREFIX "../../"
#define OBS_RELEASE_CANDIDATE_MAJOR 0
#define OBS_RELEASE_CANDIDATE_MINOR 0
#define OBS_RELEASE_CANDIDATE_PATCH 0
#define OBS_RELEASE_CANDIDATE_VER 0
#define OBS_RELEASE_CANDIDATE 0
#endif
#define OBS_INSTALL_DATA_PATH OBS_INSTALL_PREFIX OBS_DATA_PATH
