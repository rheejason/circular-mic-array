// Board bring-up test: LED ring chase once, then a live mic level meter on the ring.
#ifndef BOARD_TEST_H
#define BOARD_TEST_H

void board_test_init(void);   // call once after the MX_..._Init() calls
void board_test_loop(void);   // call forever from the main loop

#endif
