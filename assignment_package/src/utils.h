#pragma once

#ifndef UTILS_H
#define UTILS_H

enum class TerrainDrawType{ opaque, transparent };
#include <memory>

#define uPtr std::unique_ptr
#define mkU std::make_unique

#define sPtr std::shared_ptr
#define mkS std::make_shared

#endif // UTILS_H
