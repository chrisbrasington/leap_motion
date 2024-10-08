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
const float swipeThreshold = 100.0; // Threshold to detect significant swipes

float initialHandPositionX = 0; // Initial X position of the hand
float finalHandPositionX = 0; // Final X position of the hand
int handDetected = 0; // Flag to indicate whether a hand is detected
int64_t lastFrameID = 0; // The last frame received

void detectSwipe(float initialPositionX, float finalPositionX) {
    float distance = finalPositionX - initialPositionX; // Calculate the distance of the swipe

    if (distance > swipeThreshold) {
        // Swipe LEFT detected
        printf("Swipe Swipe detected. Distance: %.2f\n", distance);
        system("ydotool key ctrl+alt+left 2>/dev/null"); // Simulate CTRL+ALT+Swipe and suppress errors
    } else if (-distance > swipeThreshold) {
        // Swipe RIGHT detected
        printf("Swipe RIGHT detected. Distance: %.2f\n", -distance);
        system("ydotool key ctrl+alt+right 2>/dev/null"); // Simulate CTRL+ALT+RIGHT and suppress errors
    } else {
        // Swipe distance is below threshold
        printf("Swipe distance too short. Distance: %.2f\n", distance);
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
                    if (!handDetected) {
                        initialHandPositionX = currentHandPositionX;
                    }

                    // Update final position of the hand
                    finalHandPositionX = currentHandPositionX;
                }
            } else if (handDetected) {
                // No hands detected, process the swipe
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
