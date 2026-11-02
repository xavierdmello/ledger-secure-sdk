/**
 * @file nbgl_layout_internal.h
 * @brief Internal functions/constants of NBGL layout layer
 *
 */

#ifndef NBGL_LAYOUT_INTERNAL_H
#define NBGL_LAYOUT_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nbgl_layout.h"
#include "glyphs.h"
#include "os_pic.h"

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/
// internal margin, between sub-items
#define INTERNAL_MARGIN 8

#ifdef SCREEN_SIZE_WALLET
#if SMALL_BUTTON_RADIUS == 32
#define SMALL_BUTTON_HEIGHT       64
#define SMALL_BUTTON_RADIUS_INDEX RADIUS_32_PIXELS
#elif SMALL_BUTTON_RADIUS == 20
#define SMALL_BUTTON_HEIGHT       40
#define SMALL_BUTTON_RADIUS_INDEX RADIUS_20_PIXELS
#endif  // SMALL_BUTTON_RADIUS
#endif  // SCREEN_SIZE_WALLET

/**
 * @brief Max number of complex objects with callback retrievable from pool
 *
 */
#define LAYOUT_OBJ_POOL_LEN 16

#define KEYBOARD_FOOTER_TYPE 99
#define KEYPAD_FOOTER_TYPE   98

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    nbgl_obj_t  *obj;
    uint8_t      token;   // user token, attached to callback
    uint8_t      index;   // index within the token
    tune_index_e tuneId;  // if not @ref NBGL_NO_TUNE, a tune will be played
} layoutObj_t;

typedef enum {
    SWIPE_USAGE_NAVIGATION,
    SWIPE_USAGE_CUSTOM,
    SWIPE_USAGE_SUGGESTIONS,  // for suggestion buttons
    NB_SWIPE_USAGE
} nbgl_swipe_usage_t;

// used by screen (top level)
enum {
    HEADER_INDEX = 0,  // For header container
    MAIN_CONTAINER_INDEX,
    TOP_RIGHT_BUTTON_INDEX,
    FOOTER_INDEX,
    UP_FOOTER_INDEX,
    LEFT_BORDER_INDEX,
    NB_MAX_SCREEN_CHILDREN
};

/**
 * @brief Structure containing all information about the current layout.
 * @note It shall not be used externally
 *
 */
typedef struct nbgl_layoutInternal_s {
    struct nbgl_layoutInternal_s *top;       ///< layout above this one, in stack
    struct nbgl_layoutInternal_s *bottom;    ///< layout under this one, in stack
    nbgl_obj_t                  **children;  ///< children for main screen
    nbgl_container_t
        *headerContainer;  ///< container used to store header (progress, back, empty space...)
    nbgl_container_t *footerContainer;    ///< container used to store footer (buttons, nav....)
    nbgl_container_t *upFooterContainer;  ///< container used on top on footer to store special
                                          ///< contents like tip-box or long-press button
    nbgl_text_area_t          *tapText;
    nbgl_layoutTouchCallback_t callback;  ///< user callback for all controls
    // This is the pool of callback objects, potentially used by this layout
    layoutObj_t callbackObjPool[LAYOUT_OBJ_POOL_LEN];

    nbgl_container_t       *container;
    const nbgl_animation_t *animation;  ///< current animation (if not NULL)

    uint8_t                   nbPages;       ///< number of pages for navigation bar
    uint8_t                   activePage;    ///< index of active page for navigation bar
    nbgl_layoutHeaderType_t   headerType;    ///< type of header
    nbgl_layoutFooterType_t   footerType;    ///< type of footer
    nbgl_layoutUpFooterType_t upFooterType;  ///< type of up-footer
    uint8_t                   modal : 1;     ///< if true, means the screen is a modal
    uint8_t withLeftBorder : 1;  ///< if true, draws a light gray left border on the whole height of
                                 ///< the screen
    uint8_t incrementAnim : 1;   ///< if true, means that animation index is currently incrementing
    uint8_t layer : 5;           ///< layer in screen stack
    uint8_t nbUsedCallbackObjs : 6;  ///< number of callback objects used by the whole layout in
                                     ///< callbackObjPool
    uint8_t            isUsed : 1;   ///< if true, means this layout is in use
    uint8_t            invertedColors : 1;  ///< if true, means that background is black
    uint8_t            iconIdxInAnim;       ///< current icon index in animation
    nbgl_swipe_usage_t swipeUsage;
} nbgl_layoutInternal_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
bool         keyboardSwipeCallback(nbgl_layoutInternal_t *layoutInt,
                                   nbgl_obj_t            *obj,
                                   nbgl_touchType_t       eventType);
void         layoutAddObject(nbgl_layoutInternal_t *layout, nbgl_obj_t *obj);
layoutObj_t *layoutAddCallbackObj(nbgl_layoutInternal_t *layout,
                                  nbgl_obj_t            *obj,
                                  uint8_t                token,
                                  tune_index_e           tuneId);
void layoutUpdateCallbackObjToken(nbgl_layoutInternal_t *layout, nbgl_obj_t *obj, uint8_t token);
void layoutNavigationPopulate(nbgl_container_t                 *navContainer,
                              const nbgl_layoutNavigationBar_t *navConfig,
                              uint8_t                           layer);
bool layoutNavigationCallback(nbgl_obj_t      *obj,
                              nbgl_touchType_t eventType,
                              uint8_t          nbPages,
                              uint8_t         *activePage);

/**********************
 *  INLINE FUNCTIONS
 **********************/

/**
 * @brief returns the font used to display the values of a list of tag/value pairs
 *
 * @param smallCaseForValue smallCaseForValue field of the list
 * @return the font of the values
 */
static inline nbgl_font_id_e layoutGetTagValueFont(bool smallCaseForValue)
{
    return smallCaseForValue ? SMALL_BOLD_FONT : LARGE_MEDIUM_FONT;
}

/**
 * @brief returns the icon displayed on the right of the value of the given tag/value pair
 *
 * @param pair tag/value pair
 * @return the (>) icon for an alias, otherwise the icon of the pair (NULL if none)
 */
static inline const nbgl_icon_details_t *layoutGetTagValueIcon(const nbgl_contentTagValue_t *pair)
{
    // valueIcon is in a union with extension, so aliasValue must be checked first
    if (pair->aliasValue) {
        // the (>) icon is automatically displayed for an alias
        return &MINI_PUSH_ICON;
    }
    if (pair->valueIcon != NULL) {
        return (const nbgl_icon_details_t *) PIC(pair->valueIcon);
    }
    return NULL;
}

/**
 * @brief returns the width of the value of a tag/value pair. Shared by the layout and the use
 * cases, so that the number of lines used to compute the number of pairs fitting in a page is
 * always the one really displayed
 *
 * @param icon icon on the right of the value, as returned by @ref layoutGetTagValueIcon()
 * @return the width in pixels of the value
 */
static inline uint16_t layoutGetTagValueWidth(const nbgl_icon_details_t *icon)
{
    // a value with an icon on its right has a reduced width
    if (icon != NULL) {
        return AVAILABLE_WIDTH - icon->width - VALUE_ICON_INTERVALE;
    }
    return AVAILABLE_WIDTH;
}

/**
 * @brief returns the max number of lines of the value of the given tag/value pair, which is the
 * smallest non-zero of the list one and the pair one
 *
 * @param listMaxLines max number of lines for all values of the list (0 if not limited)
 * @param pair tag/value pair
 * @param forceEllipsis [out] if not NULL, set to true if the pair has a max number of lines, in
 * which case the last line must end with "..." if the value doesn't fit, whichever limit cuts it
 * @return the max number of lines (0 if not limited)
 */
static inline uint8_t layoutGetTagValueMaxLines(uint8_t                       listMaxLines,
                                                const nbgl_contentTagValue_t *pair,
                                                bool                         *forceEllipsis)
{
    if (forceEllipsis != NULL) {
        *forceEllipsis = (pair->nbMaxLinesForValue > 0);
    }
    if ((pair->nbMaxLinesForValue > 0)
        && ((listMaxLines == 0) || (pair->nbMaxLinesForValue < listMaxLines))) {
        return pair->nbMaxLinesForValue;
    }
    return listMaxLines;
}

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NBGL_LAYOUT_INTERNAL_H */
