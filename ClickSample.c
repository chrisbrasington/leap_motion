#include <stdio.h>
#include <stdlib.h>
#include <time.h> // For time functions
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "LeapC.h"
#include "ExampleConnection.h"

// Constants
const int handHoldTime = 500; // Time in milliseconds to detect hand hold

// Global variables
int handDetected = 0; // Flag to indicate whether a hand is detected
clock_t handDetectionStartTime; // Start time for hand detection

void holdMouseButton() {
    printf("Hand detected for at least %d milliseconds. Holding down the left mouse button.\n", handHoldTime);
    // system("ydotool click 1"); 
}

void releaseMouseButton() {
    printf("No hand detected. Releasing the left mouse button.\n");
    // system("ydotool click 1"); 
}

int main(int argc, char** argv) {
    OpenConnection();
    while (!IsConnected)
        millisleep(100); // Wait a bit to let the connection complete

    LEAP_DEVICE_INFO* deviceProps = GetDeviceProperties();
    if (deviceProps) {
        printf("Using device %s.\n", deviceProps->serial);
    }

    printf("Listening is ready. Waiting for hand...\n");

    for (;;) {
        LEAP_TRACKING_EVENT *frame = GetFrame();
        if (frame) {
            if (frame->nHands > 0) {
                if (!handDetected) {
                    // Hand detected for the first time
                    handDetected = 1;
                    handDetectionStartTime = clock(); // Record the start time
                    printf("Hand detected. Tracking started.\n");
                    system("ydotool click 1"); // Simulate mouse click
                }

                // Check if the hand has been detected for at least half a second
                clock_t currentTime = clock();
                double elapsedTime = (double)(currentTime - handDetectionStartTime) / CLOCKS_PER_SEC * 1000;
                if (elapsedTime >= handHoldTime) {
                    holdMouseButton();
                }
            } else if (handDetected) {
                // No hand detected, release the mouse button
                releaseMouseButton();
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
