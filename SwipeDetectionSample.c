#include <stdio.h>
#include <stdlib.h>
#include <time.h> // For time functions
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

#include "LeapC.h"
#include "ExampleConnection.h"

// Function prototypes
void detectSwipe(float initialPositionX, float finalPositionX);

// Swipe settings
const float swipeThreshold = 10.0; // Adjust threshold based on your needs (10 inches)

float initialHandPositionX = 0; // Initial X position of the hand
float finalHandPositionX = 0; // Final X position of the hand
int handDetected = 0; // Flag to indicate whether a hand is detected
int64_t lastFrameID = 0; // The last frame received

void detectSwipe(float initialPositionX, float finalPositionX) {
    // Determine swipe direction based on threshold
    if (finalPositionX - initialPositionX > swipeThreshold) {
        // Swipe RIGHT detected
        printf("Swipe RIGHT detected.\n");
        // system("ydotool key ctrl+alt+right"); // Simulate CTRL+ALT+RIGHT
    } else if (initialPositionX - finalPositionX > swipeThreshold) {
        // Swipe LEFT detected
        printf("Swipe LEFT detected.\n");
        // system("ydotool key ctrl+alt+left"); // Simulate CTRL+ALT+LEFT
    } else {
        // Swipe distance is below threshold
        printf("Swipe distance too short.\n");
    }
}

int main(int argc, char** argv) {
    OpenConnection();
    while (!IsConnected)
        millisleep(100); // wait a bit to let the connection complete

    LEAP_DEVICE_INFO* deviceProps = GetDeviceProperties();
    if (deviceProps) {
        printf("Using device %s.\n", deviceProps->serial);
    }

    printf("Listening is ready. Waiting for swipes...\n");

    for (;;) {
        LEAP_TRACKING_EVENT *frame = GetFrame();
        if (frame && (frame->tracking_frame_id > lastFrameID)) {
            lastFrameID = frame->tracking_frame_id;

            if (frame->nHands > 0) {
                handDetected = 1; // Hand detected
                for (uint32_t h = 0; h < frame->nHands; h++) {
                    LEAP_HAND* hand = &frame->pHands[h];

                    float currentHandPositionX = hand->palm.position.x;
                    // printf("Current Hand Position X: %f\n", currentHandPositionX);

                    // Initialize the hand positions when the hand is first detected
                    if (!handDetected) {
                        initialHandPositionX = currentHandPositionX;
                    }

                    // Update final position of the hand
                    finalHandPositionX = currentHandPositionX;
                }
            } else if (handDetected) {
                // No hands detected, detect swipe and reset detection
                printf("No hand detected, detecting swipe.\n");
                detectSwipe(initialHandPositionX, finalHandPositionX);
                handDetected = 0; // Reset hand detection flag
            }
        }
        // Sleep to avoid high CPU usage; adjust sleep time as needed
        #ifdef _WIN32
        Sleep(10); // Sleep for 10 milliseconds on Windows
        #else
        usleep(10000); // Sleep for 10 milliseconds on Linux/Unix
        #endif
    } // ctrl-c to exit

    return 0;
}
