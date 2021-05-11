/*****************************************************************************
 * Copyright (c) 2014-2020 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include "../common.h"
#include "../peep/Peep.h"

#include <vector>

namespace RCT1
{
    struct RCT1VehicleColourSchemeCopyDescriptor
    {
        int8_t colour1, colour2, colour3;
    };

    colour_t GetColour(colour_t colour);
    PeepSpriteType GetPeepSpriteType(uint8_t rct1SpriteType);

    uint8_t GetRideType(uint8_t rideType, uint8_t vehicleType);
    RCT1VehicleColourSchemeCopyDescriptor GetColourSchemeCopyDescriptor(uint8_t vehicleType);
    bool RideTypeUsesVehicles(uint8_t rideType);
    bool PathIsQueue(uint8_t pathType);
    uint8_t NormalisePathAddition(uint8_t pathAdditionType);
    uint8_t GetVehicleSubEntryIndex(uint8_t vehicleSubEntry);

    std::string_view GetRideTypeObject(uint8_t rideType);
    std::string_view GetVehicleObject(uint8_t vehicleType);
    std::string_view GetSmallSceneryObject(uint8_t smallSceneryType);
    std::string_view GetLargeSceneryObject(uint8_t largeSceneryType);
    std::string_view GetWallObject(uint8_t wallType);
    std::string_view GetPathSurfaceObject(uint8_t pathType);
    std::string_view GetPathAddtionObject(uint8_t pathAdditionType);
    std::string_view GetFootpathRailingsObject(uint8_t footpathRailingsType);
    std::string_view GetSceneryGroupObject(uint8_t sceneryGroupType);
    std::string_view GetWaterObject(uint8_t waterType);
    std::string_view GetTerrainSurfaceObject(uint8_t terrain);
    std::string_view GetTerrainEdgeObject(uint8_t terrainEdge);

    const std::vector<const char*> GetSceneryObjects(uint8_t sceneryType);
} // namespace RCT1
