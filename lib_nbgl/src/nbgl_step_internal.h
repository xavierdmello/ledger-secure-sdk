/**
 * @file nbgl_step_internal.h
 * @brief Internal functions of NBGL step layer
 *
 */

#ifndef NBGL_STEP_INTERNAL_H
#define NBGL_STEP_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "nbgl_step.h"

/**********************
 * GLOBAL PROTOTYPES
 **********************/
nbgl_step_t stepDrawText(nbgl_stepPosition_t               pos,
                         nbgl_stepButtonCallback_t         onActionCallback,
                         nbgl_screenTickerConfiguration_t *ticker,
                         const char                       *text,
                         const char                       *subText,
                         nbgl_contentCenteredInfoStyle_t   style,
                         bool                              modal,
                         uint8_t                           nbMaxLinesForSubText);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NBGL_STEP_INTERNAL_H */
