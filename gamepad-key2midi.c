#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <alsa/asoundlib.h>

/*

gcc gamepad-key2midi.c -o gamepad-key2midi -I/usr/include/libevdev-1.0 -levdev -lasound

*/
  
#define NUM_CONTROLS 19

// Structure to store the state of each button
struct ControlState {
    int evcode;  // PS3 button code
    int state;    // 0 for released, 1-255 for pressed
    int MIDIevcode;   // midi event associated with the control
};

/*
void MIDI_noteON(Display *display, KeySym key) {

}

void MIDI_noteOFF(Display *display, KeySym key) {

}
*/
int main(int argc, char **argv) {

    // Input device vendor product ID
    int vendor_id = 0x045e;
    int product_id = 0x028e;

    // Find the device with the matching vendor/product IDs
    struct libevdev* dev = NULL;
    int fd = -1;
    int rc = -1;

    for (int i = 0; i < 300; i++) {
        char path[128];
        snprintf(path, sizeof(path), "/dev/input/event%d", i);

        fd = open(path, O_RDONLY|O_NONBLOCK);
        if (fd < 0) {continue;}

        rc = libevdev_new_from_fd(fd, &dev);
        if (rc < 0) {close(fd); continue;}

        if (libevdev_get_id_vendor(dev) == vendor_id &&
            libevdev_get_id_product(dev) == product_id) 
			{break;}
    }

    if (fd < 0 || rc < 0) {
        fprintf(stderr, "Failed to find device with vendor/product ID %04x:%04x\n", vendor_id, product_id);
        return 1;
    }

    printf("Device found: %s\n", libevdev_get_name(dev));

	// const int NOTE_MAP[] = {60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76};

	// Array to store the state of each button
	// ev.code, ev.value, MIDI 
	struct ControlState controlStates[NUM_CONTROLS] = {
		{304, 0, 0},      	// A
		{305, 0, 4},      	// B
		{307, 0, 2},      	// X
		{308, 0, 5},      	// Y
		{310, 0, 9}, 		// L1
		{311, 0, 7}, 		// R1
		{314, 0, 1},    	// SELECT
		{315, 0, 3},       	// START
		{316, 0, 6},        // HOME		
		{317, 0, 8}, 		// L3
		{318, 0, 10},   	// R3
		{2, 0, 12},     	// L2
		{5, 0, 11},       	// R2
		{0, 0, 120},       	// Left X-axis
		{1, 0, 121},		// Left Y-axis
		{3, 0, 122},		// Right X-axis
		{4, 0, 123},		// Right Y-axis
		{16, 0, 124},		// Right Y-axis
		{17, 0, 125},		// Right Y-axis
		// Add more buttons as needed
	};

	// Process events
	while (1) {
		struct input_event ev;
		rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
		if (rc == 0) {
		    // Find the corresponding button state
		    int controlIndex = -1;

		    for (int i = 0; i < NUM_CONTROLS; i++) {
		        // Print the values for debugging
		        //printf("ev.code: %d, ev.value: %d, controlStates[%d].evcode: %d\n", ev.code, ev.value, i, controlStates[i].evcode);

		        if (ev.code == controlStates[i].evcode) {
		            controlIndex = i;
		            break;
		        }
		    }

		    if (controlIndex != -1 && (ev.type == EV_ABS || ev.type == EV_KEY)) {
				switch (ev.code) {
					// Buttons
					case 2:
					case 5:
					case 304 ... 318:  // Button pressed
			        	//printf("BTN Event: time %ld.%06ld, code %d, value %d\n", ev.time.tv_sec, ev.time.tv_usec, ev.code, ev.value);
						printf("BTN ev: %d, val: %d, MIDI: %d\n", ev.code, ev.value, controlStates[controlIndex].MIDIevcode);
						break;

					case 16:
					case 17:
						printf("DPAD ev: %d, val: %d, MIDI: %d\n", ev.code, ev.value, controlStates[controlIndex].MIDIevcode);
						break;

					// Joystick Axes
					case 0: 
					case 1:
					case 3:
					case 4:
						//printf("JOY Event: time %ld.%06ld, code %d, value %d\n", ev.time.tv_sec, ev.time.tv_usec, ev.code, ev.value);
						printf("JOY ev: %d, val: %d, MIDI: %d\n", ev.code, ev.value, controlStates[controlIndex].MIDIevcode);
						break;
				}
		    }

		} else if (rc == -EAGAIN) {
		    // No events available, try again
		    continue;
		} else {
		    // Error reading event, exit loop
		    break;
		}
	}

    // Clean up resources
	libevdev_free(dev);
	close(fd);
	return 0;
}


/*
void simulateKeyPress(Display *display, KeySym key) {
    KeyCode code = XKeysymToKeycode(display, key);
    XTestFakeKeyEvent(display, code, True, 0);
    XFlush(display);
}

void simulateKeyRelease(Display *display, KeySym key) {
    KeyCode code = XKeysymToKeycode(display, key);
    XTestFakeKeyEvent(display, code, False, 0);
    XFlush(display);
}

	struct ControlState controlStates[NUM_CONTROLS] = {
		{304, 0, XK_A},      	// A
		{305, 0, XK_B},      	// B
		{307, 0, XK_X},      	// X
		{308, 0, XK_Y},      	// Y
		{310, 0, XK_BackSpace}, // L1
		{311, 0, XK_Control_R}, // R1
		{314, 0, XK_Escape},    // SELECT
		{315, 0, XK_Tab},       // START
		{316, 0, XK_A},         // HOME		
		{317, 0, XK_BackSpace}, // L3
		{318, 0, XK_Delete},    // R3
		{2, 0, XK_Shift_R},     // L2
		{5, 0, XK_Alt_R},       // R2
		{0, 0, XK_Alt_L},       // Left X-axis
		{1, 0, XK_Alt_L},		// Left Y-axis
		{3, 0, XK_Alt_L},		// Right X-axis
		{4, 0, XK_Alt_L},		// Right Y-axis
		// Add more buttons as needed
	};


		    if (controlIndex != -1) {
				switch (ev.value) {
					case 1 ... 255:  // Button pressed
						if (controlStates[controlIndex].state == 0) {
							simulateKeyPress(display, controlStates[controlIndex].key);
							controlStates[controlIndex].state = 1;
						}
						break;

					case 0:  // Button released
						if (controlStates[controlIndex].state == 1) {
							simulateKeyRelease(display, controlStates[controlIndex].key);
							controlStates[controlIndex].state = 0;
						}
						break;
				}
		    }

*/