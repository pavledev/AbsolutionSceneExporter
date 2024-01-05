#pragma once

#include <set>

#include "Resources/HeaderLibrary.h"

inline HeaderLibrary globalHeaderLibrary;
inline HeaderLibrary headerLibrary;
inline rapidjson::StringBuffer sceneJsonBuffer;
inline rapidjson::StringBuffer materialJsonBuffer;
inline std::string outputFolderPath;
inline std::set<unsigned long long> primRuntimeResourceIDs;
