/*
 * app_main.h
 *
 * Application entry points, called from the USER CODE blocks in main.c.
 */
#ifndef APP_MAIN_H
#define APP_MAIN_H

void app_init(void);    /* once, after the MX_*_Init() calls */
void app_loop(void);    /* every pass of the while(1) loop */

#endif /* APP_MAIN_H */
