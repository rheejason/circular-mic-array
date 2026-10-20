# 8-mic board: layout checklist

From the 2026-10-20 schematic review. Tick items off as they're done. The schematic itself was checked against
every datasheet and matches the PCB netlist; the items below are what's left. No LCD on this revision.

## 1. Schematic changes before layout

| Done | Item | Why |
|---|---|---|
| [ ] | **U7 pin 11 (B4) → GND** (remove the no-connect flag) | DIR4 = GND makes B4 an input; TI: inputs must not float (extra current, can oscillate next to the audio lines) |
| [ ] | **USBLC6 (U2): put pins 1 + 6 on one net and pins 3 + 4 on one net** | Removes any dependence on the chip's internal connection; route D+ / D- straight through both pads |
| [ ] | **Green LED + 2.2 k on PB0, red LED + 2.2 k on PB14** (to GND), placed away from the LED ring and the translucent window | Same pins as the Nucleo's LD1 / LD3, so `app_main` blink and `Error_Handler` work unchanged. Dim, and firmware turns them off in normal use, so they don't light the enclosure |
| [ ] | **1×8 2.54 mm debug header** (can be left unpopulated) near U7: GND, PD8 (USART3 TX), PD9 (USART3 RX), PE5 (BCLK), PE4 (FSYNC), PE6 (SDATA), PDM_CLK0, PDM_DAT0 | Logic analyzer leads / Dupont jumpers plug straight on; doubles as the UART debug port (same USART3 as the Nucleo VCP). No power pins, so a misplaced jumper can't short a rail |
| [ ] | **One GND test loop** (e.g. Keystone 5001) | Scope ground clip. Measure rails on the regulator caps |
| [ ] | Optional: 2-pin BOOT0 jumper (BOOT0 to 3V3) | USB DFU fallback if SWD ever can't connect |
| [ ] | Optional: reset button NRST to GND | Convenience; SWD and replugging USB also reset |
| [ ] | **Mounting holes**: 3 or 4 × M3 (MountingHole_3.2mm_M3), on GND or isolated | Enclosure / standoffs |
| [ ] | Copy the T5838, ADAU7118, TLC59116F, SN74AXC4T774 symbols and `T5838.pretty` into the project; point `sym-lib-table` / `fp-lib-table` at `${KIPRJMOD}` | Tables currently point at `C:/Users/sstim/Downloads/...`, so Update PCB fails on any other computer |

Not needed: SWO (Live Expressions work over plain SWD), LCD / expansion header.

## 2. Board setup

| Done | Item |
|---|---|
| [ ] | **4 layers**: signal / GND / power / signal. Solid GND plane under the MCU, ADAU7118 and mics |
| [ ] | Board outline: circle (or rounded square) around the array; holes are at 55.0 mm radius, mic bodies need ~3 mm more, so ≥ 125 mm diameter |
| [ ] | **Decide the acoustic face.** Mics are bottom-port parts on the top layer, so sound enters through holes in the *bottom* face. Either mount the board components-down (holes face up; the right-angle LEDs still shine outward), or plan standoffs and an open path under the board |
| [ ] | Leave the 8 mic footprints exactly where they are (already at 55.0 mm, 45° apart) |

## 3. Placement and routing

| Done | Item |
|---|---|
| [ ] | ADAU7118 (U4) near the centre of the circle so the 8 PDM traces are short and similar. Exposed pad soldered to GND |
| [ ] | PDM_CLK0 → MK1–4 and PDM_CLK1 → MK5–8 as daisy chains or short stubs; keep PDM lines away from USB D+/D- and the crystal |
| [ ] | 0.1 µF next to every mic VDD pin (C8–C17), on the same side, short return to GND |
| [ ] | Keep copper, vias and silkscreen out of each mic's sound hole and sealing ring; no parts over the holes on the acoustic side |
| [ ] | U7 (level shifter) between U4 and the MCU's PE4–PE6; keep BCLK / FSYNC / SDATA short (timing margin is about 8 ns at 12.288 MHz) |
| [ ] | Crystal Y1 + C20/C21 right next to PH0/PH1, GND guard, no traces under it |
| [ ] | VCAP caps C22/C23 at pins 71 and 106; one 0.1 µF per VDD pin, as close as possible |
| [ ] | USB-C (J2) on the board edge; D+/D- as a short pair through U2 to PA11/PA12; U2 close to the connector |
| [ ] | MCP1826S (U5) tab on a copper pour (dissipates ~0.5 W) |
| [ ] | SWD header J1 and the new UART header at the edge, reachable with the board in the enclosure |
| [ ] | 16 right-angle LEDs in a ring, each one facing outward; optionally assign OUT0–OUT15 in ring order with pin swaps (the order is scrambled now) |
| [ ] | Label test pads, J1 pinout, LED colours and MK1 on silkscreen |

## 4. Before ordering

| Done | Item |
|---|---|
| [ ] | ERC and DRC clean (in KiCad 10) |
| [ ] | Print the board 1:1 and check the T5838, ADAU7118, USB-C and crystal footprints against real parts |
| [ ] | All parts in stock (T5838, ADAU7118ACPZRL, SN74AXC4T774PWR, TLC59116FIPWR, ECS-160-8-30Q-VY-TR); order spare mics |
| [ ] | Stencil or PCBA: the T5838 (LGA) and ADAU7118 (0.5 mm LFCSP) can't be soldered with an iron. Don't wash the mics |
| [ ] | BOM says STM32H753ZIT6 (same as the Nucleo) or H743ZIT6, chosen on purpose |
