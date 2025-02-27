/*****************************************************************************
 * Copyright (c) 2014-2020 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "../UiContext.h"
#include "../interface/InGameConsole.h"
#include "../scripting/CustomMenu.h"

#include <algorithm>
#include <iterator>
#include <limits>
#include <openrct2-ui/interface/Dropdown.h>
#include <openrct2-ui/interface/LandTool.h>
#include <openrct2-ui/interface/Viewport.h>
#include <openrct2-ui/interface/Widget.h>
#include <openrct2-ui/windows/Window.h>
#include <openrct2/Cheats.h>
#include <openrct2/Context.h>
#include <openrct2/Editor.h>
#include <openrct2/Game.h>
#include <openrct2/Input.h>
#include <openrct2/OpenRCT2.h>
#include <openrct2/ParkImporter.h>
#include <openrct2/actions/BannerPlaceAction.h>
#include <openrct2/actions/BannerSetColourAction.h>
#include <openrct2/actions/ClearAction.h>
#include <openrct2/actions/FootpathAdditionPlaceAction.h>
#include <openrct2/actions/LandLowerAction.h>
#include <openrct2/actions/LandRaiseAction.h>
#include <openrct2/actions/LandSmoothAction.h>
#include <openrct2/actions/LargeSceneryPlaceAction.h>
#include <openrct2/actions/LargeScenerySetColourAction.h>
#include <openrct2/actions/LoadOrQuitAction.h>
#include <openrct2/actions/PauseToggleAction.h>
#include <openrct2/actions/SetCheatAction.h>
#include <openrct2/actions/SmallSceneryPlaceAction.h>
#include <openrct2/actions/SmallScenerySetColourAction.h>
#include <openrct2/actions/SurfaceSetStyleAction.h>
#include <openrct2/actions/WallPlaceAction.h>
#include <openrct2/actions/WallSetColourAction.h>
#include <openrct2/actions/WaterLowerAction.h>
#include <openrct2/actions/WaterRaiseAction.h>
#include <openrct2/audio/audio.h>
#include <openrct2/config/Config.h>
#include <openrct2/interface/Chat.h>
#include <openrct2/interface/InteractiveConsole.h>
#include <openrct2/interface/Screenshot.h>
#include <openrct2/network/network.h>
#include <openrct2/paint/VirtualFloor.h>
#include <openrct2/peep/Staff.h>
#include <openrct2/scenario/Scenario.h>
#include <openrct2/util/Util.h>
#include <openrct2/windows/Intent.h>
#include <openrct2/world/Footpath.h>
#include <openrct2/world/LargeScenery.h>
#include <openrct2/world/Park.h>
#include <openrct2/world/Scenery.h>
#include <openrct2/world/SmallScenery.h>
#include <openrct2/world/Surface.h>
#include <openrct2/world/Wall.h>
#include <string>

using namespace OpenRCT2;
using namespace OpenRCT2::Ui;

// clang-format off
enum {
    WIDX_PAUSE,
    WIDX_FILE_MENU,
    WIDX_MUTE,
    WIDX_ZOOM_OUT,
    WIDX_ZOOM_IN,
    WIDX_ROTATE,
    WIDX_VIEW_MENU,
    WIDX_MAP,

    WIDX_LAND,
    WIDX_WATER,
    WIDX_SCENERY,
    WIDX_PATH,
    WIDX_CONSTRUCT_RIDE,
    WIDX_RIDES,
    WIDX_PARK,
    WIDX_STAFF,
    WIDX_GUESTS,
    WIDX_CLEAR_SCENERY,

    WIDX_FASTFORWARD,
    WIDX_CHEATS,
    WIDX_DEBUG,
    WIDX_FINANCES,
    WIDX_RESEARCH,
    WIDX_NEWS,
    WIDX_NETWORK,
    WIDX_CHAT,

    WIDX_SEPARATOR,
};

validate_global_widx(WC_TOP_TOOLBAR, WIDX_PAUSE);
validate_global_widx(WC_TOP_TOOLBAR, WIDX_LAND);
validate_global_widx(WC_TOP_TOOLBAR, WIDX_WATER);
validate_global_widx(WC_TOP_TOOLBAR, WIDX_SCENERY);
validate_global_widx(WC_TOP_TOOLBAR, WIDX_PATH);

enum FILE_MENU_DDIDX {
    DDIDX_NEW_GAME = 0,
    DDIDX_LOAD_GAME = 1,
    DDIDX_SAVE_GAME = 2,
    DDIDX_SAVE_GAME_AS = 3,
    // separator
    DDIDX_ABOUT = 5,
    DDIDX_OPTIONS = 6,
    DDIDX_SCREENSHOT = 7,
    DDIDX_GIANT_SCREENSHOT = 8,
    // separator
    DDIDX_QUIT_TO_MENU = 10,
    DDIDX_EXIT_OPENRCT2 = 11,
    // separator
    DDIDX_UPDATE_AVAILABLE = 13,
};

enum TOP_TOOLBAR_VIEW_MENU_DDIDX {
    DDIDX_UNDERGROUND_INSIDE = 0,
    DDIDX_TRANSPARENT_WATER = 1,
    DDIDX_HIDE_BASE = 2,
    DDIDX_HIDE_VERTICAL = 3,
    // separator
    DDIDX_SEETHROUGH_RIDES = 5,
    DDIDX_SEETHROUGH_SCENERY = 6,
    DDIDX_SEETHROUGH_PATHS = 7,
    DDIDX_INVISIBLE_SUPPORTS = 8,
    DDIDX_INVISIBLE_PEEPS = 9,
    // separator
    DDIDX_LAND_HEIGHTS = 11,
    DDIDX_TRACK_HEIGHTS = 12,
    DDIDX_PATH_HEIGHTS = 13,
    // separator
    DDIDX_VIEW_CLIPPING = 15,
    DDIDX_HIGHLIGHT_PATH_ISSUES = 16,

    TOP_TOOLBAR_VIEW_MENU_COUNT
};

enum TOP_TOOLBAR_DEBUG_DDIDX {
    DDIDX_CONSOLE = 0,
    DDIDX_DEBUG_PAINT = 1,

    TOP_TOOLBAR_DEBUG_COUNT
};

enum TOP_TOOLBAR_NETWORK_DDIDX {
    DDIDX_MULTIPLAYER = 0,
    DDIDX_MULTIPLAYER_RECONNECT = 1,

    TOP_TOOLBAR_NETWORK_COUNT
};

enum {
    DDIDX_CHEATS,
    DDIDX_TILE_INSPECTOR = 1,
    DDIDX_OBJECT_SELECTION = 2,
    DDIDX_INVENTIONS_LIST = 3,
    DDIDX_SCENARIO_OPTIONS = 4,
    DDIDX_OBJECTIVE_OPTIONS = 5,
    // 6 is a separator
    DDIDX_ENABLE_SANDBOX_MODE = 7,
    DDIDX_DISABLE_CLEARANCE_CHECKS = 8,
    DDIDX_DISABLE_SUPPORT_LIMITS = 9,

    TOP_TOOLBAR_CHEATS_COUNT
};

enum {
    DDIDX_SHOW_MAP,
    DDIDX_OPEN_VIEWPORT,
};

enum {
    DDIDX_ROTATE_CLOCKWISE,
    DDIDX_ROTATE_ANTI_CLOCKWISE,
};

#pragma region Toolbar_widget_ordering

// from left to right
static constexpr const int32_t left_aligned_widgets_order[] = {
    WIDX_PAUSE,
    WIDX_FASTFORWARD,
    WIDX_FILE_MENU,
    WIDX_MUTE,
    WIDX_NETWORK,
    WIDX_CHAT,
    WIDX_CHEATS,
    WIDX_DEBUG,

    WIDX_SEPARATOR,

    WIDX_ZOOM_OUT,
    WIDX_ZOOM_IN,
    WIDX_ROTATE,
    WIDX_VIEW_MENU,
    WIDX_MAP,

};

// from right to left
static constexpr const int32_t right_aligned_widgets_order[] = {
    WIDX_NEWS,
    WIDX_GUESTS,
    WIDX_STAFF,
    WIDX_PARK,
    WIDX_RIDES,
    WIDX_RESEARCH,
    WIDX_FINANCES,

    WIDX_SEPARATOR,

    WIDX_CONSTRUCT_RIDE,
    WIDX_PATH,
    WIDX_SCENERY,
    WIDX_WATER,
    WIDX_LAND,
    WIDX_CLEAR_SCENERY
};

#pragma endregion

static rct_widget window_top_toolbar_widgets[] = {
    MakeRemapWidget({  0, 0}, {30, TOP_TOOLBAR_HEIGHT + 1}, WindowWidgetType::TrnBtn, WindowColour::Primary   , SPR_TOOLBAR_PAUSE,          STR_PAUSE_GAME_TIP                ), // Pause
    MakeRemapWidget({ 60, 0}, {30, TOP_TOOLBAR_HEIGHT + 1}, WindowWidgetType::TrnBtn, WindowColour::Primary   , SPR_TOOLBAR_FILE,           STR_DISC_AND_GAME_OPTIONS_TIP     ), // File menu
    MakeRemapWidget({250, 0}, {30, TOP_TOOLBAR_HEIGHT + 1}, WindowWidgetType::TrnBtn, WindowColour::Primary   , SPR_G2_TOOLBAR_MUTE,        STR_TOOLBAR_MUTE_TIP              ), // Mute
    MakeRemapWidget({100, 0}, {30, TOP_TOOLBAR_HEIGHT + 1}, WindowWidgetType::TrnBtn, WindowColour::Secondary , SPR_TOOLBAR_ZOOM_OUT,       STR_ZOOM_OUT_TIP                  ), // Zoom out
    MakeRemapWidget({130, 0}, {30, TOP_TOOLBAR_HEIGHT + 1}, WindowWidgetType::TrnBtn, WindowColour::Secondary , SPR_TOOLBAR_ZOOM_IN,        STR_ZOOM_IN_TIP                   ), // Zoom in
    MakeRemapWidget({160, 0}, {30, TOP_TOOLBAR_HEIGHT + 1}, WindowWidgetType::TrnBtn, WindowColour::Secondary , SPR_TOOLBAR_ROTATE,         STR_ROTATE_TIP                    ), // Rotate camera
    MakeRemapWidget({190, 0}, {30, TOP_TOOLBAR_HEIGHT + 1}, WindowWidgetType::TrnBtn, WindowColour::Secondary , SPR_TOOLBAR_VIEW,           STR_VIEW_OPTIONS_TIP              ), // Transparency menu
    MakeRemapWidget({220, 0}, {30, TOP_TOOLBAR_HEIGHT + 1}, WindowWidgetType::TrnBtn, WindowColour::Secondary , SPR_TOOLBAR_MAP,            STR_SHOW_MAP_TIP                  ), // Map
    MakeRemapWidget({267, 0}, {30, TOP_TOOLBAR_HEIGHT + 1}, WindowWidgetType::TrnBtn, WindowColour::Tertiary  , SPR_TOOLBAR_LAND,           STR_ADJUST_LAND_TIP               ), // Land
    MakeRemapWidget({297, 0}, {30, TOP_TOOLBAR_HEIGHT + 1}, WindowWidgetType::TrnBtn, WindowColour::Tertiary  , SPR_TOOLBAR_WATER,          STR_ADJUST_WATER_TIP              ), // Water
    MakeRemapWidget({327, 0}, {30, TOP_TOOLBAR_HEIGHT + 1}, WindowWidgetType::TrnBtn, WindowColour::Tertiary  , SPR_TOOLBAR_SCENERY,        STR_PLACE_SCENERY_TIP             ), // Scenery
    MakeRemapWidget({357, 0}, {30, TOP_TOOLBAR_HEIGHT + 1}, WindowWidgetType::TrnBtn, WindowColour::Tertiary  , SPR_TOOLBAR_FOOTPATH,       STR_BUILD_FOOTPATH_TIP            ), // Path
    MakeRemapWidget({387, 0}, {30, TOP_TOOLBAR_HEIGHT + 1}, WindowWidgetType::TrnBtn, WindowColour::Tertiary  , SPR_TOOLBAR_CONSTRUCT_RIDE, STR_BUILD_RIDE_TIP                ), // Construct ride
    MakeRemapWidget({490, 0}, {30, TOP_TOOLBAR_HEIGHT + 1}, WindowWidgetType::TrnBtn, WindowColour::Quaternary, SPR_TOOLBAR_RIDES,          STR_RIDES_IN_PARK_TIP             ), // Rides
    MakeRemapWidget({520, 0}, {30, TOP_TOOLBAR_HEIGHT + 1}, WindowWidgetType::TrnBtn, WindowColour::Quaternary, SPR_TOOLBAR_PARK,           STR_PARK_INFORMATION_TIP          ), // Park
    MakeRemapWidget({550, 0}, {30, TOP_TOOLBAR_HEIGHT + 1}, WindowWidgetType::TrnBtn, WindowColour::Quaternary, SPR_TAB_TOOLBAR,            STR_STAFF_TIP                     ), // Staff
    MakeRemapWidget({560, 0}, {30, TOP_TOOLBAR_HEIGHT + 1}, WindowWidgetType::TrnBtn, WindowColour::Quaternary, SPR_TOOLBAR_GUESTS,         STR_GUESTS_TIP                    ), // Guests
    MakeRemapWidget({560, 0}, {30, TOP_TOOLBAR_HEIGHT + 1}, WindowWidgetType::TrnBtn, WindowColour::Tertiary  , SPR_TOOLBAR_CLEAR_SCENERY,  STR_CLEAR_SCENERY_TIP             ), // Clear scenery
    MakeRemapWidget({ 30, 0}, {30, TOP_TOOLBAR_HEIGHT + 1}, WindowWidgetType::TrnBtn, WindowColour::Primary   , SPR_TAB_TOOLBAR,            STR_GAME_SPEED_TIP                ), // Fast forward
    MakeRemapWidget({ 30, 0}, {30, TOP_TOOLBAR_HEIGHT + 1}, WindowWidgetType::TrnBtn, WindowColour::Primary   , SPR_TAB_TOOLBAR,            STR_CHEATS_TIP                    ), // Cheats
    MakeRemapWidget({ 30, 0}, {30, TOP_TOOLBAR_HEIGHT + 1}, WindowWidgetType::TrnBtn, WindowColour::Primary   , SPR_TAB_TOOLBAR,            STR_DEBUG_TIP                     ), // Debug
    MakeRemapWidget({ 30, 0}, {30, TOP_TOOLBAR_HEIGHT + 1}, WindowWidgetType::TrnBtn, WindowColour::Quaternary, SPR_TAB_TOOLBAR,            STR_SCENARIO_OPTIONS_FINANCIAL_TIP), // Finances
    MakeRemapWidget({ 30, 0}, {30, TOP_TOOLBAR_HEIGHT + 1}, WindowWidgetType::TrnBtn, WindowColour::Quaternary, SPR_TAB_TOOLBAR,            STR_FINANCES_RESEARCH_TIP         ), // Research
    MakeRemapWidget({ 30, 0}, {30, TOP_TOOLBAR_HEIGHT + 1}, WindowWidgetType::TrnBtn, WindowColour::Quaternary, SPR_TAB_TOOLBAR,            STR_SHOW_RECENT_MESSAGES_TIP      ), // News
    MakeRemapWidget({ 30, 0}, {30, TOP_TOOLBAR_HEIGHT + 1}, WindowWidgetType::TrnBtn, WindowColour::Primary   , SPR_G2_TOOLBAR_MULTIPLAYER, STR_SHOW_MULTIPLAYER_STATUS_TIP   ), // Network
    MakeRemapWidget({ 30, 0}, {30, TOP_TOOLBAR_HEIGHT + 1}, WindowWidgetType::TrnBtn, WindowColour::Primary   , SPR_TAB_TOOLBAR,            STR_TOOLBAR_CHAT_TIP              ), // Chat
    MakeWidget     ({  0, 0}, {10,                      1}, WindowWidgetType::Empty,  WindowColour::Primary                                                                   ), // Artificial widget separator
    { WIDGETS_END },
};

static void window_top_toolbar_mouseup(rct_window *w, rct_widgetindex widgetIndex);
static void window_top_toolbar_mousedown(rct_window *w, rct_widgetindex widgetIndex, rct_widget* widget);
static void window_top_toolbar_dropdown(rct_window *w, rct_widgetindex widgetIndex, int32_t dropdownIndex);
static void window_top_toolbar_tool_update(rct_window* w, rct_widgetindex widgetIndex, const ScreenCoordsXY& screenCoords);
static void window_top_toolbar_tool_down(rct_window* w, rct_widgetindex widgetIndex, const ScreenCoordsXY& screenCoords);
static void window_top_toolbar_tool_drag(rct_window* w, rct_widgetindex widgetIndex, const ScreenCoordsXY& screenCoords);
static void window_top_toolbar_tool_up(rct_window* w, rct_widgetindex widgetIndex, const ScreenCoordsXY& screenCoordsy);
static void window_top_toolbar_tool_abort(rct_window *w, rct_widgetindex widgetIndex);
static void window_top_toolbar_invalidate(rct_window *w);
static void window_top_toolbar_paint(rct_window *w, rct_drawpixelinfo *dpi);

static rct_window_event_list window_top_toolbar_events([](auto& events)
{
    events.mouse_up = &window_top_toolbar_mouseup;
    events.mouse_down = &window_top_toolbar_mousedown;
    events.dropdown = &window_top_toolbar_dropdown;
    events.tool_update = &window_top_toolbar_tool_update;
    events.tool_down = &window_top_toolbar_tool_down;
    events.tool_drag = &window_top_toolbar_tool_drag;
    events.tool_up = &window_top_toolbar_tool_up;
    events.tool_abort = &window_top_toolbar_tool_abort;
    events.invalidate = &window_top_toolbar_invalidate;
    events.paint = &window_top_toolbar_paint;
});
// clang-format on

static void top_toolbar_init_view_menu(rct_window* window, rct_widget* widget);
static void top_toolbar_view_menu_dropdown(int16_t dropdownIndex);
static void top_toolbar_init_map_menu(rct_window* window, rct_widget* widget);
static void top_toolbar_map_menu_dropdown(int16_t dropdownIndex);
static void top_toolbar_init_fastforward_menu(rct_window* window, rct_widget* widget);
static void top_toolbar_fastforward_menu_dropdown(int16_t dropdownIndex);
static void top_toolbar_init_rotate_menu(rct_window* window, rct_widget* widget);
static void top_toolbar_rotate_menu_dropdown(int16_t dropdownIndex);
static void top_toolbar_init_cheats_menu(rct_window* window, rct_widget* widget);
static void top_toolbar_cheats_menu_dropdown(int16_t dropdownIndex);
static void top_toolbar_init_debug_menu(rct_window* window, rct_widget* widget);
static void top_toolbar_debug_menu_dropdown(int16_t dropdownIndex);
static void top_toolbar_init_network_menu(rct_window* window, rct_widget* widget);
static void top_toolbar_network_menu_dropdown(int16_t dropdownIndex);

static void toggle_footpath_window();
static void toggle_land_window(rct_window* topToolbar, rct_widgetindex widgetIndex);
static void toggle_clear_scenery_window(rct_window* topToolbar, rct_widgetindex widgetIndex);
static void toggle_water_window(rct_window* topToolbar, rct_widgetindex widgetIndex);

static money32 selection_lower_land(uint8_t flags);
static money32 selection_raise_land(uint8_t flags);

static ClearAction GetClearAction();

static bool _landToolBlocked;
static uint8_t _unkF64F0E;
static int16_t _unkF64F0A;

/**
 * Creates the main game top toolbar window.
 *  rct2: 0x0066B485 (part of 0x0066B3E8)
 */
rct_window* window_top_toolbar_open()
{
    rct_window* window = WindowCreate(
        ScreenCoordsXY(0, 0), context_get_width(), TOP_TOOLBAR_HEIGHT + 1, &window_top_toolbar_events, WC_TOP_TOOLBAR,
        WF_STICK_TO_FRONT | WF_TRANSPARENT | WF_NO_BACKGROUND);
    window->widgets = window_top_toolbar_widgets;

    WindowInitScrollWidgets(window);

    return window;
}

/**
 *
 *  rct2: 0x0066C957
 */
static void window_top_toolbar_mouseup(rct_window* w, rct_widgetindex widgetIndex)
{
    rct_window* mainWindow;

    switch (widgetIndex)
    {
        case WIDX_PAUSE:
            if (network_get_mode() != NETWORK_MODE_CLIENT)
            {
                auto pauseToggleAction = PauseToggleAction();
                GameActions::Execute(&pauseToggleAction);
            }
            break;
        case WIDX_ZOOM_OUT:
            if ((mainWindow = window_get_main()) != nullptr)
                window_zoom_out(mainWindow, false);
            break;
        case WIDX_ZOOM_IN:
            if ((mainWindow = window_get_main()) != nullptr)
                window_zoom_in(mainWindow, false);
            break;
        case WIDX_CLEAR_SCENERY:
            toggle_clear_scenery_window(w, WIDX_CLEAR_SCENERY);
            break;
        case WIDX_LAND:
            toggle_land_window(w, WIDX_LAND);
            break;
        case WIDX_WATER:
            toggle_water_window(w, WIDX_WATER);
            break;
        case WIDX_SCENERY:
            if (!tool_set(w, WIDX_SCENERY, Tool::Arrow))
            {
                input_set_flag(INPUT_FLAG_6, true);
                context_open_window(WC_SCENERY);
            }
            break;
        case WIDX_PATH:
            toggle_footpath_window();
            break;
        case WIDX_CONSTRUCT_RIDE:
            context_open_window(WC_CONSTRUCT_RIDE);
            break;
        case WIDX_RIDES:
            context_open_window(WC_RIDE_LIST);
            break;
        case WIDX_PARK:
            context_open_window(WC_PARK_INFORMATION);
            break;
        case WIDX_STAFF:
            context_open_window(WC_STAFF_LIST);
            break;
        case WIDX_GUESTS:
            context_open_window(WC_GUEST_LIST);
            break;
        case WIDX_FINANCES:
            context_open_window(WC_FINANCES);
            break;
        case WIDX_RESEARCH:
            context_open_window(WC_RESEARCH);
            break;
        case WIDX_NEWS:
            context_open_window(WC_RECENT_NEWS);
            break;
        case WIDX_MUTE:
            OpenRCT2::Audio::ToggleAllSounds();
            break;
        case WIDX_CHAT:
            if (chat_available())
            {
                chat_toggle();
            }
            else
            {
                context_show_error(STR_CHAT_UNAVAILABLE, STR_NONE, {});
            }
            break;
    }
}

/**
 *
 *  rct2: 0x0066CA3B
 */
static void window_top_toolbar_mousedown(rct_window* w, rct_widgetindex widgetIndex, rct_widget* widget)
{
    int32_t numItems;

    switch (widgetIndex)
    {
        case WIDX_FILE_MENU:
            if (gScreenFlags & (SCREEN_FLAGS_TRACK_DESIGNER | SCREEN_FLAGS_TRACK_MANAGER))
            {
                gDropdownItemsFormat[0] = STR_ABOUT;
                gDropdownItemsFormat[1] = STR_OPTIONS;
                gDropdownItemsFormat[2] = STR_SCREENSHOT;
                gDropdownItemsFormat[3] = STR_GIANT_SCREENSHOT;
                gDropdownItemsFormat[4] = STR_EMPTY;
                gDropdownItemsFormat[5] = STR_QUIT_TRACK_DESIGNS_MANAGER;
                gDropdownItemsFormat[6] = STR_EXIT_OPENRCT2;

                if (gScreenFlags & SCREEN_FLAGS_TRACK_DESIGNER)
                    gDropdownItemsFormat[5] = STR_QUIT_ROLLERCOASTER_DESIGNER;

                numItems = 7;
            }
            else if (gScreenFlags & SCREEN_FLAGS_SCENARIO_EDITOR)
            {
                gDropdownItemsFormat[0] = STR_LOAD_LANDSCAPE;
                gDropdownItemsFormat[1] = STR_SAVE_LANDSCAPE;
                gDropdownItemsFormat[2] = STR_EMPTY;
                gDropdownItemsFormat[3] = STR_ABOUT;
                gDropdownItemsFormat[4] = STR_OPTIONS;
                gDropdownItemsFormat[5] = STR_SCREENSHOT;
                gDropdownItemsFormat[6] = STR_GIANT_SCREENSHOT;
                gDropdownItemsFormat[7] = STR_EMPTY;
                gDropdownItemsFormat[8] = STR_QUIT_SCENARIO_EDITOR;
                gDropdownItemsFormat[9] = STR_EXIT_OPENRCT2;
                numItems = 10;
            }
            else
            {
                gDropdownItemsFormat[0] = STR_NEW_GAME;
                gDropdownItemsFormat[1] = STR_LOAD_GAME;
                gDropdownItemsFormat[2] = STR_SAVE_GAME;
                gDropdownItemsFormat[3] = STR_SAVE_GAME_AS;
                gDropdownItemsFormat[4] = STR_EMPTY;
                gDropdownItemsFormat[5] = STR_ABOUT;
                gDropdownItemsFormat[6] = STR_OPTIONS;
                gDropdownItemsFormat[7] = STR_SCREENSHOT;
                gDropdownItemsFormat[8] = STR_GIANT_SCREENSHOT;
                gDropdownItemsFormat[9] = STR_EMPTY;
                gDropdownItemsFormat[10] = STR_QUIT_TO_MENU;
                gDropdownItemsFormat[11] = STR_EXIT_OPENRCT2;
                numItems = 12;
                if (OpenRCT2::GetContext()->HasNewVersionInfo())
                {
                    gDropdownItemsFormat[12] = STR_EMPTY;
                    gDropdownItemsFormat[13] = STR_UPDATE_AVAILABLE;
                    numItems += 2;
                }
            }
            WindowDropdownShowText(
                { w->windowPos.x + widget->left, w->windowPos.y + widget->top }, widget->height() + 1, w->colours[0] | 0x80,
                Dropdown::Flag::StayOpen, numItems);
            break;
        case WIDX_CHEATS:
            top_toolbar_init_cheats_menu(w, widget);
            break;
        case WIDX_VIEW_MENU:
            top_toolbar_init_view_menu(w, widget);
            break;
        case WIDX_MAP:
            top_toolbar_init_map_menu(w, widget);
            break;
        case WIDX_FASTFORWARD:
            top_toolbar_init_fastforward_menu(w, widget);
            break;
        case WIDX_ROTATE:
            top_toolbar_init_rotate_menu(w, widget);
            break;
        case WIDX_DEBUG:
            top_toolbar_init_debug_menu(w, widget);
            break;
        case WIDX_NETWORK:
            top_toolbar_init_network_menu(w, widget);
            break;
    }
}

static void window_top_toolbar_scenarioselect_callback(const utf8* path)
{
    window_close_by_class(WC_EDITOR_OBJECT_SELECTION);
    context_load_park_from_file(path);
}

/**
 *
 *  rct2: 0x0066C9EA
 */
static void window_top_toolbar_dropdown(rct_window* w, rct_widgetindex widgetIndex, int32_t dropdownIndex)
{
    switch (widgetIndex)
    {
        case WIDX_FILE_MENU:

            // New game is only available in the normal game. Skip one position to avoid incorrect mappings in the menus of the
            // other modes.
            if (gScreenFlags & (SCREEN_FLAGS_SCENARIO_EDITOR))
                dropdownIndex += 1;

            // Quicksave is only available in the normal game. Skip one position to avoid incorrect mappings in the menus of the
            // other modes.
            if (gScreenFlags & (SCREEN_FLAGS_SCENARIO_EDITOR) && dropdownIndex > DDIDX_LOAD_GAME)
                dropdownIndex += 1;

            // Track designer and track designs manager start with About, not Load/save
            if (gScreenFlags & (SCREEN_FLAGS_TRACK_DESIGNER | SCREEN_FLAGS_TRACK_MANAGER))
                dropdownIndex += DDIDX_ABOUT;

            switch (dropdownIndex)
            {
                case DDIDX_NEW_GAME:
                {
                    auto intent = Intent(WC_SCENARIO_SELECT);
                    intent.putExtra(INTENT_EXTRA_CALLBACK, reinterpret_cast<void*>(window_top_toolbar_scenarioselect_callback));
                    context_open_intent(&intent);
                    break;
                }
                case DDIDX_LOAD_GAME:
                {
                    auto loadOrQuitAction = LoadOrQuitAction(LoadOrQuitModes::OpenSavePrompt);
                    GameActions::Execute(&loadOrQuitAction);
                    break;
                }
                case DDIDX_SAVE_GAME:
                    tool_cancel();
                    save_game();
                    break;
                case DDIDX_SAVE_GAME_AS:
                    if (gScreenFlags & SCREEN_FLAGS_SCENARIO_EDITOR)
                    {
                        auto intent = Intent(WC_LOADSAVE);
                        intent.putExtra(INTENT_EXTRA_LOADSAVE_TYPE, LOADSAVETYPE_SAVE | LOADSAVETYPE_LANDSCAPE);
                        intent.putExtra(INTENT_EXTRA_PATH, std::string{ gS6Info.name });
                        context_open_intent(&intent);
                    }
                    else
                    {
                        tool_cancel();
                        save_game_as();
                    }
                    break;
                case DDIDX_ABOUT:
                    context_open_window(WC_ABOUT);
                    break;
                case DDIDX_OPTIONS:
                    context_open_window(WC_OPTIONS);
                    break;
                case DDIDX_SCREENSHOT:
                    gScreenshotCountdown = 10;
                    break;
                case DDIDX_GIANT_SCREENSHOT:
                    screenshot_giant();
                    break;
                case DDIDX_QUIT_TO_MENU:
                {
                    window_close_by_class(WC_MANAGE_TRACK_DESIGN);
                    window_close_by_class(WC_TRACK_DELETE_PROMPT);
                    auto loadOrQuitAction = LoadOrQuitAction(LoadOrQuitModes::OpenSavePrompt, PromptMode::SaveBeforeQuit);
                    GameActions::Execute(&loadOrQuitAction);
                    break;
                }
                case DDIDX_EXIT_OPENRCT2:
                    context_quit();
                    break;
                case DDIDX_UPDATE_AVAILABLE:
                    context_open_window_view(WV_NEW_VERSION_INFO);
                    break;
            }
            break;
        case WIDX_CHEATS:
            top_toolbar_cheats_menu_dropdown(dropdownIndex);
            break;
        case WIDX_VIEW_MENU:
            top_toolbar_view_menu_dropdown(dropdownIndex);
            break;
        case WIDX_MAP:
            top_toolbar_map_menu_dropdown(dropdownIndex);
            break;
        case WIDX_FASTFORWARD:
            top_toolbar_fastforward_menu_dropdown(dropdownIndex);
            break;
        case WIDX_ROTATE:
            top_toolbar_rotate_menu_dropdown(dropdownIndex);
            break;
        case WIDX_DEBUG:
            top_toolbar_debug_menu_dropdown(dropdownIndex);
            break;
        case WIDX_NETWORK:
            top_toolbar_network_menu_dropdown(dropdownIndex);
            break;
    }
}

/**
 *
 *  rct2: 0x0066C810
 */
static void window_top_toolbar_invalidate(rct_window* w)
{
    int32_t x, enabledWidgets, widgetIndex, widgetWidth, firstAlignment;
    rct_widget* widget;

    // Enable / disable buttons
    window_top_toolbar_widgets[WIDX_PAUSE].type = WindowWidgetType::TrnBtn;
    window_top_toolbar_widgets[WIDX_FILE_MENU].type = WindowWidgetType::TrnBtn;
    window_top_toolbar_widgets[WIDX_ZOOM_OUT].type = WindowWidgetType::TrnBtn;
    window_top_toolbar_widgets[WIDX_ZOOM_IN].type = WindowWidgetType::TrnBtn;
    window_top_toolbar_widgets[WIDX_ROTATE].type = WindowWidgetType::TrnBtn;
    window_top_toolbar_widgets[WIDX_VIEW_MENU].type = WindowWidgetType::TrnBtn;
    window_top_toolbar_widgets[WIDX_MAP].type = WindowWidgetType::TrnBtn;
    window_top_toolbar_widgets[WIDX_MUTE].type = WindowWidgetType::TrnBtn;
    window_top_toolbar_widgets[WIDX_CHAT].type = WindowWidgetType::TrnBtn;
    window_top_toolbar_widgets[WIDX_LAND].type = WindowWidgetType::TrnBtn;
    window_top_toolbar_widgets[WIDX_WATER].type = WindowWidgetType::TrnBtn;
    window_top_toolbar_widgets[WIDX_SCENERY].type = WindowWidgetType::TrnBtn;
    window_top_toolbar_widgets[WIDX_PATH].type = WindowWidgetType::TrnBtn;
    window_top_toolbar_widgets[WIDX_CONSTRUCT_RIDE].type = WindowWidgetType::TrnBtn;
    window_top_toolbar_widgets[WIDX_RIDES].type = WindowWidgetType::TrnBtn;
    window_top_toolbar_widgets[WIDX_PARK].type = WindowWidgetType::TrnBtn;
    window_top_toolbar_widgets[WIDX_STAFF].type = WindowWidgetType::TrnBtn;
    window_top_toolbar_widgets[WIDX_GUESTS].type = WindowWidgetType::TrnBtn;
    window_top_toolbar_widgets[WIDX_CLEAR_SCENERY].type = WindowWidgetType::TrnBtn;
    window_top_toolbar_widgets[WIDX_FINANCES].type = WindowWidgetType::TrnBtn;
    window_top_toolbar_widgets[WIDX_RESEARCH].type = WindowWidgetType::TrnBtn;
    window_top_toolbar_widgets[WIDX_FASTFORWARD].type = WindowWidgetType::TrnBtn;
    window_top_toolbar_widgets[WIDX_CHEATS].type = WindowWidgetType::TrnBtn;
    window_top_toolbar_widgets[WIDX_DEBUG].type = gConfigGeneral.debugging_tools ? WindowWidgetType::TrnBtn
                                                                                 : WindowWidgetType::Empty;
    window_top_toolbar_widgets[WIDX_NEWS].type = WindowWidgetType::TrnBtn;
    window_top_toolbar_widgets[WIDX_NETWORK].type = WindowWidgetType::TrnBtn;

    if (!gConfigInterface.toolbar_show_mute)
    {
        window_top_toolbar_widgets[WIDX_MUTE].type = WindowWidgetType::Empty;
    }

    if (!gConfigInterface.toolbar_show_chat)
    {
        window_top_toolbar_widgets[WIDX_CHAT].type = WindowWidgetType::Empty;
    }

    if (gScreenFlags & SCREEN_FLAGS_SCENARIO_EDITOR || gScreenFlags & SCREEN_FLAGS_TRACK_MANAGER)
    {
        window_top_toolbar_widgets[WIDX_PAUSE].type = WindowWidgetType::Empty;
    }

    if (gScreenFlags & SCREEN_FLAGS_EDITOR)
    {
        window_top_toolbar_widgets[WIDX_PARK].type = WindowWidgetType::Empty;
        window_top_toolbar_widgets[WIDX_STAFF].type = WindowWidgetType::Empty;
        window_top_toolbar_widgets[WIDX_GUESTS].type = WindowWidgetType::Empty;
        window_top_toolbar_widgets[WIDX_FINANCES].type = WindowWidgetType::Empty;
        window_top_toolbar_widgets[WIDX_RESEARCH].type = WindowWidgetType::Empty;
        window_top_toolbar_widgets[WIDX_NEWS].type = WindowWidgetType::Empty;
        window_top_toolbar_widgets[WIDX_NETWORK].type = WindowWidgetType::Empty;

        if (gS6Info.editor_step != EditorStep::LandscapeEditor)
        {
            window_top_toolbar_widgets[WIDX_LAND].type = WindowWidgetType::Empty;
            window_top_toolbar_widgets[WIDX_WATER].type = WindowWidgetType::Empty;
        }

        if (gS6Info.editor_step != EditorStep::RollercoasterDesigner)
        {
            window_top_toolbar_widgets[WIDX_RIDES].type = WindowWidgetType::Empty;
            window_top_toolbar_widgets[WIDX_CONSTRUCT_RIDE].type = WindowWidgetType::Empty;
            window_top_toolbar_widgets[WIDX_FASTFORWARD].type = WindowWidgetType::Empty;
        }

        if (gS6Info.editor_step != EditorStep::LandscapeEditor && gS6Info.editor_step != EditorStep::RollercoasterDesigner)
        {
            window_top_toolbar_widgets[WIDX_MAP].type = WindowWidgetType::Empty;
            window_top_toolbar_widgets[WIDX_SCENERY].type = WindowWidgetType::Empty;
            window_top_toolbar_widgets[WIDX_PATH].type = WindowWidgetType::Empty;
            window_top_toolbar_widgets[WIDX_CLEAR_SCENERY].type = WindowWidgetType::Empty;

            window_top_toolbar_widgets[WIDX_ZOOM_OUT].type = WindowWidgetType::Empty;
            window_top_toolbar_widgets[WIDX_ZOOM_IN].type = WindowWidgetType::Empty;
            window_top_toolbar_widgets[WIDX_ROTATE].type = WindowWidgetType::Empty;
            window_top_toolbar_widgets[WIDX_VIEW_MENU].type = WindowWidgetType::Empty;
        }
    }
    else
    {
        if ((gParkFlags & PARK_FLAGS_NO_MONEY) || !gConfigInterface.toolbar_show_finances)
            window_top_toolbar_widgets[WIDX_FINANCES].type = WindowWidgetType::Empty;

        if (!gConfigInterface.toolbar_show_research)
            window_top_toolbar_widgets[WIDX_RESEARCH].type = WindowWidgetType::Empty;

        if (!gConfigInterface.toolbar_show_cheats)
            window_top_toolbar_widgets[WIDX_CHEATS].type = WindowWidgetType::Empty;

        if (!gConfigInterface.toolbar_show_news)
            window_top_toolbar_widgets[WIDX_NEWS].type = WindowWidgetType::Empty;

        if (!gConfigInterface.toolbar_show_zoom)
        {
            window_top_toolbar_widgets[WIDX_ZOOM_IN].type = WindowWidgetType::Empty;
            window_top_toolbar_widgets[WIDX_ZOOM_OUT].type = WindowWidgetType::Empty;
        }

        switch (network_get_mode())
        {
            case NETWORK_MODE_NONE:
                window_top_toolbar_widgets[WIDX_NETWORK].type = WindowWidgetType::Empty;
                window_top_toolbar_widgets[WIDX_CHAT].type = WindowWidgetType::Empty;
                break;
            case NETWORK_MODE_CLIENT:
                window_top_toolbar_widgets[WIDX_PAUSE].type = WindowWidgetType::Empty;
                [[fallthrough]];
            case NETWORK_MODE_SERVER:
                window_top_toolbar_widgets[WIDX_FASTFORWARD].type = WindowWidgetType::Empty;
                break;
        }
    }

    enabledWidgets = 0;
    for (int i = WIDX_PAUSE; i <= WIDX_CHAT; i++)
        if (window_top_toolbar_widgets[i].type != WindowWidgetType::Empty)
            enabledWidgets |= (1 << i);
    w->enabled_widgets = enabledWidgets;

    // Align left hand side toolbar buttons
    firstAlignment = 1;
    x = 0;
    for (size_t i = 0; i < std::size(left_aligned_widgets_order); ++i)
    {
        widgetIndex = left_aligned_widgets_order[i];
        widget = &window_top_toolbar_widgets[widgetIndex];
        if (widget->type == WindowWidgetType::Empty && widgetIndex != WIDX_SEPARATOR)
            continue;

        if (firstAlignment && widgetIndex == WIDX_SEPARATOR)
            continue;

        widgetWidth = widget->width();
        widget->left = x;
        x += widgetWidth;
        widget->right = x;
        x += 1;
        firstAlignment = 0;
    }

    // Align right hand side toolbar buttons
    int32_t screenWidth = context_get_width();
    firstAlignment = 1;
    x = std::max(640, screenWidth);
    for (size_t i = 0; i < std::size(right_aligned_widgets_order); ++i)
    {
        widgetIndex = right_aligned_widgets_order[i];
        widget = &window_top_toolbar_widgets[widgetIndex];
        if (widget->type == WindowWidgetType::Empty && widgetIndex != WIDX_SEPARATOR)
            continue;

        if (firstAlignment && widgetIndex == WIDX_SEPARATOR)
            continue;

        widgetWidth = widget->width();
        x -= 1;
        widget->right = x;
        x -= widgetWidth;
        widget->left = x;
        firstAlignment = 0;
    }

    // Footpath button pressed down
    if (window_find_by_class(WC_FOOTPATH) == nullptr)
        w->pressed_widgets &= ~(1 << WIDX_PATH);
    else
        w->pressed_widgets |= (1 << WIDX_PATH);

    if (gGamePaused & GAME_PAUSED_NORMAL)
        w->pressed_widgets |= (1 << WIDX_PAUSE);
    else
        w->pressed_widgets &= ~(1 << WIDX_PAUSE);

    if (!OpenRCT2::Audio::gGameSoundsOff)
        window_top_toolbar_widgets[WIDX_MUTE].image = IMAGE_TYPE_REMAP | SPR_G2_TOOLBAR_MUTE;
    else
        window_top_toolbar_widgets[WIDX_MUTE].image = IMAGE_TYPE_REMAP | SPR_G2_TOOLBAR_UNMUTE;

    // Set map button to the right image.
    if (window_top_toolbar_widgets[WIDX_MAP].type != WindowWidgetType::Empty)
    {
        static constexpr uint32_t imageIdByRotation[] = {
            SPR_G2_MAP_NORTH,
            SPR_G2_MAP_WEST,
            SPR_G2_MAP_SOUTH,
            SPR_G2_MAP_EAST,
        };

        uint32_t mapImageId = imageIdByRotation[get_current_rotation()];
        window_top_toolbar_widgets[WIDX_MAP].image = IMAGE_TYPE_REMAP | mapImageId;
    }

    // Zoomed out/in disable. Not sure where this code is in the original.
    const auto* mainWindow = window_get_main();
    if (mainWindow == nullptr || mainWindow->viewport == nullptr)
    {
        log_error("mainWindow or mainWindow->viewport is null!");
        return;
    }

    if (mainWindow->viewport->zoom == ZoomLevel::min())
    {
        w->disabled_widgets |= (1 << WIDX_ZOOM_IN);
    }
    else if (mainWindow->viewport->zoom >= ZoomLevel::max())
    {
        w->disabled_widgets |= (1 << WIDX_ZOOM_OUT);
    }
    else
    {
        w->disabled_widgets &= ~((1 << WIDX_ZOOM_IN) | (1 << WIDX_ZOOM_OUT));
    }
}

/**
 *
 *  rct2: 0x0066C8EC
 */
static void window_top_toolbar_paint(rct_window* w, rct_drawpixelinfo* dpi)
{
    int32_t imgId;

    WindowDrawWidgets(w, dpi);

    ScreenCoordsXY screenPos{};
    // Draw staff button image (setting masks to the staff colours)
    if (window_top_toolbar_widgets[WIDX_STAFF].type != WindowWidgetType::Empty)
    {
        screenPos = { w->windowPos.x + window_top_toolbar_widgets[WIDX_STAFF].left,
                      w->windowPos.y + window_top_toolbar_widgets[WIDX_STAFF].top };
        imgId = SPR_TOOLBAR_STAFF;
        if (WidgetIsPressed(w, WIDX_STAFF))
            imgId++;
        gfx_draw_sprite(dpi, ImageId(imgId, gStaffHandymanColour, gStaffMechanicColour), screenPos);
    }

    // Draw fast forward button
    if (window_top_toolbar_widgets[WIDX_FASTFORWARD].type != WindowWidgetType::Empty)
    {
        screenPos = { w->windowPos.x + window_top_toolbar_widgets[WIDX_FASTFORWARD].left + 0,
                      w->windowPos.y + window_top_toolbar_widgets[WIDX_FASTFORWARD].top + 0 };
        if (WidgetIsPressed(w, WIDX_FASTFORWARD))
            screenPos.y++;
        gfx_draw_sprite(dpi, ImageId(SPR_G2_FASTFORWARD), screenPos + ScreenCoordsXY{ 6, 3 });

        for (int32_t i = 0; i < gGameSpeed && gGameSpeed <= 4; i++)
        {
            gfx_draw_sprite(dpi, ImageId(SPR_G2_SPEED_ARROW), screenPos + ScreenCoordsXY{ 5 + i * 5, 15 });
        }
        for (int32_t i = 0; i < 3 && i < gGameSpeed - 4 && gGameSpeed >= 5; i++)
        {
            gfx_draw_sprite(dpi, ImageId(SPR_G2_HYPER_ARROW), screenPos + ScreenCoordsXY{ 5 + i * 6, 15 });
        }
    }

    // Draw cheats button
    if (window_top_toolbar_widgets[WIDX_CHEATS].type != WindowWidgetType::Empty)
    {
        screenPos = w->windowPos
            + ScreenCoordsXY{ window_top_toolbar_widgets[WIDX_CHEATS].left - 1,
                              window_top_toolbar_widgets[WIDX_CHEATS].top - 1 };
        if (WidgetIsPressed(w, WIDX_CHEATS))
            screenPos.y++;
        gfx_draw_sprite(dpi, ImageId(SPR_G2_SANDBOX), screenPos);

        // Draw an overlay if clearance checks are disabled
        if (gCheatsDisableClearanceChecks)
        {
            DrawTextBasic(
                dpi, screenPos + ScreenCoordsXY{ 26, 2 }, STR_OVERLAY_CLEARANCE_CHECKS_DISABLED, {},
                { COLOUR_DARK_ORANGE | COLOUR_FLAG_OUTLINE, TextAlignment::RIGHT });
        }
    }

    // Draw chat button
    if (window_top_toolbar_widgets[WIDX_CHAT].type != WindowWidgetType::Empty)
    {
        screenPos = w->windowPos
            + ScreenCoordsXY{ window_top_toolbar_widgets[WIDX_CHAT].left, window_top_toolbar_widgets[WIDX_CHAT].top - 2 };
        if (WidgetIsPressed(w, WIDX_CHAT))
            screenPos.y++;
        gfx_draw_sprite(dpi, ImageId(SPR_G2_CHAT), screenPos);
    }

    // Draw debug button
    if (window_top_toolbar_widgets[WIDX_DEBUG].type != WindowWidgetType::Empty)
    {
        screenPos = w->windowPos
            + ScreenCoordsXY{ window_top_toolbar_widgets[WIDX_DEBUG].left, window_top_toolbar_widgets[WIDX_DEBUG].top - 1 };
        if (WidgetIsPressed(w, WIDX_DEBUG))
            screenPos.y++;
        gfx_draw_sprite(dpi, ImageId(SPR_TAB_GEARS_0), screenPos);
    }

    // Draw research button
    if (window_top_toolbar_widgets[WIDX_RESEARCH].type != WindowWidgetType::Empty)
    {
        screenPos = w->windowPos
            + ScreenCoordsXY{ window_top_toolbar_widgets[WIDX_RESEARCH].left - 1,
                              window_top_toolbar_widgets[WIDX_RESEARCH].top };
        if (WidgetIsPressed(w, WIDX_RESEARCH))
            screenPos.y++;
        gfx_draw_sprite(dpi, ImageId(SPR_TAB_FINANCES_RESEARCH_0), screenPos);
    }

    // Draw finances button
    if (window_top_toolbar_widgets[WIDX_FINANCES].type != WindowWidgetType::Empty)
    {
        screenPos = w->windowPos
            + ScreenCoordsXY{ window_top_toolbar_widgets[WIDX_FINANCES].left + 3,
                              window_top_toolbar_widgets[WIDX_FINANCES].top + 1 };
        if (WidgetIsPressed(w, WIDX_FINANCES))
            screenPos.y++;
        gfx_draw_sprite(dpi, ImageId(SPR_FINANCE), screenPos);
    }

    // Draw news button
    if (window_top_toolbar_widgets[WIDX_NEWS].type != WindowWidgetType::Empty)
    {
        screenPos = w->windowPos
            + ScreenCoordsXY{ window_top_toolbar_widgets[WIDX_NEWS].left + 3, window_top_toolbar_widgets[WIDX_NEWS].top + 0 };
        if (WidgetIsPressed(w, WIDX_NEWS))
            screenPos.y++;
        gfx_draw_sprite(dpi, ImageId(SPR_G2_TAB_NEWS), screenPos);
    }

    // Draw network button
    if (window_top_toolbar_widgets[WIDX_NETWORK].type != WindowWidgetType::Empty)
    {
        screenPos = w->windowPos
            + ScreenCoordsXY{ window_top_toolbar_widgets[WIDX_NETWORK].left + 3,
                              window_top_toolbar_widgets[WIDX_NETWORK].top + 0 };
        if (WidgetIsPressed(w, WIDX_NETWORK))
            screenPos.y++;

        // Draw (de)sync icon.
        imgId = (network_is_desynchronised() ? SPR_G2_MULTIPLAYER_DESYNC : SPR_G2_MULTIPLAYER_SYNC);
        gfx_draw_sprite(dpi, ImageId(imgId), screenPos + ScreenCoordsXY{ 3, 11 });

        // Draw number of players.
        auto ft = Formatter();
        ft.Add<int32_t>(network_get_num_players());
        DrawTextBasic(
            dpi, screenPos + ScreenCoordsXY{ 23, 1 }, STR_COMMA16, ft,
            { COLOUR_WHITE | COLOUR_FLAG_OUTLINE, TextAlignment::RIGHT });
    }
}

/**
 *
 *  rct2: 0x006E3158
 */
static void repaint_scenery_tool_down(const ScreenCoordsXY& windowPos, rct_widgetindex widgetIndex)
{
    auto flags = EnumsToFlags(
        ViewportInteractionItem::Scenery, ViewportInteractionItem::Wall, ViewportInteractionItem::LargeScenery,
        ViewportInteractionItem::Banner);
    auto info = get_map_coordinates_from_pos(windowPos, flags);
    switch (info.SpriteType)
    {
        case ViewportInteractionItem::Scenery:
        {
            rct_scenery_entry* scenery_entry = info.Element->AsSmallScenery()->GetEntry();

            // If can't repaint
            if (!scenery_small_entry_has_flag(
                    scenery_entry, SMALL_SCENERY_FLAG_HAS_PRIMARY_COLOUR | SMALL_SCENERY_FLAG_HAS_GLASS))
                return;

            uint8_t quadrant = info.Element->AsSmallScenery()->GetSceneryQuadrant();
            auto repaintScenery = SmallScenerySetColourAction(
                { info.Loc, info.Element->GetBaseZ() }, quadrant, info.Element->AsSmallScenery()->GetEntryIndex(),
                gWindowSceneryPrimaryColour, gWindowScenerySecondaryColour);

            GameActions::Execute(&repaintScenery);
            break;
        }
        case ViewportInteractionItem::Wall:
        {
            rct_scenery_entry* scenery_entry = info.Element->AsWall()->GetEntry();

            // If can't repaint
            if (!(scenery_entry->wall.flags & (WALL_SCENERY_HAS_PRIMARY_COLOUR | WALL_SCENERY_HAS_GLASS)))
                return;

            auto repaintScenery = WallSetColourAction(
                { info.Loc, info.Element->GetBaseZ(), info.Element->GetDirection() }, gWindowSceneryPrimaryColour,
                gWindowScenerySecondaryColour, gWindowSceneryTertiaryColour);

            GameActions::Execute(&repaintScenery);
            break;
        }
        case ViewportInteractionItem::LargeScenery:
        {
            rct_scenery_entry* scenery_entry = info.Element->AsLargeScenery()->GetEntry();

            // If can't repaint
            if (!(scenery_entry->large_scenery.flags & LARGE_SCENERY_FLAG_HAS_PRIMARY_COLOUR))
                return;

            auto repaintScenery = LargeScenerySetColourAction(
                { info.Loc, info.Element->GetBaseZ(), info.Element->GetDirection() },
                info.Element->AsLargeScenery()->GetSequenceIndex(), gWindowSceneryPrimaryColour, gWindowScenerySecondaryColour);

            GameActions::Execute(&repaintScenery);
            break;
        }
        case ViewportInteractionItem::Banner:
        {
            auto banner = info.Element->AsBanner()->GetBanner();
            if (banner != nullptr)
            {
                auto scenery_entry = get_banner_entry(banner->type);
                if (scenery_entry->banner.flags & BANNER_ENTRY_FLAG_HAS_PRIMARY_COLOUR)
                {
                    auto repaintScenery = BannerSetColourAction(
                        { info.Loc, info.Element->GetBaseZ(), info.Element->AsBanner()->GetPosition() },
                        gWindowSceneryPrimaryColour);

                    GameActions::Execute(&repaintScenery);
                }
            }
            break;
        }
        default:
            return;
    }
}

static void scenery_eyedropper_tool_down(const ScreenCoordsXY& windowPos, rct_widgetindex widgetIndex)
{
    auto flags = EnumsToFlags(
        ViewportInteractionItem::Scenery, ViewportInteractionItem::Wall, ViewportInteractionItem::LargeScenery,
        ViewportInteractionItem::Banner, ViewportInteractionItem::FootpathItem);
    auto info = get_map_coordinates_from_pos(windowPos, flags);
    switch (info.SpriteType)
    {
        case ViewportInteractionItem::Scenery:
        {
            SmallSceneryElement* sceneryElement = info.Element->AsSmallScenery();
            auto entryIndex = sceneryElement->GetEntryIndex();
            rct_scenery_entry* sceneryEntry = get_small_scenery_entry(entryIndex);
            if (sceneryEntry != nullptr)
            {
                if (window_scenery_set_selected_item({ SCENERY_TYPE_SMALL, entryIndex }))
                {
                    gWindowSceneryRotation = sceneryElement->GetDirectionWithOffset(get_current_rotation());
                    gWindowSceneryPrimaryColour = sceneryElement->GetPrimaryColour();
                    gWindowScenerySecondaryColour = sceneryElement->GetSecondaryColour();
                    gWindowSceneryEyedropperEnabled = false;
                }
            }
            break;
        }
        case ViewportInteractionItem::Wall:
        {
            auto entryIndex = info.Element->AsWall()->GetEntryIndex();
            rct_scenery_entry* sceneryEntry = get_wall_entry(entryIndex);
            if (sceneryEntry != nullptr)
            {
                if (window_scenery_set_selected_item({ SCENERY_TYPE_WALL, entryIndex }))
                {
                    gWindowSceneryPrimaryColour = info.Element->AsWall()->GetPrimaryColour();
                    gWindowScenerySecondaryColour = info.Element->AsWall()->GetSecondaryColour();
                    gWindowSceneryTertiaryColour = info.Element->AsWall()->GetTertiaryColour();
                    gWindowSceneryEyedropperEnabled = false;
                }
            }
            break;
        }
        case ViewportInteractionItem::LargeScenery:
        {
            auto entryIndex = info.Element->AsLargeScenery()->GetEntryIndex();
            rct_scenery_entry* sceneryEntry = get_large_scenery_entry(entryIndex);
            if (sceneryEntry != nullptr)
            {
                if (window_scenery_set_selected_item({ SCENERY_TYPE_LARGE, entryIndex }))
                {
                    gWindowSceneryRotation = (get_current_rotation() + info.Element->GetDirection()) & 3;
                    gWindowSceneryPrimaryColour = info.Element->AsLargeScenery()->GetPrimaryColour();
                    gWindowScenerySecondaryColour = info.Element->AsLargeScenery()->GetSecondaryColour();
                    gWindowSceneryEyedropperEnabled = false;
                }
            }
            break;
        }
        case ViewportInteractionItem::Banner:
        {
            auto banner = info.Element->AsBanner()->GetBanner();
            if (banner != nullptr)
            {
                auto sceneryEntry = get_banner_entry(banner->type);
                if (sceneryEntry != nullptr)
                {
                    if (window_scenery_set_selected_item({ SCENERY_TYPE_BANNER, banner->type }))
                    {
                        gWindowSceneryEyedropperEnabled = false;
                    }
                }
            }
            break;
        }
        case ViewportInteractionItem::FootpathItem:
        {
            auto entryIndex = info.Element->AsPath()->GetAdditionEntryIndex();
            rct_scenery_entry* sceneryEntry = get_footpath_item_entry(entryIndex);
            if (sceneryEntry != nullptr)
            {
                if (window_scenery_set_selected_item({ SCENERY_TYPE_PATH_ITEM, entryIndex }))
                {
                    gWindowSceneryEyedropperEnabled = false;
                }
            }
            break;
        }
        default:
            break;
    }
}

static void sub_6E1F34_update_screen_coords_and_buttons_pressed(bool canRaiseItem, ScreenCoordsXY& screenPos)
{
    if (!canRaiseItem && !gCheatsDisableSupportLimits)
    {
        gSceneryCtrlPressed = false;
        gSceneryShiftPressed = false;
    }
    else
    {
        if (!gSceneryCtrlPressed)
        {
            if (InputTestPlaceObjectModifier(PLACE_OBJECT_MODIFIER_COPY_Z))
            {
                // CTRL pressed
                auto flags = EnumsToFlags(
                    ViewportInteractionItem::Terrain, ViewportInteractionItem::Ride, ViewportInteractionItem::Scenery,
                    ViewportInteractionItem::Footpath, ViewportInteractionItem::Wall, ViewportInteractionItem::LargeScenery);
                auto info = get_map_coordinates_from_pos(screenPos, flags);

                if (info.SpriteType != ViewportInteractionItem::None)
                {
                    gSceneryCtrlPressed = true;
                    gSceneryCtrlPressZ = info.Element->GetBaseZ();
                }
            }
        }
        else
        {
            if (!(InputTestPlaceObjectModifier(PLACE_OBJECT_MODIFIER_COPY_Z)))
            {
                // CTRL not pressed
                gSceneryCtrlPressed = false;
            }
        }

        if (!gSceneryShiftPressed)
        {
            if (InputTestPlaceObjectModifier(PLACE_OBJECT_MODIFIER_SHIFT_Z))
            {
                // SHIFT pressed
                gSceneryShiftPressed = true;
                gSceneryShiftPressX = screenPos.x;
                gSceneryShiftPressY = screenPos.y;
                gSceneryShiftPressZOffset = 0;
            }
        }
        else
        {
            if (InputTestPlaceObjectModifier(PLACE_OBJECT_MODIFIER_SHIFT_Z))
            {
                // SHIFT pressed
                gSceneryShiftPressZOffset = (gSceneryShiftPressY - screenPos.y + 4);
                // Scale delta by zoom to match mouse position.
                auto* mainWnd = window_get_main();
                if (mainWnd && mainWnd->viewport)
                {
                    gSceneryShiftPressZOffset = gSceneryShiftPressZOffset * mainWnd->viewport->zoom;
                }
                gSceneryShiftPressZOffset = floor2(gSceneryShiftPressZOffset, 8);

                screenPos.x = gSceneryShiftPressX;
                screenPos.y = gSceneryShiftPressY;
            }
            else
            {
                // SHIFT not pressed
                gSceneryShiftPressed = false;
            }
        }
    }
}

static void sub_6E1F34_small_scenery(
    const ScreenCoordsXY& sourceScreenPos, ObjectEntryIndex sceneryIndex, CoordsXY& gridPos, uint8_t* outQuadrant,
    Direction* outRotation)
{
    rct_window* w = window_find_by_class(WC_SCENERY);

    if (w == nullptr)
    {
        gridPos.setNull();
        return;
    }

    auto screenPos = sourceScreenPos;
    uint16_t maxPossibleHeight = (std::numeric_limits<decltype(TileElement::base_height)>::max() - 32) * ZoomLevel::max();
    bool can_raise_item = false;

    rct_scenery_entry* scenery = get_small_scenery_entry(sceneryIndex);
    maxPossibleHeight -= scenery->small_scenery.height;
    if (scenery_small_entry_has_flag(scenery, SMALL_SCENERY_FLAG_STACKABLE))
    {
        can_raise_item = true;
    }

    sub_6E1F34_update_screen_coords_and_buttons_pressed(can_raise_item, screenPos);

    // Small scenery
    if (!scenery_small_entry_has_flag(scenery, SMALL_SCENERY_FLAG_FULL_TILE))
    {
        uint8_t quadrant = 0;

        // If CTRL not pressed
        if (!gSceneryCtrlPressed)
        {
            auto gridCoords = screen_get_map_xy_quadrant(screenPos, &quadrant);
            if (!gridCoords)
            {
                gridPos.setNull();
                return;
            }
            gridPos = *gridCoords;

            gSceneryPlaceZ = 0;

            // If SHIFT pressed
            if (gSceneryShiftPressed)
            {
                auto* surfaceElement = map_get_surface_element_at(gridPos);

                if (surfaceElement == nullptr)
                {
                    gridPos.setNull();
                    return;
                }

                int16_t z = (surfaceElement->GetBaseZ()) & 0xFFF0;
                z += gSceneryShiftPressZOffset;

                z = std::clamp<int16_t>(z, 16, maxPossibleHeight);

                gSceneryPlaceZ = z;
            }
        }
        else
        {
            int16_t z = gSceneryCtrlPressZ;

            auto mapCoords = screen_get_map_xy_quadrant_with_z(screenPos, z, &quadrant);
            if (!mapCoords)
            {
                gridPos.setNull();
                return;
            }
            gridPos = *mapCoords;

            // If SHIFT pressed
            if (gSceneryShiftPressed)
            {
                z += gSceneryShiftPressZOffset;
            }

            z = std::clamp<int16_t>(z, 16, maxPossibleHeight);

            gSceneryPlaceZ = z;
        }

        if (gridPos.isNull())
            return;

        uint8_t rotation = gWindowSceneryRotation;

        if (!scenery_small_entry_has_flag(scenery, SMALL_SCENERY_FLAG_ROTATABLE))
        {
            rotation = util_rand() & 0xFF;
        }

        rotation -= get_current_rotation();
        rotation &= 0x3;

        if (gConfigGeneral.virtual_floor_style != VirtualFloorStyles::Off)
        {
            virtual_floor_set_height(gSceneryPlaceZ);
        }

        *outQuadrant = quadrant ^ 2;
        *outRotation = rotation;

        return;
    }

    // If CTRL not pressed
    if (!gSceneryCtrlPressed)
    {
        auto flags = EnumsToFlags(ViewportInteractionItem::Terrain, ViewportInteractionItem::Water);

        auto info = get_map_coordinates_from_pos(screenPos, flags);
        gridPos = info.Loc;

        if (info.SpriteType == ViewportInteractionItem::None)
        {
            gridPos.setNull();
            return;
        }

        // If CTRL and SHIFT not pressed
        gSceneryPlaceZ = 0;

        // If SHIFT pressed
        if (gSceneryShiftPressed)
        {
            auto surfaceElement = map_get_surface_element_at(gridPos);

            if (surfaceElement == nullptr)
            {
                gridPos.setNull();
                return;
            }

            int16_t z = (surfaceElement->GetBaseZ()) & 0xFFF0;
            z += gSceneryShiftPressZOffset;

            z = std::clamp<int16_t>(z, 16, maxPossibleHeight);

            gSceneryPlaceZ = z;
        }
    }
    else
    {
        int16_t z = gSceneryCtrlPressZ;
        auto coords = screen_get_map_xy_with_z(screenPos, z);
        if (coords)
        {
            gridPos = *coords;
        }
        else
        {
            gridPos.setNull();
        }
        // If SHIFT pressed
        if (gSceneryShiftPressed)
        {
            z += gSceneryShiftPressZOffset;
        }

        z = std::clamp<int16_t>(z, 16, maxPossibleHeight);

        gSceneryPlaceZ = z;
    }

    if (gridPos.isNull())
        return;

    gridPos = gridPos.ToTileStart();
    uint8_t rotation = gWindowSceneryRotation;

    if (!scenery_small_entry_has_flag(scenery, SMALL_SCENERY_FLAG_ROTATABLE))
    {
        rotation = util_rand() & 0xFF;
    }

    rotation -= get_current_rotation();
    rotation &= 0x3;

    if (gConfigGeneral.virtual_floor_style != VirtualFloorStyles::Off)
    {
        virtual_floor_set_height(gSceneryPlaceZ);
    }

    *outQuadrant = 0;
    *outRotation = rotation;
}

static void sub_6E1F34_path_item(
    const ScreenCoordsXY& sourceScreenPos, ObjectEntryIndex sceneryIndex, CoordsXY& gridPos, int32_t* outZ)
{
    rct_window* w = window_find_by_class(WC_SCENERY);

    if (w == nullptr)
    {
        gridPos.setNull();
        return;
    }

    auto screenPos = sourceScreenPos;
    sub_6E1F34_update_screen_coords_and_buttons_pressed(false, screenPos);

    // Path bits
    auto flags = EnumsToFlags(ViewportInteractionItem::Footpath, ViewportInteractionItem::FootpathItem);
    auto info = get_map_coordinates_from_pos(screenPos, flags);
    gridPos = info.Loc;

    if (info.SpriteType == ViewportInteractionItem::None)
    {
        gridPos.setNull();
        return;
    }

    if (gConfigGeneral.virtual_floor_style != VirtualFloorStyles::Off)
    {
        virtual_floor_set_height(gSceneryPlaceZ);
    }

    *outZ = info.Element->GetBaseZ();
}

static void sub_6E1F34_wall(
    const ScreenCoordsXY& sourceScreenPos, ObjectEntryIndex sceneryIndex, CoordsXY& gridPos, uint8_t* outEdges)
{
    rct_window* w = window_find_by_class(WC_SCENERY);

    if (w == nullptr)
    {
        gridPos.setNull();
        return;
    }

    auto screenPos = sourceScreenPos;
    uint16_t maxPossibleHeight = (std::numeric_limits<decltype(TileElement::base_height)>::max() - 32) * ZoomLevel::max();

    rct_scenery_entry* scenery_entry = get_wall_entry(sceneryIndex);
    if (scenery_entry)
    {
        maxPossibleHeight -= scenery_entry->wall.height;
    }

    sub_6E1F34_update_screen_coords_and_buttons_pressed(true, screenPos);

    // Walls
    uint8_t edge;
    // If CTRL not pressed
    if (!gSceneryCtrlPressed)
    {
        auto gridCoords = screen_get_map_xy_side(screenPos, &edge);
        if (!gridCoords)
        {
            gridPos.setNull();
            return;
        }
        gridPos = *gridCoords;

        gSceneryPlaceZ = 0;

        // If SHIFT pressed
        if (gSceneryShiftPressed)
        {
            auto* surfaceElement = map_get_surface_element_at(gridPos);

            if (surfaceElement == nullptr)
            {
                gridPos.setNull();
                return;
            }

            int16_t z = (surfaceElement->GetBaseZ()) & 0xFFF0;
            z += gSceneryShiftPressZOffset;

            z = std::clamp<int16_t>(z, 16, maxPossibleHeight);

            gSceneryPlaceZ = z;
        }
    }
    else
    {
        int16_t z = gSceneryCtrlPressZ;
        auto mapCoords = screen_get_map_xy_side_with_z(screenPos, z, &edge);
        if (!mapCoords)
        {
            gridPos.setNull();
            return;
        }
        gridPos = *mapCoords;

        // If SHIFT pressed
        if (gSceneryShiftPressed)
        {
            z += gSceneryShiftPressZOffset;
        }

        z = std::clamp<int16_t>(z, 16, maxPossibleHeight);

        gSceneryPlaceZ = z;
    }

    if (gridPos.isNull())
        return;

    if (gConfigGeneral.virtual_floor_style != VirtualFloorStyles::Off)
    {
        virtual_floor_set_height(gSceneryPlaceZ);
    }

    *outEdges = edge;
}

static void sub_6E1F34_large_scenery(
    const ScreenCoordsXY& sourceScreenPos, ObjectEntryIndex sceneryIndex, CoordsXY& gridPos, Direction* outDirection)
{
    rct_window* w = window_find_by_class(WC_SCENERY);

    if (w == nullptr)
    {
        gridPos.setNull();
        return;
    }

    auto screenPos = sourceScreenPos;
    uint16_t maxPossibleHeight = (std::numeric_limits<decltype(TileElement::base_height)>::max() - 32) * ZoomLevel::max();

    rct_scenery_entry* scenery_entry = get_large_scenery_entry(sceneryIndex);
    if (scenery_entry)
    {
        int16_t maxClearZ = 0;
        for (int32_t i = 0; scenery_entry->large_scenery.tiles[i].x_offset != -1; ++i)
        {
            maxClearZ = std::max<int16_t>(maxClearZ, scenery_entry->large_scenery.tiles[i].z_clearance);
        }
        maxPossibleHeight = std::max(0, maxPossibleHeight - maxClearZ);
    }

    sub_6E1F34_update_screen_coords_and_buttons_pressed(true, screenPos);

    // Large scenery
    // If CTRL not pressed
    if (!gSceneryCtrlPressed)
    {
        const CoordsXY mapCoords = ViewportInteractionGetTileStartAtCursor(screenPos);
        gridPos = mapCoords;

        if (gridPos.isNull())
            return;

        gSceneryPlaceZ = 0;

        // If SHIFT pressed
        if (gSceneryShiftPressed)
        {
            auto* surfaceElement = map_get_surface_element_at(gridPos);

            if (surfaceElement == nullptr)
            {
                gridPos.setNull();
                return;
            }

            int16_t z = (surfaceElement->GetBaseZ()) & 0xFFF0;
            z += gSceneryShiftPressZOffset;

            z = std::clamp<int16_t>(z, 16, maxPossibleHeight);

            gSceneryPlaceZ = z;
        }
    }
    else
    {
        int16_t z = gSceneryCtrlPressZ;
        auto coords = screen_get_map_xy_with_z(screenPos, z);
        if (coords)
        {
            gridPos = *coords;
        }
        else
        {
            gridPos.setNull();
        }

        // If SHIFT pressed
        if (gSceneryShiftPressed)
        {
            z += gSceneryShiftPressZOffset;
        }

        z = std::clamp<int16_t>(z, 16, maxPossibleHeight);

        gSceneryPlaceZ = z;
    }

    if (gridPos.isNull())
        return;

    gridPos = gridPos.ToTileStart();

    Direction rotation = gWindowSceneryRotation;
    rotation -= get_current_rotation();
    rotation &= 0x3;

    if (gConfigGeneral.virtual_floor_style != VirtualFloorStyles::Off)
    {
        virtual_floor_set_height(gSceneryPlaceZ);
    }

    *outDirection = rotation;
}

static void sub_6E1F34_banner(
    const ScreenCoordsXY& sourceScreenPos, ObjectEntryIndex sceneryIndex, CoordsXY& gridPos, int32_t* outZ,
    Direction* outDirection)
{
    rct_window* w = window_find_by_class(WC_SCENERY);

    if (w == nullptr)
    {
        gridPos.setNull();
        return;
    }

    auto screenPos = sourceScreenPos;
    sub_6E1F34_update_screen_coords_and_buttons_pressed(false, screenPos);

    // Banner
    auto flags = EnumsToFlags(ViewportInteractionItem::Footpath, ViewportInteractionItem::FootpathItem);
    auto info = get_map_coordinates_from_pos(screenPos, flags);
    gridPos = info.Loc;

    if (info.SpriteType == ViewportInteractionItem::None)
    {
        gridPos.setNull();
        return;
    }

    uint8_t rotation = gWindowSceneryRotation;
    rotation -= get_current_rotation();
    rotation &= 0x3;

    auto z = info.Element->GetBaseZ();

    if (info.Element->AsPath()->IsSloped())
    {
        if (rotation != direction_reverse(info.Element->AsPath()->GetSlopeDirection()))
        {
            z += (2 * COORDS_Z_STEP);
        }
    }

    if (gConfigGeneral.virtual_floor_style != VirtualFloorStyles::Off)
    {
        virtual_floor_set_height(gSceneryPlaceZ);
    }

    *outDirection = rotation;
    *outZ = z;
}

/**
 *
 *  rct2: 0x006E2CC6
 */
static void window_top_toolbar_scenery_tool_down(const ScreenCoordsXY& windowPos, rct_window* w, rct_widgetindex widgetIndex)
{
    scenery_remove_ghost_tool_placement();
    if (gWindowSceneryPaintEnabled & 1)
    {
        repaint_scenery_tool_down(windowPos, widgetIndex);
        return;
    }
    else if (gWindowSceneryEyedropperEnabled)
    {
        scenery_eyedropper_tool_down(windowPos, widgetIndex);
        return;
    }

    ScenerySelection selectedTab = gWindowSceneryTabSelections[gWindowSceneryActiveTabIndex];
    uint8_t sceneryType = selectedTab.SceneryType;
    uint16_t selectedScenery = selectedTab.EntryIndex;

    if (selectedTab.IsUndefined())
        return;

    CoordsXY gridPos;

    switch (sceneryType)
    {
        case SCENERY_TYPE_SMALL:
        {
            uint8_t quadrant;
            Direction rotation;
            sub_6E1F34_small_scenery(windowPos, selectedScenery, gridPos, &quadrant, &rotation);
            if (gridPos.isNull())
                return;

            int32_t quantity = 1;
            bool isCluster = gWindowSceneryScatterEnabled
                && (network_get_mode() != NETWORK_MODE_CLIENT
                    || network_can_perform_command(network_get_current_player_group_index(), -2));

            if (isCluster)
            {
                switch (gWindowSceneryScatterDensity)
                {
                    case ScatterToolDensity::LowDensity:
                        quantity = gWindowSceneryScatterSize;
                        break;

                    case ScatterToolDensity::MediumDensity:
                        quantity = gWindowSceneryScatterSize * 2;
                        break;

                    case ScatterToolDensity::HighDensity:
                        quantity = gWindowSceneryScatterSize * 3;
                        break;
                }
            }

            bool forceError = true;
            for (int32_t q = 0; q < quantity; q++)
            {
                int32_t zCoordinate = gSceneryPlaceZ;
                rct_scenery_entry* scenery = get_small_scenery_entry(selectedScenery);

                int16_t cur_grid_x = gridPos.x;
                int16_t cur_grid_y = gridPos.y;

                if (isCluster)
                {
                    if (!scenery_small_entry_has_flag(scenery, SMALL_SCENERY_FLAG_FULL_TILE))
                    {
                        quadrant = util_rand() & 3;
                    }

                    int16_t grid_x_offset = (util_rand() % gWindowSceneryScatterSize) - (gWindowSceneryScatterSize / 2);
                    int16_t grid_y_offset = (util_rand() % gWindowSceneryScatterSize) - (gWindowSceneryScatterSize / 2);
                    if (gWindowSceneryScatterSize % 2 == 0)
                    {
                        grid_x_offset += 1;
                        grid_y_offset += 1;
                    }
                    cur_grid_x += grid_x_offset * COORDS_XY_STEP;
                    cur_grid_y += grid_y_offset * COORDS_XY_STEP;

                    if (!scenery_small_entry_has_flag(scenery, SMALL_SCENERY_FLAG_ROTATABLE))
                    {
                        gSceneryPlaceRotation = (gSceneryPlaceRotation + 1) & 3;
                    }
                }

                uint8_t zAttemptRange = 1;
                if (gSceneryPlaceZ != 0 && gSceneryShiftPressed)
                {
                    zAttemptRange = 20;
                }

                auto success = GameActions::Status::Unknown;
                // Try find a valid z coordinate
                for (; zAttemptRange != 0; zAttemptRange--)
                {
                    auto smallSceneryPlaceAction = SmallSceneryPlaceAction(
                        { cur_grid_x, cur_grid_y, gSceneryPlaceZ, gSceneryPlaceRotation }, quadrant, selectedScenery,
                        gWindowSceneryPrimaryColour, gWindowScenerySecondaryColour);
                    auto res = GameActions::Query(&smallSceneryPlaceAction);
                    success = res->Error;
                    if (res->Error == GameActions::Status::Ok)
                    {
                        break;
                    }

                    if (res->Error == GameActions::Status::InsufficientFunds)
                    {
                        break;
                    }
                    if (zAttemptRange != 1)
                    {
                        gSceneryPlaceZ += 8;
                    }
                }

                // Actually place
                if (success == GameActions::Status::Ok || ((q + 1 == quantity) && forceError))
                {
                    auto smallSceneryPlaceAction = SmallSceneryPlaceAction(
                        { cur_grid_x, cur_grid_y, gSceneryPlaceZ, gSceneryPlaceRotation }, quadrant, selectedScenery,
                        gWindowSceneryPrimaryColour, gWindowScenerySecondaryColour);

                    smallSceneryPlaceAction.SetCallback([=](const GameAction* ga, const GameActions::Result* result) {
                        if (result->Error == GameActions::Status::Ok)
                        {
                            OpenRCT2::Audio::Play3D(OpenRCT2::Audio::SoundId::PlaceItem, result->Position);
                        }
                    });
                    auto res = GameActions::Execute(&smallSceneryPlaceAction);
                    if (res->Error == GameActions::Status::Ok)
                    {
                        forceError = false;
                    }

                    if (res->Error == GameActions::Status::InsufficientFunds)
                    {
                        break;
                    }
                }
                gSceneryPlaceZ = zCoordinate;
            }
            break;
        }
        case SCENERY_TYPE_PATH_ITEM:
        {
            int32_t z;
            sub_6E1F34_path_item(windowPos, selectedScenery, gridPos, &z);
            if (gridPos.isNull())
                return;

            auto footpathAdditionPlaceAction = FootpathAdditionPlaceAction({ gridPos, z }, selectedScenery + 1);

            footpathAdditionPlaceAction.SetCallback([](const GameAction* ga, const GameActions::Result* result) {
                if (result->Error != GameActions::Status::Ok)
                {
                    return;
                }
                OpenRCT2::Audio::Play3D(OpenRCT2::Audio::SoundId::PlaceItem, result->Position);
            });
            auto res = GameActions::Execute(&footpathAdditionPlaceAction);
            break;
        }
        case SCENERY_TYPE_WALL:
        {
            uint8_t edges;
            sub_6E1F34_wall(windowPos, selectedScenery, gridPos, &edges);
            if (gridPos.isNull())
                return;

            uint8_t zAttemptRange = 1;
            if (gSceneryPlaceZ != 0 && gSceneryShiftPressed)
            {
                zAttemptRange = 20;
            }

            for (; zAttemptRange != 0; zAttemptRange--)
            {
                auto wallPlaceAction = WallPlaceAction(
                    selectedScenery, { gridPos, gSceneryPlaceZ }, edges, gWindowSceneryPrimaryColour,
                    gWindowScenerySecondaryColour, gWindowSceneryTertiaryColour);

                auto res = GameActions::Query(&wallPlaceAction);
                if (res->Error == GameActions::Status::Ok)
                {
                    break;
                }

                if (auto message = res->ErrorMessage.AsStringId())
                {
                    if (*message == STR_NOT_ENOUGH_CASH_REQUIRES || *message == STR_CAN_ONLY_BUILD_THIS_ON_WATER)
                    {
                        break;
                    }
                }

                if (zAttemptRange != 1)
                {
                    gSceneryPlaceZ += 8;
                }
            }

            auto wallPlaceAction = WallPlaceAction(
                selectedScenery, { gridPos, gSceneryPlaceZ }, edges, gWindowSceneryPrimaryColour, gWindowScenerySecondaryColour,
                gWindowSceneryTertiaryColour);

            wallPlaceAction.SetCallback([](const GameAction* ga, const GameActions::Result* result) {
                if (result->Error == GameActions::Status::Ok)
                {
                    OpenRCT2::Audio::Play3D(OpenRCT2::Audio::SoundId::PlaceItem, result->Position);
                }
            });
            auto res = GameActions::Execute(&wallPlaceAction);
            break;
        }
        case SCENERY_TYPE_LARGE:
        {
            Direction direction;
            sub_6E1F34_large_scenery(windowPos, selectedScenery, gridPos, &direction);
            if (gridPos.isNull())
                return;

            uint8_t zAttemptRange = 1;
            if (gSceneryPlaceZ != 0 && gSceneryShiftPressed)
            {
                zAttemptRange = 20;
            }

            for (; zAttemptRange != 0; zAttemptRange--)
            {
                CoordsXYZD loc = { gridPos, gSceneryPlaceZ, direction };

                auto sceneryPlaceAction = LargeSceneryPlaceAction(
                    loc, selectedScenery, gWindowSceneryPrimaryColour, gWindowScenerySecondaryColour);

                auto res = GameActions::Query(&sceneryPlaceAction);
                if (res->Error == GameActions::Status::Ok)
                {
                    break;
                }

                if (auto message = res->ErrorMessage.AsStringId())
                {
                    if (*message == STR_NOT_ENOUGH_CASH_REQUIRES || *message == STR_CAN_ONLY_BUILD_THIS_ON_WATER)
                    {
                        break;
                    }
                }

                if (zAttemptRange != 1)
                {
                    gSceneryPlaceZ += 8;
                }
            }

            CoordsXYZD loc = { gridPos, gSceneryPlaceZ, direction };

            auto sceneryPlaceAction = LargeSceneryPlaceAction(
                loc, selectedScenery, gWindowSceneryPrimaryColour, gWindowScenerySecondaryColour);
            sceneryPlaceAction.SetCallback([=](const GameAction* ga, const GameActions::Result* result) {
                if (result->Error == GameActions::Status::Ok)
                {
                    OpenRCT2::Audio::Play3D(OpenRCT2::Audio::SoundId::PlaceItem, result->Position);
                }
                else
                {
                    OpenRCT2::Audio::Play3D(OpenRCT2::Audio::SoundId::Error, { loc.x, loc.y, gSceneryPlaceZ });
                }
            });
            auto res = GameActions::Execute(&sceneryPlaceAction);
            break;
        }
        case SCENERY_TYPE_BANNER:
        {
            int32_t z;
            Direction direction;
            sub_6E1F34_banner(windowPos, selectedScenery, gridPos, &z, &direction);
            if (gridPos.isNull())
                return;

            CoordsXYZD loc{ gridPos, z, direction };
            auto primaryColour = gWindowSceneryPrimaryColour;
            auto bannerIndex = create_new_banner(0);
            if (bannerIndex == BANNER_INDEX_NULL)
            {
                context_show_error(STR_CANT_POSITION_THIS_HERE, STR_TOO_MANY_BANNERS_IN_GAME, {});
                break;
            }
            auto bannerPlaceAction = BannerPlaceAction(loc, selectedScenery, bannerIndex, primaryColour);
            bannerPlaceAction.SetCallback([=](const GameAction* ga, const GameActions::Result* result) {
                if (result->Error == GameActions::Status::Ok)
                {
                    OpenRCT2::Audio::Play3D(OpenRCT2::Audio::SoundId::PlaceItem, result->Position);
                    context_open_detail_window(WD_BANNER, bannerIndex);
                }
            });
            GameActions::Execute(&bannerPlaceAction);
            break;
        }
    }
}

static uint8_t top_toolbar_tool_update_land_paint(const ScreenCoordsXY& screenPos)
{
    uint8_t state_changed = 0;

    map_invalidate_selection_rect();
    gMapSelectFlags &= ~MAP_SELECT_FLAG_ENABLE;

    auto mapTile = screen_get_map_xy(screenPos, nullptr);

    if (!mapTile)
    {
        if (gClearSceneryCost != MONEY32_UNDEFINED)
        {
            gClearSceneryCost = MONEY32_UNDEFINED;
            window_invalidate_by_class(WC_CLEAR_SCENERY);
        }
        return state_changed;
    }

    if (!(gMapSelectFlags & MAP_SELECT_FLAG_ENABLE))
    {
        gMapSelectFlags |= MAP_SELECT_FLAG_ENABLE;
        state_changed++;
    }

    if (gMapSelectType != MAP_SELECT_TYPE_FULL)
    {
        gMapSelectType = MAP_SELECT_TYPE_FULL;
        state_changed++;
    }

    int16_t tool_size = std::max<uint16_t>(1, gLandToolSize);
    int16_t tool_length = (tool_size - 1) * 32;

    // Move to tool bottom left
    mapTile->x -= (tool_size - 1) * 16;
    mapTile->y -= (tool_size - 1) * 16;
    mapTile = mapTile->ToTileStart();

    if (gMapSelectPositionA.x != mapTile->x)
    {
        gMapSelectPositionA.x = mapTile->x;
        state_changed++;
    }

    if (gMapSelectPositionA.y != mapTile->y)
    {
        gMapSelectPositionA.y = mapTile->y;
        state_changed++;
    }

    mapTile->x += tool_length;
    mapTile->y += tool_length;

    if (gMapSelectPositionB.x != mapTile->x)
    {
        gMapSelectPositionB.x = mapTile->x;
        state_changed++;
    }

    if (gMapSelectPositionB.y != mapTile->y)
    {
        gMapSelectPositionB.y = mapTile->y;
        state_changed++;
    }

    map_invalidate_selection_rect();
    return state_changed;
}

/**
 *
 *  rct2: 0x0068E213
 */
static void top_toolbar_tool_update_scenery_clear(const ScreenCoordsXY& screenPos)
{
    if (!top_toolbar_tool_update_land_paint(screenPos))
        return;

    auto action = GetClearAction();
    auto result = GameActions::Query(&action);
    auto cost = (result->Error == GameActions::Status::Ok ? result->Cost : MONEY32_UNDEFINED);
    if (gClearSceneryCost != cost)
    {
        gClearSceneryCost = cost;
        window_invalidate_by_class(WC_CLEAR_SCENERY);
    }
}

/**
 *
 *  rct2: 0x00664280
 */
static void top_toolbar_tool_update_land(const ScreenCoordsXY& screenPos)
{
    const bool mapCtrlPressed = InputTestPlaceObjectModifier(PLACE_OBJECT_MODIFIER_COPY_Z);

    map_invalidate_selection_rect();

    if (gCurrentToolId == Tool::UpDownArrow)
    {
        if (!(gMapSelectFlags & MAP_SELECT_FLAG_ENABLE))
            return;

        money32 lower_cost = selection_lower_land(0);
        money32 raise_cost = selection_raise_land(0);

        if (gLandToolRaiseCost != raise_cost || gLandToolLowerCost != lower_cost)
        {
            gLandToolRaiseCost = raise_cost;
            gLandToolLowerCost = lower_cost;
            window_invalidate_by_class(WC_LAND);
        }
        return;
    }

    int16_t tool_size = gLandToolSize;
    std::optional<CoordsXY> mapTile;
    uint8_t side{};

    gMapSelectFlags &= ~MAP_SELECT_FLAG_ENABLE;
    if (tool_size == 1)
    {
        int32_t selectionType;
        // Get selection type and map coordinates from mouse x,y position
        screen_pos_to_map_pos(screenPos, &selectionType);
        mapTile = screen_get_map_xy_side(screenPos, &side);

        if (!mapTile)
        {
            money32 lower_cost = MONEY32_UNDEFINED;
            money32 raise_cost = MONEY32_UNDEFINED;

            if (gLandToolRaiseCost != raise_cost || gLandToolLowerCost != lower_cost)
            {
                gLandToolRaiseCost = raise_cost;
                gLandToolLowerCost = lower_cost;
                window_invalidate_by_class(WC_LAND);
            }
            return;
        }

        uint8_t state_changed = 0;

        if (!(gMapSelectFlags & MAP_SELECT_FLAG_ENABLE))
        {
            gMapSelectFlags |= MAP_SELECT_FLAG_ENABLE;
            state_changed++;
        }

        if (gMapSelectType != selectionType)
        {
            gMapSelectType = selectionType;
            state_changed++;
        }

        if ((gMapSelectType != MAP_SELECT_TYPE_EDGE_0 + (side & 0xFF)) && mapCtrlPressed)
        {
            gMapSelectType = MAP_SELECT_TYPE_EDGE_0 + (side & 0xFF);
            state_changed++;
        }

        if (gMapSelectPositionA.x != mapTile->x)
        {
            gMapSelectPositionA.x = mapTile->x;
            state_changed++;
        }

        if (gMapSelectPositionA.y != mapTile->y)
        {
            gMapSelectPositionA.y = mapTile->y;
            state_changed++;
        }

        if (gMapSelectPositionB.x != mapTile->x)
        {
            gMapSelectPositionB.x = mapTile->x;
            state_changed++;
        }

        if (gMapSelectPositionB.y != mapTile->y)
        {
            gMapSelectPositionB.y = mapTile->y;
            state_changed++;
        }

        map_invalidate_selection_rect();
        if (!state_changed)
            return;

        money32 lower_cost = selection_lower_land(0);
        money32 raise_cost = selection_raise_land(0);

        if (gLandToolRaiseCost != raise_cost || gLandToolLowerCost != lower_cost)
        {
            gLandToolRaiseCost = raise_cost;
            gLandToolLowerCost = lower_cost;
            window_invalidate_by_class(WC_LAND);
        }
        return;
    }

    // Get map coordinates and the side of the tile that is being hovered over
    mapTile = screen_get_map_xy_side(screenPos, &side);

    if (!mapTile)
    {
        money32 lower_cost = MONEY32_UNDEFINED;
        money32 raise_cost = MONEY32_UNDEFINED;

        if (gLandToolRaiseCost != raise_cost || gLandToolLowerCost != lower_cost)
        {
            gLandToolRaiseCost = raise_cost;
            gLandToolLowerCost = lower_cost;
            window_invalidate_by_class(WC_LAND);
        }
        return;
    }

    uint8_t state_changed = 0;

    if (!(gMapSelectFlags & MAP_SELECT_FLAG_ENABLE))
    {
        gMapSelectFlags |= MAP_SELECT_FLAG_ENABLE;
        state_changed++;
    }

    if (gMapSelectType != MAP_SELECT_TYPE_FULL)
    {
        gMapSelectType = MAP_SELECT_TYPE_FULL;
        state_changed++;
    }

    if ((gMapSelectType != MAP_SELECT_TYPE_EDGE_0 + (side & 0xFF)) && mapCtrlPressed)
    {
        gMapSelectType = MAP_SELECT_TYPE_EDGE_0 + (side & 0xFF);
        state_changed++;
    }

    if (tool_size == 0)
        tool_size = 1;

    int16_t tool_length = (tool_size - 1) * 32;

    // Decide on shape of the brush for bigger selection size
    switch (gMapSelectType)
    {
        case MAP_SELECT_TYPE_EDGE_0:
        case MAP_SELECT_TYPE_EDGE_2:
            // Line
            mapTile->y -= (tool_size - 1) * 16;
            mapTile->y = mapTile->ToTileStart().y;
            break;
        case MAP_SELECT_TYPE_EDGE_1:
        case MAP_SELECT_TYPE_EDGE_3:
            // Line
            mapTile->x -= (tool_size - 1) * 16;
            mapTile->x = mapTile->ToTileStart().x;
            break;
        default:
            // Move to tool bottom left
            mapTile->x -= (tool_size - 1) * 16;
            mapTile->y -= (tool_size - 1) * 16;
            mapTile = mapTile->ToTileStart();
            break;
    }

    if (gMapSelectPositionA.x != mapTile->x)
    {
        gMapSelectPositionA.x = mapTile->x;
        state_changed++;
    }

    if (gMapSelectPositionA.y != mapTile->y)
    {
        gMapSelectPositionA.y = mapTile->y;
        state_changed++;
    }

    // Go to other side
    switch (gMapSelectType)
    {
        case MAP_SELECT_TYPE_EDGE_0:
        case MAP_SELECT_TYPE_EDGE_2:
            // Line
            mapTile->y += tool_length;
            gMapSelectType = MAP_SELECT_TYPE_FULL;
            break;
        case MAP_SELECT_TYPE_EDGE_1:
        case MAP_SELECT_TYPE_EDGE_3:
            // Line
            mapTile->x += tool_length;
            gMapSelectType = MAP_SELECT_TYPE_FULL;
            break;
        default:
            mapTile->x += tool_length;
            mapTile->y += tool_length;
            break;
    }

    if (gMapSelectPositionB.x != mapTile->x)
    {
        gMapSelectPositionB.x = mapTile->x;
        state_changed++;
    }

    if (gMapSelectPositionB.y != mapTile->y)
    {
        gMapSelectPositionB.y = mapTile->y;
        state_changed++;
    }

    map_invalidate_selection_rect();
    if (!state_changed)
        return;

    money32 lower_cost = selection_lower_land(0);
    money32 raise_cost = selection_raise_land(0);

    if (gLandToolRaiseCost != raise_cost || gLandToolLowerCost != lower_cost)
    {
        gLandToolRaiseCost = raise_cost;
        gLandToolLowerCost = lower_cost;
        window_invalidate_by_class(WC_LAND);
    }
}

/**
 *
 *  rct2: 0x006E6BDC
 */
static void top_toolbar_tool_update_water(const ScreenCoordsXY& screenPos)
{
    map_invalidate_selection_rect();

    if (gCurrentToolId == Tool::UpDownArrow)
    {
        if (!(gMapSelectFlags & MAP_SELECT_FLAG_ENABLE))
            return;

        auto waterLowerAction = WaterLowerAction(
            { gMapSelectPositionA.x, gMapSelectPositionA.y, gMapSelectPositionB.x, gMapSelectPositionB.y });
        auto waterRaiseAction = WaterRaiseAction(
            { gMapSelectPositionA.x, gMapSelectPositionA.y, gMapSelectPositionB.x, gMapSelectPositionB.y });

        auto res = GameActions::Query(&waterLowerAction);
        money32 lowerCost = res->Error == GameActions::Status::Ok ? res->Cost : MONEY32_UNDEFINED;

        res = GameActions::Query(&waterRaiseAction);
        money32 raiseCost = res->Error == GameActions::Status::Ok ? res->Cost : MONEY32_UNDEFINED;

        if (gWaterToolRaiseCost != raiseCost || gWaterToolLowerCost != lowerCost)
        {
            gWaterToolRaiseCost = raiseCost;
            gWaterToolLowerCost = lowerCost;
            window_invalidate_by_class(WC_WATER);
        }
        return;
    }

    gMapSelectFlags &= ~MAP_SELECT_FLAG_ENABLE;

    auto info = get_map_coordinates_from_pos(
        screenPos, EnumsToFlags(ViewportInteractionItem::Terrain, ViewportInteractionItem::Water));

    if (info.SpriteType == ViewportInteractionItem::None)
    {
        if (gWaterToolRaiseCost != MONEY32_UNDEFINED || gWaterToolLowerCost != MONEY32_UNDEFINED)
        {
            gWaterToolRaiseCost = MONEY32_UNDEFINED;
            gWaterToolLowerCost = MONEY32_UNDEFINED;
            window_invalidate_by_class(WC_WATER);
        }
        return;
    }

    auto mapTile = info.Loc.ToTileCentre();

    uint8_t state_changed = 0;

    if (!(gMapSelectFlags & MAP_SELECT_FLAG_ENABLE))
    {
        gMapSelectFlags |= MAP_SELECT_FLAG_ENABLE;
        state_changed++;
    }

    if (gMapSelectType != MAP_SELECT_TYPE_FULL_WATER)
    {
        gMapSelectType = MAP_SELECT_TYPE_FULL_WATER;
        state_changed++;
    }

    int16_t tool_size = std::max<uint16_t>(1, gLandToolSize);
    int16_t tool_length = (tool_size - 1) * 32;

    // Move to tool bottom left
    mapTile.x -= (tool_size - 1) * 16;
    mapTile.y -= (tool_size - 1) * 16;
    mapTile.x &= 0xFFE0;
    mapTile.y &= 0xFFE0;

    if (gMapSelectPositionA.x != mapTile.x)
    {
        gMapSelectPositionA.x = mapTile.x;
        state_changed++;
    }

    if (gMapSelectPositionA.y != mapTile.y)
    {
        gMapSelectPositionA.y = mapTile.y;
        state_changed++;
    }

    mapTile.x += tool_length;
    mapTile.y += tool_length;

    if (gMapSelectPositionB.x != mapTile.x)
    {
        gMapSelectPositionB.x = mapTile.x;
        state_changed++;
    }

    if (gMapSelectPositionB.y != mapTile.y)
    {
        gMapSelectPositionB.y = mapTile.y;
        state_changed++;
    }

    map_invalidate_selection_rect();
    if (!state_changed)
        return;

    auto waterLowerAction = WaterLowerAction(
        { gMapSelectPositionA.x, gMapSelectPositionA.y, gMapSelectPositionB.x, gMapSelectPositionB.y });
    auto waterRaiseAction = WaterRaiseAction(
        { gMapSelectPositionA.x, gMapSelectPositionA.y, gMapSelectPositionB.x, gMapSelectPositionB.y });

    auto res = GameActions::Query(&waterLowerAction);
    money32 lowerCost = res->Error == GameActions::Status::Ok ? res->Cost : MONEY32_UNDEFINED;

    res = GameActions::Query(&waterRaiseAction);
    money32 raiseCost = res->Error == GameActions::Status::Ok ? res->Cost : MONEY32_UNDEFINED;

    if (gWaterToolRaiseCost != raiseCost || gWaterToolLowerCost != lowerCost)
    {
        gWaterToolRaiseCost = raiseCost;
        gWaterToolLowerCost = lowerCost;
        window_invalidate_by_class(WC_WATER);
    }
}

/**
 *
 *  rct2: 0x006E24F6
 * On failure returns MONEY32_UNDEFINED
 * On success places ghost scenery and returns cost to place proper
 */
static money32 try_place_ghost_small_scenery(
    CoordsXYZD loc, uint8_t quadrant, ObjectEntryIndex entryIndex, colour_t primaryColour, colour_t secondaryColour)
{
    scenery_remove_ghost_tool_placement();

    // 6e252b
    auto smallSceneryPlaceAction = SmallSceneryPlaceAction(loc, quadrant, entryIndex, primaryColour, secondaryColour);
    smallSceneryPlaceAction.SetFlags(GAME_COMMAND_FLAG_GHOST | GAME_COMMAND_FLAG_ALLOW_DURING_PAUSED);
    auto res = GameActions::Execute(&smallSceneryPlaceAction);
    auto sspar = dynamic_cast<SmallSceneryPlaceActionResult*>(res.get());
    if (sspar == nullptr || res->Error != GameActions::Status::Ok)
        return MONEY32_UNDEFINED;

    gSceneryPlaceRotation = loc.direction;
    gSceneryPlaceObject.SceneryType = SCENERY_TYPE_SMALL;
    gSceneryPlaceObject.EntryIndex = entryIndex;

    TileElement* tileElement = sspar->tileElement;
    gSceneryGhostPosition = { loc, tileElement->GetBaseZ() };
    gSceneryQuadrant = tileElement->AsSmallScenery()->GetSceneryQuadrant();
    if (sspar->GroundFlags & ELEMENT_IS_UNDERGROUND)
    {
        // Set underground on
        viewport_set_visibility(4);
    }
    else
    {
        // Set underground off
        viewport_set_visibility(5);
    }

    gSceneryGhostType |= SCENERY_GHOST_FLAG_0;
    return res->Cost;
}

static money32 try_place_ghost_path_addition(CoordsXYZ loc, ObjectEntryIndex entryIndex)
{
    scenery_remove_ghost_tool_placement();

    // 6e265b
    auto footpathAdditionPlaceAction = FootpathAdditionPlaceAction(loc, entryIndex + 1);
    footpathAdditionPlaceAction.SetFlags(GAME_COMMAND_FLAG_GHOST | GAME_COMMAND_FLAG_ALLOW_DURING_PAUSED);
    footpathAdditionPlaceAction.SetCallback([=](const GameAction* ga, const GameActions::Result* result) {
        if (result->Error != GameActions::Status::Ok)
        {
            return;
        }
        gSceneryGhostPosition = loc;
        gSceneryGhostType |= SCENERY_GHOST_FLAG_1;
    });
    auto res = GameActions::Execute(&footpathAdditionPlaceAction);
    if (res->Error != GameActions::Status::Ok)
        return MONEY32_UNDEFINED;

    return res->Cost;
}

static money32 try_place_ghost_wall(
    CoordsXYZ loc, uint8_t edge, ObjectEntryIndex entryIndex, colour_t primaryColour, colour_t secondaryColour,
    colour_t tertiaryColour)
{
    scenery_remove_ghost_tool_placement();

    // 6e26b0
    auto wallPlaceAction = WallPlaceAction(entryIndex, loc, edge, primaryColour, secondaryColour, tertiaryColour);
    wallPlaceAction.SetFlags(GAME_COMMAND_FLAG_GHOST | GAME_COMMAND_FLAG_ALLOW_DURING_PAUSED | GAME_COMMAND_FLAG_NO_SPEND);
    wallPlaceAction.SetCallback([=](const GameAction* ga, const WallPlaceActionResult* result) {
        if (result->Error != GameActions::Status::Ok)
            return;

        gSceneryGhostPosition = { loc, result->tileElement->GetBaseZ() };
        gSceneryGhostWallRotation = edge;

        gSceneryGhostType |= SCENERY_GHOST_FLAG_2;
    });

    auto res = GameActions::Execute(&wallPlaceAction);
    if (res->Error != GameActions::Status::Ok)
        return MONEY32_UNDEFINED;

    return res->Cost;
}

static money32 try_place_ghost_large_scenery(
    CoordsXYZD loc, ObjectEntryIndex entryIndex, colour_t primaryColour, colour_t secondaryColour)
{
    scenery_remove_ghost_tool_placement();

    // 6e25a7
    auto sceneryPlaceAction = LargeSceneryPlaceAction(loc, entryIndex, primaryColour, secondaryColour);
    sceneryPlaceAction.SetFlags(GAME_COMMAND_FLAG_GHOST | GAME_COMMAND_FLAG_ALLOW_DURING_PAUSED | GAME_COMMAND_FLAG_NO_SPEND);
    auto res = GameActions::Execute(&sceneryPlaceAction);
    auto lspar = dynamic_cast<LargeSceneryPlaceActionResult*>(res.get());
    if (lspar == nullptr || res->Error != GameActions::Status::Ok)
        return MONEY32_UNDEFINED;

    gSceneryPlaceRotation = loc.direction;

    TileElement* tileElement = lspar->tileElement;
    gSceneryGhostPosition = { loc, tileElement->GetBaseZ() };

    if (lspar->GroundFlags & ELEMENT_IS_UNDERGROUND)
    {
        // Set underground on
        viewport_set_visibility(4);
    }
    else
    {
        // Set underground off
        viewport_set_visibility(5);
    }

    gSceneryGhostType |= SCENERY_GHOST_FLAG_3;
    return res->Cost;
}

static money32 try_place_ghost_banner(CoordsXYZD loc, ObjectEntryIndex entryIndex)
{
    scenery_remove_ghost_tool_placement();

    // 6e2612
    auto primaryColour = gWindowSceneryPrimaryColour;
    auto bannerIndex = create_new_banner(0);
    if (bannerIndex == BANNER_INDEX_NULL)
    {
        // Silently fail as this is just for the ghost
        return 0;
    }
    auto bannerPlaceAction = BannerPlaceAction(loc, entryIndex, bannerIndex, primaryColour);
    bannerPlaceAction.SetFlags(GAME_COMMAND_FLAG_GHOST | GAME_COMMAND_FLAG_ALLOW_DURING_PAUSED | GAME_COMMAND_FLAG_NO_SPEND);
    auto res = GameActions::Execute(&bannerPlaceAction);
    if (res->Error != GameActions::Status::Ok)
        return MONEY32_UNDEFINED;

    gSceneryGhostPosition = loc;
    gSceneryGhostPosition.z += PATH_HEIGHT_STEP;
    gSceneryPlaceRotation = loc.direction;
    gSceneryGhostType |= SCENERY_GHOST_FLAG_4;
    return res->Cost;
}

/**
 *
 *  rct2: 0x006E287B
 */
static void top_toolbar_tool_update_scenery(const ScreenCoordsXY& screenPos)
{
    map_invalidate_selection_rect();
    map_invalidate_map_selection_tiles();

    if (gConfigGeneral.virtual_floor_style != VirtualFloorStyles::Off)
    {
        virtual_floor_invalidate();
    }

    gMapSelectFlags &= ~MAP_SELECT_FLAG_ENABLE;
    gMapSelectFlags &= ~MAP_SELECT_FLAG_ENABLE_CONSTRUCT;

    if (gWindowSceneryPaintEnabled)
        return;
    if (gWindowSceneryEyedropperEnabled)
        return;

    ScenerySelection selection = gWindowSceneryTabSelections[gWindowSceneryActiveTabIndex];

    if (selection.IsUndefined())
    {
        scenery_remove_ghost_tool_placement();
        return;
    }

    money32 cost = 0;

    switch (selection.SceneryType)
    {
        case SCENERY_TYPE_SMALL:
        {
            CoordsXY mapTile = {};
            uint8_t quadrant;
            Direction rotation;

            sub_6E1F34_small_scenery(screenPos, selection.EntryIndex, mapTile, &quadrant, &rotation);

            if (mapTile.isNull())
            {
                scenery_remove_ghost_tool_placement();
                return;
            }

            gMapSelectFlags |= MAP_SELECT_FLAG_ENABLE;
            if (gWindowSceneryScatterEnabled)
            {
                uint16_t cluster_size = (gWindowSceneryScatterSize - 1) * COORDS_XY_STEP;
                gMapSelectPositionA.x = mapTile.x - cluster_size / 2;
                gMapSelectPositionA.y = mapTile.y - cluster_size / 2;
                gMapSelectPositionB.x = mapTile.x + cluster_size / 2;
                gMapSelectPositionB.y = mapTile.y + cluster_size / 2;
                if (gWindowSceneryScatterSize % 2 == 0)
                {
                    gMapSelectPositionB.x += COORDS_XY_STEP;
                    gMapSelectPositionB.y += COORDS_XY_STEP;
                }
            }
            else
            {
                gMapSelectPositionA.x = mapTile.x;
                gMapSelectPositionA.y = mapTile.y;
                gMapSelectPositionB.x = mapTile.x;
                gMapSelectPositionB.y = mapTile.y;
            }

            rct_scenery_entry* scenery = get_small_scenery_entry(selection.EntryIndex);

            gMapSelectType = MAP_SELECT_TYPE_FULL;
            if (!scenery_small_entry_has_flag(scenery, SMALL_SCENERY_FLAG_FULL_TILE) && !gWindowSceneryScatterEnabled)
            {
                gMapSelectType = MAP_SELECT_TYPE_QUARTER_0 + (quadrant ^ 2);
            }

            map_invalidate_selection_rect();

            // If no change in ghost placement
            if ((gSceneryGhostType & SCENERY_GHOST_FLAG_0) && mapTile == gSceneryGhostPosition && quadrant == _unkF64F0E
                && gSceneryPlaceZ == _unkF64F0A && gSceneryPlaceObject.SceneryType == SCENERY_TYPE_SMALL
                && gSceneryPlaceObject.EntryIndex == selection.EntryIndex)
            {
                return;
            }

            scenery_remove_ghost_tool_placement();

            _unkF64F0E = quadrant;
            _unkF64F0A = gSceneryPlaceZ;

            uint8_t attemptsLeft = 1;
            if (gSceneryPlaceZ != 0 && gSceneryShiftPressed)
            {
                attemptsLeft = 20;
            }

            for (; attemptsLeft != 0; attemptsLeft--)
            {
                cost = try_place_ghost_small_scenery(
                    { mapTile, gSceneryPlaceZ, rotation }, quadrant, selection.EntryIndex, gWindowSceneryPrimaryColour,
                    gWindowScenerySecondaryColour);

                if (cost != MONEY32_UNDEFINED)
                    break;
                gSceneryPlaceZ += 8;
            }

            gSceneryPlaceCost = cost;
            break;
        }
        case SCENERY_TYPE_PATH_ITEM:
        {
            CoordsXY mapTile = {};
            int32_t z;

            sub_6E1F34_path_item(screenPos, selection.EntryIndex, mapTile, &z);

            if (mapTile.isNull())
            {
                scenery_remove_ghost_tool_placement();
                return;
            }

            gMapSelectFlags |= MAP_SELECT_FLAG_ENABLE;
            gMapSelectPositionA.x = mapTile.x;
            gMapSelectPositionA.y = mapTile.y;
            gMapSelectPositionB.x = mapTile.x;
            gMapSelectPositionB.y = mapTile.y;
            gMapSelectType = MAP_SELECT_TYPE_FULL;

            map_invalidate_selection_rect();

            // If no change in ghost placement
            if ((gSceneryGhostType & SCENERY_GHOST_FLAG_1) && mapTile == gSceneryGhostPosition && z == gSceneryGhostPosition.z)
            {
                return;
            }

            scenery_remove_ghost_tool_placement();

            cost = try_place_ghost_path_addition({ mapTile, z }, selection.EntryIndex);

            gSceneryPlaceCost = cost;
            break;
        }
        case SCENERY_TYPE_WALL:
        {
            CoordsXY mapTile = {};
            uint8_t edge;

            sub_6E1F34_wall(screenPos, selection.EntryIndex, mapTile, &edge);

            if (mapTile.isNull())
            {
                scenery_remove_ghost_tool_placement();
                return;
            }

            gMapSelectFlags |= MAP_SELECT_FLAG_ENABLE;
            gMapSelectPositionA.x = mapTile.x;
            gMapSelectPositionA.y = mapTile.y;
            gMapSelectPositionB.x = mapTile.x;
            gMapSelectPositionB.y = mapTile.y;
            gMapSelectType = MAP_SELECT_TYPE_EDGE_0 + edge;

            map_invalidate_selection_rect();

            // If no change in ghost placement
            if ((gSceneryGhostType & SCENERY_GHOST_FLAG_2) && mapTile == gSceneryGhostPosition
                && edge == gSceneryGhostWallRotation && gSceneryPlaceZ == _unkF64F0A)
            {
                return;
            }

            scenery_remove_ghost_tool_placement();

            gSceneryGhostWallRotation = edge;
            _unkF64F0A = gSceneryPlaceZ;

            uint8_t attemptsLeft = 1;
            if (gSceneryPlaceZ != 0 && gSceneryShiftPressed)
            {
                attemptsLeft = 20;
            }

            cost = 0;
            for (; attemptsLeft != 0; attemptsLeft--)
            {
                cost = try_place_ghost_wall(
                    { mapTile, gSceneryPlaceZ }, edge, selection.EntryIndex, gWindowSceneryPrimaryColour,
                    gWindowScenerySecondaryColour, gWindowSceneryTertiaryColour);

                if (cost != MONEY32_UNDEFINED)
                    break;
                gSceneryPlaceZ += 8;
            }

            gSceneryPlaceCost = cost;
            break;
        }
        case SCENERY_TYPE_LARGE:
        {
            CoordsXY mapTile = {};
            Direction direction;

            sub_6E1F34_large_scenery(screenPos, selection.EntryIndex, mapTile, &direction);

            if (mapTile.isNull())
            {
                scenery_remove_ghost_tool_placement();
                return;
            }

            rct_scenery_entry* scenery = get_large_scenery_entry(selection.EntryIndex);
            gMapSelectionTiles.clear();

            for (rct_large_scenery_tile* tile = scenery->large_scenery.tiles;
                 tile->x_offset != static_cast<int16_t>(static_cast<uint16_t>(0xFFFF)); tile++)
            {
                CoordsXY tileLocation = { tile->x_offset, tile->y_offset };
                auto rotatedTileCoords = tileLocation.Rotate(direction);

                rotatedTileCoords.x += mapTile.x;
                rotatedTileCoords.y += mapTile.y;

                gMapSelectionTiles.push_back(rotatedTileCoords);
            }

            gMapSelectFlags |= MAP_SELECT_FLAG_ENABLE_CONSTRUCT;
            map_invalidate_map_selection_tiles();

            // If no change in ghost placement
            if ((gSceneryGhostType & SCENERY_GHOST_FLAG_3) && mapTile == gSceneryGhostPosition && gSceneryPlaceZ == _unkF64F0A
                && gSceneryPlaceObject.SceneryType == SCENERY_TYPE_LARGE
                && gSceneryPlaceObject.EntryIndex == selection.EntryIndex)
            {
                return;
            }

            scenery_remove_ghost_tool_placement();

            gSceneryPlaceObject.SceneryType = SCENERY_TYPE_LARGE;
            gSceneryPlaceObject.EntryIndex = selection.EntryIndex;
            _unkF64F0A = gSceneryPlaceZ;

            uint8_t attemptsLeft = 1;
            if (gSceneryPlaceZ != 0 && gSceneryShiftPressed)
            {
                attemptsLeft = 20;
            }

            cost = 0;
            for (; attemptsLeft != 0; attemptsLeft--)
            {
                cost = try_place_ghost_large_scenery(
                    { mapTile, gSceneryPlaceZ, direction }, selection.EntryIndex, gWindowSceneryPrimaryColour,
                    gWindowScenerySecondaryColour);

                if (cost != MONEY32_UNDEFINED)
                    break;
                gSceneryPlaceZ += COORDS_Z_STEP;
            }

            gSceneryPlaceCost = cost;
            break;
        }
        case SCENERY_TYPE_BANNER:
        {
            CoordsXY mapTile = {};
            Direction direction;
            int32_t z;

            sub_6E1F34_banner(screenPos, selection.EntryIndex, mapTile, &z, &direction);

            if (mapTile.isNull())
            {
                scenery_remove_ghost_tool_placement();
                return;
            }

            gMapSelectFlags |= MAP_SELECT_FLAG_ENABLE;
            gMapSelectPositionA.x = mapTile.x;
            gMapSelectPositionA.y = mapTile.y;
            gMapSelectPositionB.x = mapTile.x;
            gMapSelectPositionB.y = mapTile.y;
            gMapSelectType = MAP_SELECT_TYPE_FULL;

            map_invalidate_selection_rect();

            // If no change in ghost placement
            if ((gSceneryGhostType & SCENERY_GHOST_FLAG_4) && mapTile == gSceneryGhostPosition && z == gSceneryGhostPosition.z
                && direction == gSceneryPlaceRotation)
            {
                return;
            }

            scenery_remove_ghost_tool_placement();

            cost = try_place_ghost_banner({ mapTile, z, direction }, selection.EntryIndex);

            gSceneryPlaceCost = cost;
            break;
        }
    }
}

/**
 *
 *  rct2: 0x0066CB25
 */
static void window_top_toolbar_tool_update(rct_window* w, rct_widgetindex widgetIndex, const ScreenCoordsXY& screenCoords)
{
    switch (widgetIndex)
    {
        case WIDX_CLEAR_SCENERY:
            top_toolbar_tool_update_scenery_clear(screenCoords);
            break;
        case WIDX_LAND:
            if (gLandPaintMode)
                top_toolbar_tool_update_land_paint(screenCoords);
            else
                top_toolbar_tool_update_land(screenCoords);
            break;
        case WIDX_WATER:
            top_toolbar_tool_update_water(screenCoords);
            break;
        case WIDX_SCENERY:
            top_toolbar_tool_update_scenery(screenCoords);
            break;
#ifdef ENABLE_SCRIPTING
        default:
            auto& customTool = OpenRCT2::Scripting::ActiveCustomTool;
            if (customTool)
            {
                customTool->OnUpdate(screenCoords);
            }
            break;
#endif
    }
}

/**
 *
 *  rct2: 0x0066CB73
 */
static void window_top_toolbar_tool_down(rct_window* w, rct_widgetindex widgetIndex, const ScreenCoordsXY& screenCoords)
{
    switch (widgetIndex)
    {
        case WIDX_CLEAR_SCENERY:
            if (gMapSelectFlags & MAP_SELECT_FLAG_ENABLE)
            {
                auto action = GetClearAction();
                GameActions::Execute(&action);
                gCurrentToolId = Tool::Crosshair;
            }
            break;
        case WIDX_LAND:
            if (gMapSelectFlags & MAP_SELECT_FLAG_ENABLE)
            {
                auto surfaceSetStyleAction = SurfaceSetStyleAction(
                    { gMapSelectPositionA.x, gMapSelectPositionA.y, gMapSelectPositionB.x, gMapSelectPositionB.y },
                    gLandToolTerrainSurface, gLandToolTerrainEdge);

                GameActions::Execute(&surfaceSetStyleAction);

                gCurrentToolId = Tool::UpDownArrow;
            }
            else
            {
                _landToolBlocked = true;
            }
            break;
        case WIDX_WATER:
            if (gMapSelectFlags & MAP_SELECT_FLAG_ENABLE)
            {
                gCurrentToolId = Tool::UpDownArrow;
            }
            else
            {
                _landToolBlocked = true;
            }
            break;
        case WIDX_SCENERY:
            window_top_toolbar_scenery_tool_down(screenCoords, w, widgetIndex);
            break;
#ifdef ENABLE_SCRIPTING
        default:
            auto& customTool = OpenRCT2::Scripting::ActiveCustomTool;
            if (customTool)
            {
                customTool->OnDown(screenCoords);
            }
            break;
#endif
    }
}

/**
 *
 *  rct2: 0x006644DD
 */
static money32 selection_raise_land(uint8_t flags)
{
    int32_t centreX = (gMapSelectPositionA.x + gMapSelectPositionB.x) / 2;
    int32_t centreY = (gMapSelectPositionA.y + gMapSelectPositionB.y) / 2;
    centreX += 16;
    centreY += 16;

    if (gLandMountainMode)
    {
        auto landSmoothAction = LandSmoothAction(
            { centreX, centreY },
            { gMapSelectPositionA.x, gMapSelectPositionA.y, gMapSelectPositionB.x, gMapSelectPositionB.y }, gMapSelectType,
            false);
        auto res = (flags & GAME_COMMAND_FLAG_APPLY) ? GameActions::Execute(&landSmoothAction)
                                                     : GameActions::Query(&landSmoothAction);
        return res->Error == GameActions::Status::Ok ? res->Cost : MONEY32_UNDEFINED;
    }
    else
    {
        auto landRaiseAction = LandRaiseAction(
            { centreX, centreY },
            { gMapSelectPositionA.x, gMapSelectPositionA.y, gMapSelectPositionB.x, gMapSelectPositionB.y }, gMapSelectType);
        auto res = (flags & GAME_COMMAND_FLAG_APPLY) ? GameActions::Execute(&landRaiseAction)
                                                     : GameActions::Query(&landRaiseAction);

        return res->Error == GameActions::Status::Ok ? res->Cost : MONEY32_UNDEFINED;
    }
}

/**
 *
 *  rct2: 0x006645B3
 */
static money32 selection_lower_land(uint8_t flags)
{
    int32_t centreX = (gMapSelectPositionA.x + gMapSelectPositionB.x) / 2;
    int32_t centreY = (gMapSelectPositionA.y + gMapSelectPositionB.y) / 2;
    centreX += 16;
    centreY += 16;

    if (gLandMountainMode)
    {
        auto landSmoothAction = LandSmoothAction(
            { centreX, centreY },
            { gMapSelectPositionA.x, gMapSelectPositionA.y, gMapSelectPositionB.x, gMapSelectPositionB.y }, gMapSelectType,
            true);
        auto res = (flags & GAME_COMMAND_FLAG_APPLY) ? GameActions::Execute(&landSmoothAction)
                                                     : GameActions::Query(&landSmoothAction);
        return res->Error == GameActions::Status::Ok ? res->Cost : MONEY32_UNDEFINED;
    }
    else
    {
        auto landLowerAction = LandLowerAction(
            { centreX, centreY },
            { gMapSelectPositionA.x, gMapSelectPositionA.y, gMapSelectPositionB.x, gMapSelectPositionB.y }, gMapSelectType);
        auto res = (flags & GAME_COMMAND_FLAG_APPLY) ? GameActions::Execute(&landLowerAction)
                                                     : GameActions::Query(&landLowerAction);

        return res->Error == GameActions::Status::Ok ? res->Cost : MONEY32_UNDEFINED;
    }
}

/**
 *  part of window_top_toolbar_tool_drag(0x0066CB4E)
 *  rct2: 0x00664454
 */
static void window_top_toolbar_land_tool_drag(const ScreenCoordsXY& screenPos)
{
    rct_window* window = window_find_from_point(screenPos);
    if (!window)
        return;
    rct_widgetindex widget_index = window_find_widget_from_point(window, screenPos);
    if (widget_index == -1)
        return;
    rct_widget* widget = &window->widgets[widget_index];
    if (widget->type != WindowWidgetType::Viewport)
        return;
    rct_viewport* viewport = window->viewport;
    if (!viewport)
        return;

    int16_t tile_height = -16 / viewport->zoom;

    int32_t y_diff = screenPos.y - gInputDragLast.y;

    if (y_diff <= tile_height)
    {
        gInputDragLast.y += tile_height;

        selection_raise_land(GAME_COMMAND_FLAG_APPLY);

        gLandToolRaiseCost = MONEY32_UNDEFINED;
        gLandToolLowerCost = MONEY32_UNDEFINED;
    }
    else if (y_diff >= -tile_height)
    {
        gInputDragLast.y -= tile_height;

        selection_lower_land(GAME_COMMAND_FLAG_APPLY);

        gLandToolRaiseCost = MONEY32_UNDEFINED;
        gLandToolLowerCost = MONEY32_UNDEFINED;
    }
}

/**
 *  part of window_top_toolbar_tool_drag(0x0066CB4E)
 *  rct2: 0x006E6D4B
 */
static void window_top_toolbar_water_tool_drag(const ScreenCoordsXY& screenPos)
{
    rct_window* window = window_find_from_point(screenPos);
    if (!window)
        return;
    rct_widgetindex widget_index = window_find_widget_from_point(window, screenPos);
    if (widget_index == -1)
        return;
    rct_widget* widget = &window->widgets[widget_index];
    if (widget->type != WindowWidgetType::Viewport)
        return;
    rct_viewport* viewport = window->viewport;
    if (!viewport)
        return;

    int16_t dx = -16 / viewport->zoom;

    auto offsetPos = screenPos - ScreenCoordsXY{ 0, gInputDragLast.y };

    if (offsetPos.y <= dx)
    {
        gInputDragLast.y += dx;

        auto waterRaiseAction = WaterRaiseAction(
            { gMapSelectPositionA.x, gMapSelectPositionA.y, gMapSelectPositionB.x, gMapSelectPositionB.y });
        GameActions::Execute(&waterRaiseAction);

        gWaterToolRaiseCost = MONEY32_UNDEFINED;
        gWaterToolLowerCost = MONEY32_UNDEFINED;

        return;
    }

    dx = -dx;

    if (offsetPos.y >= dx)
    {
        gInputDragLast.y += dx;

        auto waterLowerAction = WaterLowerAction(
            { gMapSelectPositionA.x, gMapSelectPositionA.y, gMapSelectPositionB.x, gMapSelectPositionB.y });
        GameActions::Execute(&waterLowerAction);
        gWaterToolRaiseCost = MONEY32_UNDEFINED;
        gWaterToolLowerCost = MONEY32_UNDEFINED;

        return;
    }
}

/**
 *
 *  rct2: 0x0066CB4E
 */
static void window_top_toolbar_tool_drag(rct_window* w, rct_widgetindex widgetIndex, const ScreenCoordsXY& screenCoords)
{
    switch (widgetIndex)
    {
        case WIDX_CLEAR_SCENERY:
            if (window_find_by_class(WC_ERROR) == nullptr && (gMapSelectFlags & MAP_SELECT_FLAG_ENABLE))
            {
                auto action = GetClearAction();
                GameActions::Execute(&action);
                gCurrentToolId = Tool::Crosshair;
            }
            break;
        case WIDX_LAND:
            // Custom setting to only change land style instead of raising or lowering land
            if (gLandPaintMode)
            {
                if (gMapSelectFlags & MAP_SELECT_FLAG_ENABLE)
                {
                    auto surfaceSetStyleAction = SurfaceSetStyleAction(
                        { gMapSelectPositionA.x, gMapSelectPositionA.y, gMapSelectPositionB.x, gMapSelectPositionB.y },
                        gLandToolTerrainSurface, gLandToolTerrainEdge);

                    GameActions::Execute(&surfaceSetStyleAction);

                    // The tool is set to 12 here instead of 3 so that the dragging cursor is not the elevation change
                    // cursor
                    gCurrentToolId = Tool::Crosshair;
                }
            }
            else
            {
                if (!_landToolBlocked)
                {
                    window_top_toolbar_land_tool_drag(screenCoords);
                }
            }
            break;
        case WIDX_WATER:
            if (!_landToolBlocked)
            {
                window_top_toolbar_water_tool_drag(screenCoords);
            }
            break;
        case WIDX_SCENERY:
            if (gWindowSceneryPaintEnabled & 1)
                window_top_toolbar_scenery_tool_down(screenCoords, w, widgetIndex);
            if (gWindowSceneryEyedropperEnabled)
                window_top_toolbar_scenery_tool_down(screenCoords, w, widgetIndex);
            break;
#ifdef ENABLE_SCRIPTING
        default:
            auto& customTool = OpenRCT2::Scripting::ActiveCustomTool;
            if (customTool)
            {
                customTool->OnDrag(screenCoords);
            }
            break;
#endif
    }
}

/**
 *
 *  rct2: 0x0066CC5B
 */
static void window_top_toolbar_tool_up(rct_window* w, rct_widgetindex widgetIndex, const ScreenCoordsXY& screenCoords)
{
    _landToolBlocked = false;
    switch (widgetIndex)
    {
        case WIDX_LAND:
            map_invalidate_selection_rect();
            gMapSelectFlags &= ~MAP_SELECT_FLAG_ENABLE;
            gCurrentToolId = Tool::DigDown;
            break;
        case WIDX_WATER:
            map_invalidate_selection_rect();
            gMapSelectFlags &= ~MAP_SELECT_FLAG_ENABLE;
            gCurrentToolId = Tool::WaterDown;
            break;
        case WIDX_CLEAR_SCENERY:
            map_invalidate_selection_rect();
            gMapSelectFlags &= ~MAP_SELECT_FLAG_ENABLE;
            gCurrentToolId = Tool::Crosshair;
            break;
#ifdef ENABLE_SCRIPTING
        default:
            auto& customTool = OpenRCT2::Scripting::ActiveCustomTool;
            if (customTool)
            {
                customTool->OnUp(screenCoords);
            }
            break;
#endif
    }
}

/**
 *
 *  rct2: 0x0066CA58
 */
static void window_top_toolbar_tool_abort(rct_window* w, rct_widgetindex widgetIndex)
{
    switch (widgetIndex)
    {
        case WIDX_LAND:
        case WIDX_WATER:
        case WIDX_CLEAR_SCENERY:
            hide_gridlines();
            break;
#ifdef ENABLE_SCRIPTING
        default:
            auto& customTool = OpenRCT2::Scripting::ActiveCustomTool;
            if (customTool)
            {
                customTool->OnAbort();
                customTool = {};
            }
            break;
#endif
    }
}

static void top_toolbar_init_map_menu(rct_window* w, rct_widget* widget)
{
    auto i = 0;
    gDropdownItemsFormat[i++] = STR_SHORTCUT_SHOW_MAP;
    gDropdownItemsFormat[i++] = STR_EXTRA_VIEWPORT;
    if ((gScreenFlags & SCREEN_FLAGS_SCENARIO_EDITOR) && gS6Info.editor_step == EditorStep::LandscapeEditor)
    {
        gDropdownItemsFormat[i++] = STR_MAPGEN_WINDOW_TITLE;
    }

#ifdef ENABLE_SCRIPTING
    const auto& customMenuItems = OpenRCT2::Scripting::CustomMenuItems;
    if (!customMenuItems.empty())
    {
        gDropdownItemsFormat[i++] = STR_EMPTY;
        for (const auto& item : customMenuItems)
        {
            gDropdownItemsFormat[i] = STR_STRING;
            auto sz = item.Text.c_str();
            std::memcpy(&gDropdownItemsArgs[i], &sz, sizeof(const char*));
            i++;
        }
    }
#endif

    WindowDropdownShowText(
        { w->windowPos.x + widget->left, w->windowPos.y + widget->top }, widget->height() + 1, w->colours[1] | 0x80, 0, i);
    gDropdownDefaultIndex = DDIDX_SHOW_MAP;
}

static void top_toolbar_map_menu_dropdown(int16_t dropdownIndex)
{
    int32_t customStartIndex = 3;
    if ((gScreenFlags & SCREEN_FLAGS_SCENARIO_EDITOR) && gS6Info.editor_step == EditorStep::LandscapeEditor)
    {
        customStartIndex++;
    }

    if (dropdownIndex < customStartIndex)
    {
        switch (dropdownIndex)
        {
            case 0:
                context_open_window(WC_MAP);
                break;
            case 1:
                context_open_window(WC_VIEWPORT);
                break;
            case 2:
                context_open_window(WC_MAPGEN);
                break;
        }
    }
    else
    {
#ifdef ENABLE_SCRIPTING
        const auto& customMenuItems = OpenRCT2::Scripting::CustomMenuItems;
        auto customIndex = static_cast<size_t>(dropdownIndex - customStartIndex);
        if (customMenuItems.size() > customIndex)
        {
            customMenuItems[customIndex].Invoke();
        }
#endif
    }
}

static void top_toolbar_init_fastforward_menu(rct_window* w, rct_widget* widget)
{
    int32_t num_items = 4;
    gDropdownItemsFormat[0] = STR_TOGGLE_OPTION;
    gDropdownItemsFormat[1] = STR_TOGGLE_OPTION;
    gDropdownItemsFormat[2] = STR_TOGGLE_OPTION;
    gDropdownItemsFormat[3] = STR_TOGGLE_OPTION;
    if (gConfigGeneral.debugging_tools)
    {
        gDropdownItemsFormat[4] = STR_EMPTY;
        gDropdownItemsFormat[5] = STR_TOGGLE_OPTION;
        gDropdownItemsArgs[5] = STR_SPEED_HYPER;
        num_items = 6;
    }

    gDropdownItemsArgs[0] = STR_SPEED_NORMAL;
    gDropdownItemsArgs[1] = STR_SPEED_QUICK;
    gDropdownItemsArgs[2] = STR_SPEED_FAST;
    gDropdownItemsArgs[3] = STR_SPEED_TURBO;

    WindowDropdownShowText(
        { w->windowPos.x + widget->left, w->windowPos.y + widget->top }, widget->height() + 1, w->colours[0] | 0x80, 0,
        num_items);

    // Set checkmarks
    if (gGameSpeed <= 4)
    {
        Dropdown::SetChecked(gGameSpeed - 1, true);
    }
    if (gGameSpeed == 8)
    {
        Dropdown::SetChecked(5, true);
    }

    if (gConfigGeneral.debugging_tools)
    {
        gDropdownDefaultIndex = (gGameSpeed == 8 ? 0 : gGameSpeed);
    }
    else
    {
        gDropdownDefaultIndex = (gGameSpeed >= 4 ? 0 : gGameSpeed);
    }
    if (gDropdownDefaultIndex == 4)
    {
        gDropdownDefaultIndex = 5;
    }
}

static void top_toolbar_fastforward_menu_dropdown(int16_t dropdownIndex)
{
    rct_window* w = window_get_main();
    if (w)
    {
        if (dropdownIndex >= 0 && dropdownIndex <= 5)
        {
            gGameSpeed = dropdownIndex + 1;
            if (gGameSpeed >= 5)
                gGameSpeed = 8;
            w->Invalidate();
        }
    }
}

static void top_toolbar_init_rotate_menu(rct_window* w, rct_widget* widget)
{
    gDropdownItemsFormat[0] = STR_ROTATE_CLOCKWISE;
    gDropdownItemsFormat[1] = STR_ROTATE_ANTI_CLOCKWISE;

    WindowDropdownShowText(
        { w->windowPos.x + widget->left, w->windowPos.y + widget->top }, widget->height() + 1, w->colours[1] | 0x80, 0, 2);

    gDropdownDefaultIndex = DDIDX_ROTATE_CLOCKWISE;
}

static void top_toolbar_rotate_menu_dropdown(int16_t dropdownIndex)
{
    rct_window* w = window_get_main();
    if (w)
    {
        if (dropdownIndex == 0)
        {
            window_rotate_camera(w, 1);
            w->Invalidate();
        }
        else if (dropdownIndex == 1)
        {
            window_rotate_camera(w, -1);
            w->Invalidate();
        }
    }
}

static void top_toolbar_init_cheats_menu(rct_window* w, rct_widget* widget)
{
    using namespace Dropdown;

    constexpr Item items[] = {
        ToggleOption(DDIDX_CHEATS, STR_CHEAT_TITLE),
        ToggleOption(DDIDX_TILE_INSPECTOR, STR_DEBUG_DROPDOWN_TILE_INSPECTOR),
        ToggleOption(DDIDX_OBJECT_SELECTION, STR_DEBUG_DROPDOWN_OBJECT_SELECTION),
        ToggleOption(DDIDX_INVENTIONS_LIST, STR_DEBUG_DROPDOWN_INVENTIONS_LIST),
        ToggleOption(DDIDX_SCENARIO_OPTIONS, STR_DEBUG_DROPDOWN_SCENARIO_OPTIONS),
        ToggleOption(DDIDX_OBJECTIVE_OPTIONS, STR_CHEATS_MENU_OBJECTIVE_OPTIONS),
        Separator(),
        ToggleOption(DDIDX_ENABLE_SANDBOX_MODE, STR_ENABLE_SANDBOX_MODE),
        ToggleOption(DDIDX_DISABLE_CLEARANCE_CHECKS, STR_DISABLE_CLEARANCE_CHECKS),
        ToggleOption(DDIDX_DISABLE_SUPPORT_LIMITS, STR_DISABLE_SUPPORT_LIMITS),
    };
    static_assert(ItemIDsMatchIndices(items));

    SetItems(items);

    WindowDropdownShowText(
        { w->windowPos.x + widget->left, w->windowPos.y + widget->top }, widget->height() + 1, w->colours[0] | 0x80, 0,
        TOP_TOOLBAR_CHEATS_COUNT);

    // Disable items that are not yet available in multiplayer
    if (network_get_mode() != NETWORK_MODE_NONE)
    {
        Dropdown::SetDisabled(DDIDX_OBJECT_SELECTION, true);
        Dropdown::SetDisabled(DDIDX_INVENTIONS_LIST, true);
    }

    if (gScreenFlags & SCREEN_FLAGS_EDITOR)
    {
        Dropdown::SetDisabled(DDIDX_OBJECT_SELECTION, true);
        Dropdown::SetDisabled(DDIDX_INVENTIONS_LIST, true);
        Dropdown::SetDisabled(DDIDX_SCENARIO_OPTIONS, true);
        Dropdown::SetDisabled(DDIDX_OBJECTIVE_OPTIONS, true);
        Dropdown::SetDisabled(DDIDX_ENABLE_SANDBOX_MODE, true);
    }

    if (gCheatsSandboxMode)
    {
        Dropdown::SetChecked(DDIDX_ENABLE_SANDBOX_MODE, true);
    }
    if (gCheatsDisableClearanceChecks)
    {
        Dropdown::SetChecked(DDIDX_DISABLE_CLEARANCE_CHECKS, true);
    }
    if (gCheatsDisableSupportLimits)
    {
        Dropdown::SetChecked(DDIDX_DISABLE_SUPPORT_LIMITS, true);
    }

    gDropdownDefaultIndex = DDIDX_CHEATS;
}

static void top_toolbar_cheats_menu_dropdown(int16_t dropdownIndex)
{
    switch (dropdownIndex)
    {
        case DDIDX_CHEATS:
            context_open_window(WC_CHEATS);
            break;
        case DDIDX_TILE_INSPECTOR:
            context_open_window(WC_TILE_INSPECTOR);
            break;
        case DDIDX_OBJECT_SELECTION:
            window_close_all();
            context_open_window(WC_EDITOR_OBJECT_SELECTION);
            break;
        case DDIDX_INVENTIONS_LIST:
            context_open_window(WC_EDITOR_INVENTION_LIST);
            break;
        case DDIDX_SCENARIO_OPTIONS:
            context_open_window(WC_EDITOR_SCENARIO_OPTIONS);
            break;
        case DDIDX_OBJECTIVE_OPTIONS:
            context_open_window(WC_EDITOR_OBJECTIVE_OPTIONS);
            break;
        case DDIDX_ENABLE_SANDBOX_MODE:
            CheatsSet(CheatType::SandboxMode, !gCheatsSandboxMode);
            break;
        case DDIDX_DISABLE_CLEARANCE_CHECKS:
            CheatsSet(CheatType::DisableClearanceChecks, !gCheatsDisableClearanceChecks);
            break;
        case DDIDX_DISABLE_SUPPORT_LIMITS:
            CheatsSet(CheatType::DisableSupportLimits, !gCheatsDisableSupportLimits);
            break;
    }
}

static void top_toolbar_init_debug_menu(rct_window* w, rct_widget* widget)
{
    gDropdownItemsFormat[DDIDX_CONSOLE] = STR_TOGGLE_OPTION;
    gDropdownItemsArgs[DDIDX_CONSOLE] = STR_DEBUG_DROPDOWN_CONSOLE;
    gDropdownItemsFormat[DDIDX_DEBUG_PAINT] = STR_TOGGLE_OPTION;
    gDropdownItemsArgs[DDIDX_DEBUG_PAINT] = STR_DEBUG_DROPDOWN_DEBUG_PAINT;

    WindowDropdownShowText(
        { w->windowPos.x + widget->left, w->windowPos.y + widget->top }, widget->height() + 1, w->colours[0] | 0x80,
        Dropdown::Flag::StayOpen, TOP_TOOLBAR_DEBUG_COUNT);

    Dropdown::SetChecked(DDIDX_DEBUG_PAINT, window_find_by_class(WC_DEBUG_PAINT) != nullptr);
}

static void top_toolbar_init_network_menu(rct_window* w, rct_widget* widget)
{
    gDropdownItemsFormat[DDIDX_MULTIPLAYER] = STR_MULTIPLAYER;
    gDropdownItemsFormat[DDIDX_MULTIPLAYER_RECONNECT] = STR_MULTIPLAYER_RECONNECT;

    WindowDropdownShowText(
        { w->windowPos.x + widget->left, w->windowPos.y + widget->top }, widget->height() + 1, w->colours[0] | 0x80, 0,
        TOP_TOOLBAR_NETWORK_COUNT);

    Dropdown::SetDisabled(DDIDX_MULTIPLAYER_RECONNECT, !network_is_desynchronised());

    gDropdownDefaultIndex = DDIDX_MULTIPLAYER;
}

static void top_toolbar_debug_menu_dropdown(int16_t dropdownIndex)
{
    rct_window* w = window_get_main();
    if (w)
    {
        switch (dropdownIndex)
        {
            case DDIDX_CONSOLE:
            {
                auto& console = GetInGameConsole();
                console.Open();
                break;
            }
            case DDIDX_DEBUG_PAINT:
                if (window_find_by_class(WC_DEBUG_PAINT) == nullptr)
                {
                    context_open_window(WC_DEBUG_PAINT);
                }
                else
                {
                    window_close_by_class(WC_DEBUG_PAINT);
                }
                break;
        }
    }
}

static void top_toolbar_network_menu_dropdown(int16_t dropdownIndex)
{
    rct_window* w = window_get_main();
    if (w)
    {
        switch (dropdownIndex)
        {
            case DDIDX_MULTIPLAYER:
                context_open_window(WC_MULTIPLAYER);
                break;
            case DDIDX_MULTIPLAYER_RECONNECT:
                network_reconnect();
                break;
        }
    }
}

/**
 *
 *  rct2: 0x0066CDE4
 */
static void top_toolbar_init_view_menu(rct_window* w, rct_widget* widget)
{
    using namespace Dropdown;
    constexpr Item items[] = {
        ToggleOption(DDIDX_UNDERGROUND_INSIDE, STR_UNDERGROUND_VIEW),
        ToggleOption(DDIDX_TRANSPARENT_WATER, STR_VIEWPORT_TRANSPARENT_WATER),
        ToggleOption(DDIDX_HIDE_BASE, STR_REMOVE_BASE_LAND),
        ToggleOption(DDIDX_HIDE_VERTICAL, STR_REMOVE_VERTICAL_FACES),
        Separator(),
        ToggleOption(DDIDX_SEETHROUGH_RIDES, STR_SEE_THROUGH_RIDES),
        ToggleOption(DDIDX_SEETHROUGH_SCENERY, STR_SEE_THROUGH_SCENERY),
        ToggleOption(DDIDX_SEETHROUGH_PATHS, STR_SEE_THROUGH_PATHS),
        ToggleOption(DDIDX_INVISIBLE_SUPPORTS, STR_INVISIBLE_SUPPORTS),
        ToggleOption(DDIDX_INVISIBLE_PEEPS, STR_INVISIBLE_PEOPLE),
        Separator(),
        ToggleOption(DDIDX_LAND_HEIGHTS, STR_HEIGHT_MARKS_ON_LAND),
        ToggleOption(DDIDX_TRACK_HEIGHTS, STR_HEIGHT_MARKS_ON_RIDE_TRACKS),
        ToggleOption(DDIDX_PATH_HEIGHTS, STR_HEIGHT_MARKS_ON_PATHS),
        Separator(),
        ToggleOption(DDIDX_VIEW_CLIPPING, STR_VIEW_CLIPPING_MENU),
        ToggleOption(DDIDX_HIGHLIGHT_PATH_ISSUES, STR_HIGHLIGHT_PATH_ISSUES_MENU),
    };

    static_assert(ItemIDsMatchIndices(items));

    SetItems(items);

    WindowDropdownShowText(
        { w->windowPos.x + widget->left, w->windowPos.y + widget->top }, widget->height() + 1, w->colours[1] | 0x80, 0,
        TOP_TOOLBAR_VIEW_MENU_COUNT);

    // Set checkmarks
    rct_viewport* mainViewport = window_get_main()->viewport;
    if (mainViewport->flags & VIEWPORT_FLAG_UNDERGROUND_INSIDE)
        Dropdown::SetChecked(DDIDX_UNDERGROUND_INSIDE, true);
    if (gConfigGeneral.transparent_water)
        Dropdown::SetChecked(DDIDX_TRANSPARENT_WATER, true);
    if (mainViewport->flags & VIEWPORT_FLAG_HIDE_BASE)
        Dropdown::SetChecked(DDIDX_HIDE_BASE, true);
    if (mainViewport->flags & VIEWPORT_FLAG_HIDE_VERTICAL)
        Dropdown::SetChecked(DDIDX_HIDE_VERTICAL, true);
    if (mainViewport->flags & VIEWPORT_FLAG_SEETHROUGH_RIDES)
        Dropdown::SetChecked(DDIDX_SEETHROUGH_RIDES, true);
    if (mainViewport->flags & VIEWPORT_FLAG_SEETHROUGH_SCENERY)
        Dropdown::SetChecked(DDIDX_SEETHROUGH_SCENERY, true);
    if (mainViewport->flags & VIEWPORT_FLAG_SEETHROUGH_PATHS)
        Dropdown::SetChecked(DDIDX_SEETHROUGH_PATHS, true);
    if (mainViewport->flags & VIEWPORT_FLAG_INVISIBLE_SUPPORTS)
        Dropdown::SetChecked(DDIDX_INVISIBLE_SUPPORTS, true);
    if (mainViewport->flags & VIEWPORT_FLAG_INVISIBLE_PEEPS)
        Dropdown::SetChecked(DDIDX_INVISIBLE_PEEPS, true);
    if (mainViewport->flags & VIEWPORT_FLAG_LAND_HEIGHTS)
        Dropdown::SetChecked(DDIDX_LAND_HEIGHTS, true);
    if (mainViewport->flags & VIEWPORT_FLAG_TRACK_HEIGHTS)
        Dropdown::SetChecked(DDIDX_TRACK_HEIGHTS, true);
    if (mainViewport->flags & VIEWPORT_FLAG_PATH_HEIGHTS)
        Dropdown::SetChecked(DDIDX_PATH_HEIGHTS, true);
    if (mainViewport->flags & VIEWPORT_FLAG_CLIP_VIEW)
        Dropdown::SetChecked(DDIDX_VIEW_CLIPPING, true);
    if (mainViewport->flags & VIEWPORT_FLAG_HIGHLIGHT_PATH_ISSUES)
        Dropdown::SetChecked(DDIDX_HIGHLIGHT_PATH_ISSUES, true);

    gDropdownDefaultIndex = DDIDX_UNDERGROUND_INSIDE;

    // Opaque water relies on RCT1 sprites.
    if (!is_csg_loaded())
    {
        Dropdown::SetDisabled(DDIDX_TRANSPARENT_WATER, true);
    }
}

/**
 *
 *  rct2: 0x0066CF8A
 */
static void top_toolbar_view_menu_dropdown(int16_t dropdownIndex)
{
    rct_window* w = window_get_main();
    if (w)
    {
        switch (dropdownIndex)
        {
            case DDIDX_UNDERGROUND_INSIDE:
                w->viewport->flags ^= VIEWPORT_FLAG_UNDERGROUND_INSIDE;
                break;
            case DDIDX_TRANSPARENT_WATER:
                gConfigGeneral.transparent_water ^= 1;
                config_save_default();
                break;
            case DDIDX_HIDE_BASE:
                w->viewport->flags ^= VIEWPORT_FLAG_HIDE_BASE;
                break;
            case DDIDX_HIDE_VERTICAL:
                w->viewport->flags ^= VIEWPORT_FLAG_HIDE_VERTICAL;
                break;
            case DDIDX_SEETHROUGH_RIDES:
                w->viewport->flags ^= VIEWPORT_FLAG_SEETHROUGH_RIDES;
                break;
            case DDIDX_SEETHROUGH_SCENERY:
                w->viewport->flags ^= VIEWPORT_FLAG_SEETHROUGH_SCENERY;
                break;
            case DDIDX_SEETHROUGH_PATHS:
                w->viewport->flags ^= VIEWPORT_FLAG_SEETHROUGH_PATHS;
                break;
            case DDIDX_INVISIBLE_SUPPORTS:
                w->viewport->flags ^= VIEWPORT_FLAG_INVISIBLE_SUPPORTS;
                break;
            case DDIDX_INVISIBLE_PEEPS:
                w->viewport->flags ^= VIEWPORT_FLAG_INVISIBLE_PEEPS;
                break;
            case DDIDX_LAND_HEIGHTS:
                w->viewport->flags ^= VIEWPORT_FLAG_LAND_HEIGHTS;
                break;
            case DDIDX_TRACK_HEIGHTS:
                w->viewport->flags ^= VIEWPORT_FLAG_TRACK_HEIGHTS;
                break;
            case DDIDX_PATH_HEIGHTS:
                w->viewport->flags ^= VIEWPORT_FLAG_PATH_HEIGHTS;
                break;
            case DDIDX_VIEW_CLIPPING:
                if (window_find_by_class(WC_VIEW_CLIPPING) == nullptr)
                {
                    context_open_window(WC_VIEW_CLIPPING);
                }
                else
                {
                    // If window is already open, toggle the view clipping on/off
                    w->viewport->flags ^= VIEWPORT_FLAG_CLIP_VIEW;
                }
                break;
            case DDIDX_HIGHLIGHT_PATH_ISSUES:
                w->viewport->flags ^= VIEWPORT_FLAG_HIGHLIGHT_PATH_ISSUES;
                break;
            default:
                return;
        }
        w->Invalidate();
    }
}

/**
 *
 *  rct2: 0x0066CCE7
 */
static void toggle_footpath_window()
{
    if (window_find_by_class(WC_FOOTPATH) == nullptr)
    {
        context_open_window(WC_FOOTPATH);
    }
    else
    {
        tool_cancel();
        window_close_by_class(WC_FOOTPATH);
    }
}

/**
 *
 *  rct2: 0x0066CD54
 */
static void toggle_land_window(rct_window* topToolbar, rct_widgetindex widgetIndex)
{
    if ((input_test_flag(INPUT_FLAG_TOOL_ACTIVE)) && gCurrentToolWidget.window_classification == WC_TOP_TOOLBAR
        && gCurrentToolWidget.widget_index == WIDX_LAND)
    {
        tool_cancel();
    }
    else
    {
        _landToolBlocked = false;
        show_gridlines();
        tool_set(topToolbar, widgetIndex, Tool::DigDown);
        input_set_flag(INPUT_FLAG_6, true);
        context_open_window(WC_LAND);
    }
}

/**
 *
 *  rct2: 0x0066CD0C
 */
static void toggle_clear_scenery_window(rct_window* topToolbar, rct_widgetindex widgetIndex)
{
    if ((input_test_flag(INPUT_FLAG_TOOL_ACTIVE) && gCurrentToolWidget.window_classification == WC_TOP_TOOLBAR
         && gCurrentToolWidget.widget_index == WIDX_CLEAR_SCENERY))
    {
        tool_cancel();
    }
    else
    {
        show_gridlines();
        tool_set(topToolbar, widgetIndex, Tool::Crosshair);
        input_set_flag(INPUT_FLAG_6, true);
        context_open_window(WC_CLEAR_SCENERY);
    }
}

/**
 *
 *  rct2: 0x0066CD9C
 */
static void toggle_water_window(rct_window* topToolbar, rct_widgetindex widgetIndex)
{
    if ((input_test_flag(INPUT_FLAG_TOOL_ACTIVE)) && gCurrentToolWidget.window_classification == WC_TOP_TOOLBAR
        && gCurrentToolWidget.widget_index == WIDX_WATER)
    {
        tool_cancel();
    }
    else
    {
        _landToolBlocked = false;
        show_gridlines();
        tool_set(topToolbar, widgetIndex, Tool::WaterDown);
        input_set_flag(INPUT_FLAG_6, true);
        context_open_window(WC_WATER);
    }
}

/**
 *
 *  rct2: 0x0066D104
 */
bool land_tool_is_active()
{
    if (!(input_test_flag(INPUT_FLAG_TOOL_ACTIVE)))
        return false;
    if (gCurrentToolWidget.window_classification != WC_TOP_TOOLBAR)
        return false;
    if (gCurrentToolWidget.widget_index != WIDX_LAND)
        return false;
    return true;
}

/**
 *
 *  rct2: 0x0066D125
 */
bool clear_scenery_tool_is_active()
{
    if (!(input_test_flag(INPUT_FLAG_TOOL_ACTIVE)))
        return false;
    if (gCurrentToolWidget.window_classification != WC_TOP_TOOLBAR)
        return false;
    if (gCurrentToolWidget.widget_index != WIDX_CLEAR_SCENERY)
        return false;
    return true;
}

/**
 *
 *  rct2: 0x0066D125
 */
bool water_tool_is_active()
{
    if (!(input_test_flag(INPUT_FLAG_TOOL_ACTIVE)))
        return false;
    if (gCurrentToolWidget.window_classification != WC_TOP_TOOLBAR)
        return false;
    if (gCurrentToolWidget.widget_index != WIDX_WATER)
        return false;
    return true;
}

static ClearAction GetClearAction()
{
    auto range = MapRange(gMapSelectPositionA.x, gMapSelectPositionA.y, gMapSelectPositionB.x, gMapSelectPositionB.y);

    ClearableItems itemsToClear = 0;

    if (gClearSmallScenery)
        itemsToClear |= CLEARABLE_ITEMS::SCENERY_SMALL;
    if (gClearLargeScenery)
        itemsToClear |= CLEARABLE_ITEMS::SCENERY_LARGE;
    if (gClearFootpath)
        itemsToClear |= CLEARABLE_ITEMS::SCENERY_FOOTPATH;

    return ClearAction(range, itemsToClear);
}
