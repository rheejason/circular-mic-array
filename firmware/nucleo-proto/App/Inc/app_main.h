/*
 * app_main.h
 * Entry points called from main.c.
 */
#ifndef APP_MAIN_H
#define APP_MAIN_H

void app_init(void);    /* once, after the CubeMX init calls */
void app_loop(void);    /* every pass of the main while(1) loop */

#endif /* APP_MAIN_H */
