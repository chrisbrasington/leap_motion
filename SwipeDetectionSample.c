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

// Swipe settings
const float swipeThreshold = 0.05; // Adjust threshold based on your needs
const int swipeWindowSize = 20; // Number of data points to average swipe distance

time_t lastSwipeTime = 0; // Last time a swipe was detected
float* handPositionHistory; // Swipe history buffer
int historyIndex = 0; // Current index for history buffer
int handDetected = 0; // Flag to indicate whether a hand is detected
float lastHandPositionX = 0; // Last X position of the hand
int64_t lastFrameID = 0; // The last frame received

void initializeHistory() {
    handPositionHistory = (float*)malloc(swipeWindowSize * sizeof(float));
    if (handPositionHistory == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < swipeWindowSize; i++) {
        handPositionHistory[i] = 0;
    }
    historyIndex = 0;
}

void freeHistory() {
    free(handPositionHistory);
}

void addToHistory(float positionX) {
    handPositionHistory[historyIndex] = positionX;
    historyIndex = (historyIndex + 1) % swipeWindowSize;
}

float getAverageSwipeDistance() {
    float sum = 0;
    for (int i = 0; i < swipeWindowSize; i++) {
        sum += handPositionHistory[i];
    }
    return sum / swipeWindowSize;
}

void resetHistory() {
    for (int i = 0; i < swipeWindowSize; i++) {
        handPositionHistory[i] = 0;
    }
    historyIndex = 0;
}

void detectSwipe(float prevPositionX, float currPositionX) {
    // Add current position to history
    addToHistory(currPositionX);

    // Calculate average swipe distance
    float averageSwipeDistance = getAverageSwipeDistance();

    if (currPositionX - prevPositionX > swipeThreshold) {
        // Swipe RIGHT detected
        printf("Swipe LEFT detected.\n");
        resetHistory(); // Clear history after detection
        lastSwipeTime = time(NULL); // Update last swipe time
        #ifdef _WIN32
        Sleep(2000); // Wait for 2 seconds on Windows
        #else
        sleep(2); // Wait for 2 seconds on Linux/Unix
        #endif
    } else if (prevPositionX - currPositionX > swipeThreshold) {
        // Swipe LEFT detected
        printf("Swipe RIGHT detected.\n");
        resetHistory(); // Clear history after detection
        lastSwipeTime = time(NULL); // Update last swipe time
        #ifdef _WIN32
        Sleep(2000); // Wait for 2 seconds on Windows
        #else
        sleep(2); // Wait for 2 seconds on Linux/Unix
        #endif
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

    initializeHistory();

    for (;;) {
        LEAP_TRACKING_EVENT *frame = GetFrame();
        if (frame && (frame->tracking_frame_id > lastFrameID)) {
            lastFrameID = frame->tracking_frame_id;

            if (frame->nHands > 0) {
                handDetected = 1; // Hand detected
                for (uint32_t h = 0; h < frame->nHands; h++) {
                    LEAP_HAND* hand = &frame->pHands[h];

                    float currentHandPositionX = hand->palm.position.x;

                    // Detect swipe
                    detectSwipe(lastHandPositionX, currentHandPositionX);

                    // Update last position for the next frame
                    lastHandPositionX = currentHandPositionX;
                }
            } else if (handDetected) {
                // No hands detected, reset history
                printf("No hand detected, clearing history.\n");
                resetHistory();
                handDetected = 0; // No hand detected
            }
        }
        // Sleep to avoid high CPU usage; adjust sleep time as needed
        #ifdef _WIN32
        Sleep(10); // Sleep for 10 milliseconds on Windows
        #else
        usleep(10000); // Sleep for 10 milliseconds on Linux/Unix
        #endif
    } // ctrl-c to exit

    freeHistory();
    return 0;
}
