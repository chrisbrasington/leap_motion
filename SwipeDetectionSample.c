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

// Debounce settings
const float swipeThreshold = 0.05; // Adjust threshold based on your needs
const int debounceInterval = 2; // Increased debounce interval in seconds
const int swipeWindowSize = 10; // Number of data points to average swipe distance

time_t lastSwipeTime = 0; // Last time a swipe was detected
float* fingerPositionHistory; // Swipe history buffer
int historyIndex = 0; // Current index for history buffer
int fingerDetected = 0; // Flag to indicate whether the finger is detected
float lastFingerPositionX = 0; // Last X position of the finger
int64_t lastFrameID = 0; // The last frame received

void initializeHistory() {
    fingerPositionHistory = (float*)malloc(swipeWindowSize * sizeof(float));
    if (fingerPositionHistory == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < swipeWindowSize; i++) {
        fingerPositionHistory[i] = 0;
    }
    historyIndex = 0;
}

void freeHistory() {
    free(fingerPositionHistory);
}

void addToHistory(float positionX) {
    fingerPositionHistory[historyIndex] = positionX;
    historyIndex = (historyIndex + 1) % swipeWindowSize;
}

float getAverageSwipeDistance() {
    float sum = 0;
    for (int i = 0; i < swipeWindowSize; i++) {
        sum += fingerPositionHistory[i];
    }
    return sum / swipeWindowSize;
}

void resetHistory() {
    for (int i = 0; i < swipeWindowSize; i++) {
        fingerPositionHistory[i] = 0;
    }
    historyIndex = 0;
}

void detectSwipe(float prevPositionX, float currPositionX) {
    time_t currentTime = time(NULL);

    if (currentTime - lastSwipeTime < debounceInterval) {
        // Skip swipe detection if within debounce interval
        return;
    }

    // Add current position to history
    addToHistory(currPositionX);

    // Calculate average swipe distance
    float averageSwipeDistance = getAverageSwipeDistance();

    if (currPositionX - prevPositionX > averageSwipeDistance + swipeThreshold) {
        printf("Swipe LEFT detected.\n");
        // Simulate Ctrl + Page Down
        if (!system("ydotool key CTRL+ALT+LEFT")) {
            printf("Simulated CTRL+ALT+LEFT\n");
        }
        lastSwipeTime = currentTime; // Update last swipe time
    } else if (prevPositionX - currPositionX > averageSwipeDistance + swipeThreshold) {
        printf("Swipe RIGHT detected.\n");
        // Simulate Ctrl + Page Up
        if (!system("ydotool key CTRL+ALT+RIGHT")) {
            printf("Simulated CTRL+ALT+RIGHT\n");
        }
        lastSwipeTime = currentTime; // Update last swipe time
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
                fingerDetected = 1; // Finger detected
                for (uint32_t h = 0; h < frame->nHands; h++) {
                    LEAP_HAND* hand = &frame->pHands[h];

                    // Check if the hand has any fingers
                    if (hand->nFingers > 0) {
                        LEAP_FINGER* indexFinger = &hand->pFingers[0]; // Assuming index finger is the first

                        // Use the tip position of the index finger
                        float currentFingerPositionX = indexFinger->tip.position.x;

                        // Detect swipe
                        detectSwipe(lastFingerPositionX, currentFingerPositionX);

                        // Update last position for the next frame
                        lastFingerPositionX = currentFingerPositionX;
                    }
                }
            } else if (fingerDetected) {
                // No fingers detected, reset history
                printf("No finger detected, clearing history.\n");
                resetHistory();
                fingerDetected = 0; // No finger detected
            }
        }
    } // ctrl-c to exit

    freeHistory();
    return 0;
}
// End-of-Sample
