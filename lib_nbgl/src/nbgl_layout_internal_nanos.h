/**
 * @file nbgl_layout_internal_nanos.h
 * @brief Internal functions/constants of NBGL layout layer
 *
 */

#ifndef NBGL_LAYOUT_INTERNAL_NANOS_H
#define NBGL_LAYOUT_INTERNAL_NANOS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nbgl_layout.h"

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/
/**
 * @brief Max number of complex objects with callback retrievable from pool
 *
 */
#define LAYOUT_OBJ_POOL_LEN 10

/**
 * @brief Horizontal margin (sum of left and right) between the text of a button and its border
 *
 */
#define BUTTON_MARGIN_Y 12

/**
 * @brief Maximum number of buttons used to display the sub text in @ref BUTTON_INFO style
 *
 */
#define NB_MAX_BUTTON_ROWS 3

/**
 * @brief Maximum number of buttons used to display the sub text in @ref BUTTON_INFO style with
 * @ref nbgl_layoutAddText()
 *
 */
#define NB_MAX_BUTTON_ROWS_PUBLIC 2

/**
 * @brief Maximum number of bytes (including '\0') of the text of a button in @ref BUTTON_INFO
 * style. A row of narrow chars is cut at this length, the rest goes in the next button
 *
 */
#define NB_MAX_CHAR_IN_BUTTON_ROW 20

/**********************
 *      TYPEDEFS
 **********************/

/**
 * @brief Structure containing all information about the current layout.
 * @note It shall not be used externally
 *
 */
typedef struct nbgl_layoutInternal_s {
    bool    modal;  ///< if true, means the screen is a modal
    uint8_t layer;  ///< if >0, puts the layout on top of screen stack (modal). Otherwise puts on
                    ///< background (for apps)
    uint8_t      nbChildren;  ///< number of children in above array
    nbgl_obj_t **children;    ///< children for main screen

    nbgl_layoutButtonCallback_t callback;  // user callback for all controls
    char
        buttonTexts[NB_MAX_BUTTON_ROWS]
                   [NB_MAX_CHAR_IN_BUTTON_ROW];  ///< texts of the buttons in @ref BUTTON_INFO
                                                 ///< style, kept as long as the layout is displayed
} nbgl_layoutInternal_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void     layoutAddObject(nbgl_layoutInternal_t *layout, nbgl_obj_t *obj);
uint16_t layoutGetButtonRowLen(const char *text, uint16_t *len, uint16_t *width);
int      layoutAddText(nbgl_layout_t                  *layout,
                       const char                     *text,
                       const char                     *subText,
                       nbgl_contentCenteredInfoStyle_t style,
                       uint8_t                         nbMaxLines,
                       bool                            ellipsis);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NBGL_LAYOUT_INTERNAL_NANOS_H */
