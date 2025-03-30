/*
 *	wiimotebridged
 *
 *	Author:
 *		Alberto Di Cagno	< alberto.dicagno@melodea.io >

 *      From the boilerplate by Michael Laforest < thepara@gmail.com >
 *
 *	Copyright 2006-2025
 *
 *	This file is part of Melodea 3D Interactive Visuals stack.
 *
 *	$Header$
 *
 */

/**
 *	@file
 *
 *	@brief Bridge between WiiMote and OSC - works on Raspberry Pi chipset with BlueZ.
 */

#include <stdio.h>      /* for printf */
#include <stdlib.h>     /* for atoi */
#include <string.h>     /* for memset */
#include <time.h>       /* for time */
#include <stdbool.h>    /* for bool type */

#include "wiiuse.h"                     /* for wiimote_t, classic_ctrl_t, etc */

#ifndef WIIUSE_WIN32
#include <unistd.h>                     /* for usleep */
#endif

#include "osc.h"

#define CONNECTION_TIMEOUT			30

// Global variables
osc_client_t osc_client;
int wiimote_id = 1;  // Default ID if not specified
bool b_button_pressed = false; // Track B button state

// Add moving average filters for each axis
struct {
	moving_average_t roll;
	moving_average_t pitch;
	moving_average_t yaw;
	moving_average_t accel_x;
	moving_average_t accel_y;
	moving_average_t accel_z;
} filters;

/**
 *	@brief Callback that handles an event.
 *
 *	@param wm		Pointer to a wiimote_t structure.
 *
 *	This function is called automatically by the wiiuse library when an
 *	event occurs on the specified wiimote.
 */
void handle_event(struct wiimote_t* wm) {
	printf("\n\n--- EVENT [id %i] ---\n", wiimote_id);

	/* Track B button state for acceleration data */
	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_B)) {
		b_button_pressed = true;
		printf("B pressed - acceleration data enabled\n");
	} else if ((wm->btns_released & WIIMOTE_BUTTON_B) && b_button_pressed) {
		b_button_pressed = false;
		printf("B released - acceleration data disabled\n");
	}

	/* Handle all button events */
	if (IS_PRESSED(wm, WIIMOTE_BUTTON_A)) {
		printf("A pressed\n");
		char addr[64];
		snprintf(addr, sizeof(addr), "/wii/%d/buttons/a", wiimote_id);
		osc_send_message(&osc_client, addr, ",i", 1);
	} else if (wm->btns_released & WIIMOTE_BUTTON_A) {
		char addr[64];
		snprintf(addr, sizeof(addr), "/wii/%d/buttons/a", wiimote_id);
		osc_send_message(&osc_client, addr, ",i", 0);
	}
	
	if (IS_PRESSED(wm, WIIMOTE_BUTTON_B)) {
		printf("B pressed\n");
		char addr[64];
		snprintf(addr, sizeof(addr), "/wii/%d/buttons/b", wiimote_id);
		osc_send_message(&osc_client, addr, ",i", 1);
	} else if (wm->btns_released & WIIMOTE_BUTTON_B) {
		char addr[64];
		snprintf(addr, sizeof(addr), "/wii/%d/buttons/b", wiimote_id);
		osc_send_message(&osc_client, addr, ",i", 0);
	}

	if (IS_PRESSED(wm, WIIMOTE_BUTTON_UP)) {
		printf("UP pressed\n");
		char addr[64];
		snprintf(addr, sizeof(addr), "/wii/%d/buttons/up", wiimote_id);
		osc_send_message(&osc_client, addr, ",i", 1);
	} else if (wm->btns_released & WIIMOTE_BUTTON_UP) {
		char addr[64];
		snprintf(addr, sizeof(addr), "/wii/%d/buttons/up", wiimote_id);
		osc_send_message(&osc_client, addr, ",i", 0);
	}

	if (IS_PRESSED(wm, WIIMOTE_BUTTON_DOWN)) {
		printf("DOWN pressed\n");
		char addr[64];
		snprintf(addr, sizeof(addr), "/wii/%d/buttons/down", wiimote_id);
		osc_send_message(&osc_client, addr, ",i", 1);
	} else if (wm->btns_released & WIIMOTE_BUTTON_DOWN) {
		char addr[64];
		snprintf(addr, sizeof(addr), "/wii/%d/buttons/down", wiimote_id);
		osc_send_message(&osc_client, addr, ",i", 0);
	}

	if (IS_PRESSED(wm, WIIMOTE_BUTTON_LEFT)) {
		printf("LEFT pressed\n");
		char addr[64];
		snprintf(addr, sizeof(addr), "/wii/%d/buttons/left", wiimote_id);
		osc_send_message(&osc_client, addr, ",i", 1);
	} else if (wm->btns_released & WIIMOTE_BUTTON_LEFT) {
		char addr[64];
		snprintf(addr, sizeof(addr), "/wii/%d/buttons/left", wiimote_id);
		osc_send_message(&osc_client, addr, ",i", 0);
	}

	if (IS_PRESSED(wm, WIIMOTE_BUTTON_RIGHT)) {
		printf("RIGHT pressed\n");
		char addr[64];
		snprintf(addr, sizeof(addr), "/wii/%d/buttons/right", wiimote_id);
		osc_send_message(&osc_client, addr, ",i", 1);
	} else if (wm->btns_released & WIIMOTE_BUTTON_RIGHT) {
		char addr[64];
		snprintf(addr, sizeof(addr), "/wii/%d/buttons/right", wiimote_id);
		osc_send_message(&osc_client, addr, ",i", 0);
	}

	if (IS_PRESSED(wm, WIIMOTE_BUTTON_MINUS)) {
		printf("MINUS pressed\n");
		char addr[64];
		snprintf(addr, sizeof(addr), "/wii/%d/buttons/minus", wiimote_id);
		osc_send_message(&osc_client, addr, ",i", 1);
	} else if (wm->btns_released & WIIMOTE_BUTTON_MINUS) {
		char addr[64];
		snprintf(addr, sizeof(addr), "/wii/%d/buttons/minus", wiimote_id);
		osc_send_message(&osc_client, addr, ",i", 0);
	}

	if (IS_PRESSED(wm, WIIMOTE_BUTTON_PLUS)) {
		printf("PLUS pressed\n");
		char addr[64];
		snprintf(addr, sizeof(addr), "/wii/%d/buttons/plus", wiimote_id);
		osc_send_message(&osc_client, addr, ",i", 1);
	} else if (wm->btns_released & WIIMOTE_BUTTON_PLUS) {
		char addr[64];
		snprintf(addr, sizeof(addr), "/wii/%d/buttons/plus", wiimote_id);
		osc_send_message(&osc_client, addr, ",i", 0);
	}

	if (IS_PRESSED(wm, WIIMOTE_BUTTON_ONE)) {
		printf("ONE pressed\n");
		char addr[64];
		snprintf(addr, sizeof(addr), "/wii/%d/buttons/one", wiimote_id);
		osc_send_message(&osc_client, addr, ",i", 1);
	} else if (wm->btns_released & WIIMOTE_BUTTON_ONE) {
		char addr[64];
		snprintf(addr, sizeof(addr), "/wii/%d/buttons/one", wiimote_id);
		osc_send_message(&osc_client, addr, ",i", 0);
	}

	if (IS_PRESSED(wm, WIIMOTE_BUTTON_TWO)) {
		printf("TWO pressed\n");
		char addr[64];
		snprintf(addr, sizeof(addr), "/wii/%d/buttons/two", wiimote_id);
		osc_send_message(&osc_client, addr, ",i", 1);
	} else if (wm->btns_released & WIIMOTE_BUTTON_TWO) {
		char addr[64];
		snprintf(addr, sizeof(addr), "/wii/%d/buttons/two", wiimote_id);
		osc_send_message(&osc_client, addr, ",i", 0);
	}

	if (IS_PRESSED(wm, WIIMOTE_BUTTON_HOME)) {
		printf("HOME pressed\n");
		char addr[64];
		snprintf(addr, sizeof(addr), "/wii/%d/buttons/home", wiimote_id);
		osc_send_message(&osc_client, addr, ",i", 1);
	} else if (wm->btns_released & WIIMOTE_BUTTON_HOME) {
		char addr[64];
		snprintf(addr, sizeof(addr), "/wii/%d/buttons/home", wiimote_id);
		osc_send_message(&osc_client, addr, ",i", 0);
	}

	/* Enable/disable motion sensing based on plus/minus */
	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_MINUS)) {
		wiiuse_motion_sensing(wm, 0);
	}
	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_PLUS)) {
		wiiuse_motion_sensing(wm, 1);
	}

	/* Toggle rumble on B just press */
	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_B)) {
		wiiuse_toggle_rumble(wm);
	}

	/* IR camera control with up/down */
	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_UP)) {
		wiiuse_set_ir(wm, 1);
	}
	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_DOWN)) {
		wiiuse_set_ir(wm, 0);
	}

	/* Motion+ control with 1/2 buttons */
	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_ONE)) {
		if (WIIUSE_USING_EXP(wm)) {
			wiiuse_set_motion_plus(wm, 2);    // nunchuck pass-through
		} else {
			wiiuse_set_motion_plus(wm, 1);    // standalone
		}
	}
	if (IS_JUST_PRESSED(wm, WIIMOTE_BUTTON_TWO)) {
		wiiuse_set_motion_plus(wm, 0); // off
	}

	/* Handle acceleration data only when B button is pressed */
	if (WIIUSE_USING_ACC(wm) && b_button_pressed) {
		// Get filtered values
		float roll = moving_average_update(&filters.roll, wm->orient.roll);
		float pitch = moving_average_update(&filters.pitch, wm->orient.pitch);
		float yaw = moving_average_update(&filters.yaw, wm->orient.yaw);
		
		// Send orientation data
		char addr[64];
		snprintf(addr, sizeof(addr), "/wii/%d/orientation", wiimote_id);
		osc_send_message(&osc_client, addr, ",fff", roll, pitch, yaw);
		
		printf("wiimote roll  = %f [%f]\n", roll, wm->orient.a_roll);
		printf("wiimote pitch = %f [%f]\n", pitch, wm->orient.a_pitch);
		printf("wiimote yaw   = %f\n", yaw);

		// Handle raw acceleration data
		float accel_x = moving_average_update(&filters.accel_x, wm->accel.x);
		float accel_y = moving_average_update(&filters.accel_y, wm->accel.y);
		float accel_z = moving_average_update(&filters.accel_z, wm->accel.z);

		if (accel_x != 0 || accel_y != 0 || accel_z != 0) {
			snprintf(addr, sizeof(addr), "/wii/%d/accel", wiimote_id);
			osc_send_message(&osc_client, addr, ",fff", accel_x, accel_y, accel_z);
		}
	}

	/* Handle IR data if enabled */
	if (WIIUSE_USING_IR(wm)) {
		int i = 0;
		for (; i < 4; ++i) {
			if (wm->ir.dot[i].visible) {
				printf("IR source %i: (%u, %u)\n", i, wm->ir.dot[i].x, wm->ir.dot[i].y);
			}
		}
		printf("IR cursor: (%u, %u)\n", wm->ir.x, wm->ir.y);
		printf("IR z distance: %f\n", wm->ir.z);
	}

	/* Handle nunchuk if connected */
	if (wm->exp.type == EXP_NUNCHUK || wm->exp.type == EXP_MOTION_PLUS_NUNCHUK) {
		struct nunchuk_t* nc = (nunchuk_t*)&wm->exp.nunchuk;

		if (IS_PRESSED(nc, NUNCHUK_BUTTON_C)) {
			printf("Nunchuk: C pressed\n");
			char addr[64];
			snprintf(addr, sizeof(addr), "/wii/%d/nunchuk/buttons/c", wiimote_id);
			osc_send_message(&osc_client, addr, ",i", 1);
		} else if (nc->btns_released & NUNCHUK_BUTTON_C) {
			char addr[64];
			snprintf(addr, sizeof(addr), "/wii/%d/nunchuk/buttons/c", wiimote_id);
			osc_send_message(&osc_client, addr, ",i", 0);
		}

		if (IS_PRESSED(nc, NUNCHUK_BUTTON_Z)) {
			printf("Nunchuk: Z pressed\n");
			char addr[64];
			snprintf(addr, sizeof(addr), "/wii/%d/nunchuk/buttons/z", wiimote_id);
			osc_send_message(&osc_client, addr, ",i", 1);
		} else if (nc->btns_released & NUNCHUK_BUTTON_Z) {
			char addr[64];
			snprintf(addr, sizeof(addr), "/wii/%d/nunchuk/buttons/z", wiimote_id);
			osc_send_message(&osc_client, addr, ",i", 0);
		}

		// Send joystick data
		char addr[64];
		snprintf(addr, sizeof(addr), "/wii/%d/nunchuk/joystick", wiimote_id);
		osc_send_message(&osc_client, addr, ",ff", nc->js.x, nc->js.y);
	}
}

/**
 *	@brief Callback that handles a read event.
 *
 *	@param wm		Pointer to a wiimote_t structure.
 *	@param data		Pointer to the filled data block.
 *	@param len		Length in bytes of the data block.
 *
 *	This function is called automatically by the wiiuse library when
 *	the wiimote has returned the full data requested by a previous
 *	call to wiiuse_read_data().
 *
 *	You can read data on the wiimote, such as Mii data, if
 *	you know the offset address and the length.
 *
 *	The \a data pointer was specified on the call to wiiuse_read_data().
 *	At the time of this function being called, it is not safe to deallocate
 *	this buffer.
 */
void handle_read(struct wiimote_t* wm, byte* data, unsigned short len) {
	int i = 0;

	printf("\n\n--- DATA READ [wiimote id %i] ---\n", wm->unid);
	printf("finished read of size %i\n", len);
	for (; i < len; ++i) {
		if (!(i % 16)) {
			printf("\n");
		}
		printf("%x ", data[i]);
	}
	printf("\n\n");
}


/**
 *	@brief Callback that handles a controller status event.
 *
 *	@param wm				Pointer to a wiimote_t structure.
 *	@param attachment		Is there an attachment? (1 for yes, 0 for no)
 *	@param speaker			Is the speaker enabled? (1 for yes, 0 for no)
 *	@param ir				Is the IR support enabled? (1 for yes, 0 for no)
 *	@param led				What LEDs are lit.
 *	@param battery_level	Battery level, between 0.0 (0%) and 1.0 (100%).
 *
 *	This occurs when either the controller status changed
 *	or the controller status was requested explicitly by
 *	wiiuse_status().
 *
 *	One reason the status can change is if the nunchuk was
 *	inserted or removed from the expansion port.
 */
void handle_ctrl_status(struct wiimote_t* wm) {
	printf("\n\n--- CONTROLLER STATUS [wiimote id %i] ---\n", wiimote_id);

	printf("attachment:      %i\n", wm->exp.type);
	printf("speaker:         %i\n", WIIUSE_USING_SPEAKER(wm));
	printf("ir:              %i\n", WIIUSE_USING_IR(wm));
	printf("leds:            %i %i %i %i\n", WIIUSE_IS_LED_SET(wm, 1), WIIUSE_IS_LED_SET(wm, 2), WIIUSE_IS_LED_SET(wm, 3), WIIUSE_IS_LED_SET(wm, 4));
	printf("battery:         %f %%\n", wm->battery_level);
}


/**
 *	@brief Callback that handles a disconnection event.
 *
 *	@param wm				Pointer to a wiimote_t structure.
 *
 *	This can happen if the POWER button is pressed, or
 *	if the connection is interrupted.
 */
void handle_disconnect(wiimote* wm) {
	printf("\n\n--- DISCONNECTED [wiimote id %i] ---\n", wiimote_id);
}


void test(struct wiimote_t* wm, byte* data, unsigned short len) {
	printf("test: %i [%x %x %x %x]\n", len, data[0], data[1], data[2], data[3]);
}

/**
 * @brief Get our assigned ID for a wiimote based on its Bluetooth address
 * @param wm Pointer to wiimote structure
 * @return The assigned ID (1-based) or -1 if not found
 */
int get_assigned_wiimote_id(struct wiimote_t* wm) {
	if (!wm || !wm->bdaddr_str) return -1;
	
	return wiimote_id;
}

/**
 * @brief Checks if any wiimote in the array is currently connected.
 * @param wm Array of wiimote pointers.
 * @param wiimotes The number of slots in the array to check.
 * @return 1 if at least one wiimote is connected, 0 otherwise.
 */
short any_wiimote_connected(wiimote** wm, int wiimotes) {
	int i;
	if (!wm) {
		return 0;
	}
	for (i = 0; i < wiimotes; i++) {
		if (wm[i] && WIIMOTE_IS_CONNECTED(wm[i])) {
			return 1;
		}
	}
	return 0;
}

/**
 *	@brief main()
 *
 *	Connect to up to two wiimotes and print any events
 *	that occur on either device.
 */
int main(int argc, char** argv) {
	// MUST be the very first thing we do
	if (argc != 2) {
		fprintf(stderr, "Error: Wiimote ID argument is required\n\n");
		fprintf(stderr, "Usage: %s <wiimote_id>\n", argv[0]);
		fprintf(stderr, "  wiimote_id must be between 1 and 4\n");
		return 1;  // Exit immediately
	}

	// Convert and validate the Wiimote ID
	char* endptr;
	int wiimote_id = strtol(argv[1], &endptr, 10);
	if (*endptr != '\0' || wiimote_id < 1 || wiimote_id > 4) {
		fprintf(stderr, "Error: Invalid Wiimote ID '%s'. Must be a number between 1 and 4.\n", argv[1]);
		return 1;  // Exit immediately
	}

	// Only continue if we have a valid ID
	printf("Starting wiimotebridged with Wiimote ID: %d\n", wiimote_id);

	wiimote** wiimotes;
	int found, connected;
	time_t start_time;
	int seconds_remaining;
	
	// Initialize OSC client structure
	memset(&osc_client, 0, sizeof(osc_client));

	// Try to discover OSC server
	printf("Discovering OSC server...\n");
	if (osc_discover_server(&osc_client) < 0) {
		printf("Failed to discover OSC server. Please check if AgapeKidAvatarBridge is running.\n");
		return 1;
	}
	printf("Successfully connected to OSC server at %s:%d\n", osc_client.host, osc_client.port);

	// Initialize moving average filters
	moving_average_init(&filters.roll, 0.5f);
	moving_average_init(&filters.pitch, 0.5f);
	moving_average_init(&filters.yaw, 0.5f);
	moving_average_init(&filters.accel_x, 0.1f);
	moving_average_init(&filters.accel_y, 0.1f);
	moving_average_init(&filters.accel_z, 0.1f);

	// Initialize wiimote
	wiimotes = wiiuse_init(1);  // Just one wiimote
	if (!wiimotes) {
		printf("Failed to initialize wiimote.\n");
		return 1;
	}

	printf("Please press 1+2 on your Wiimote now...\n");
	printf("You have %d seconds to connect.\n", CONNECTION_TIMEOUT);
	
	// Record the start time
	start_time = time(NULL);
	bool is_connected = false;
	
	// Connection loop - try to find and connect wiimote until timeout
	while (!is_connected && (time(NULL) - start_time < CONNECTION_TIMEOUT)) {
		seconds_remaining = CONNECTION_TIMEOUT - (time(NULL) - start_time);
		
		// Print remaining time
		printf("\rWaiting for Wiimote... (%d seconds remaining)   ", seconds_remaining);
		fflush(stdout);
		
		// Search for wiimote (with a short timeout)
		found = wiiuse_find(wiimotes, 1, 1); // 1 second timeout
		
		if (found > 0) {
			// Try to connect to found wiimote
			connected = wiiuse_connect(wiimotes, 1);
			
			if (connected > 0 && wiimotes[0] && WIIMOTE_IS_CONNECTED(wiimotes[0])) {
				printf("\nConnected to Wiimote (address: %s)\n", wiimotes[0]->bdaddr_str);
				
				// Set LED based on ID (1-4)
				if (wiimote_id >= 1 && wiimote_id <= 4) {
					wiiuse_set_leds(wiimotes[0], WIIMOTE_LED_1 << (wiimote_id - 1));
				}
				
				// Brief rumble for feedback
				wiiuse_rumble(wiimotes[0], 1);
#ifndef WIIUSE_WIN32
				usleep(200000);
#else
				Sleep(200);
#endif
				wiiuse_rumble(wiimotes[0], 0);
				
				// Enable motion sensing
				wiiuse_motion_sensing(wiimotes[0], 1);
				
				is_connected = true;
				break;
			}
		}
		
		// Small delay to prevent CPU hogging
#ifndef WIIUSE_WIN32
		usleep(100000); // 100ms
#else
		Sleep(100);
#endif
	}
	
	printf("\n");
	
	// Check if wiimote was connected
	if (!is_connected) {
		printf("No Wiimote connected within the %d second timeout. Exiting.\n", CONNECTION_TIMEOUT);
		wiiuse_cleanup(wiimotes, 1);
		return 0;
	}
	
	printf("Connection complete. Wiimote ID %d ready.\n", wiimote_id);

	printf("\nControls:\n");
	printf("\tB toggles rumble.\n");
	printf("\tB (hold) enables acceleration data sending.\n");
	printf("\t+ to start Wiimote accelerometer reporting, - to stop\n");
	printf("\tUP to start IR camera (sensor bar mode), DOWN to stop.\n");
	printf("\t1 to start Motion+ reporting, 2 to stop.\n");
	printf("\n\n");

	// Main loop
	while (wiimotes[0] && WIIMOTE_IS_CONNECTED(wiimotes[0])) {
		if (wiiuse_poll(wiimotes, 1)) {
			switch (wiimotes[0]->event) {
				case WIIUSE_EVENT:
					handle_event(wiimotes[0]);
					break;
				case WIIUSE_STATUS:
					handle_ctrl_status(wiimotes[0]);
					break;
				case WIIUSE_DISCONNECT:
				case WIIUSE_UNEXPECTED_DISCONNECT:
					handle_disconnect(wiimotes[0]);
					break;
				default:
					break;
			}
		}
	}

	// Cleanup
	wiiuse_cleanup(wiimotes, 1);
	printf("Wiimote disconnected. Exiting.\n");
	return 0;
}
