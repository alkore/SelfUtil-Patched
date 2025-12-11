#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#ifndef UNAT_DEFINED
#define UNAT_DEFINED
typedef size_t unat;
#endif

// PT_SCE_VERSION : vérifier si déjà défini
#ifndef PT_SCE_VERSION
#define PT_SCE_VERSION (PT_LOOS + 0xfffff01)
#endif

using std::string;
