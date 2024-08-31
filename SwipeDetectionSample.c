/* Copyright (C) 2012-2017 Ultraleap Limited. All rights reserved.
 *
 * Use of this code is subject to the terms of the Ultraleap SDK agreement
 * available at https://central.leapmotion.com/agreements/SdkAgreement unless
 * Ultraleap has signed a separate license agreement with you or your
 * organisation.
 *
 */

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
const int debounceInterval = 1; // Debounce interval in seconds
time_t lastSwipeTime = 0; // Last time a swipe was detected
float lastHandPositionX = 0; // Last X position of the hand
int64_t lastFrameID = 0; // The last frame received

void detectSwipe(float prevPositionX, float currPositionX) {
    time_t currentTime = time(NULL);

    if (currentTime - lastSwipeTime < debounceInterval) {
        // Skip swipe detection if within debounce interval
        return;
    }

    if (currPositionX - prevPositionX > swipeThreshold) {
        printf("Swipe RIGHT detected.\n");
        // Simulate Ctrl + Page Down
        if (!system("ydotool key CTRL+ALT+LEFT")) {
            printf("Simulated  CTRL+ALT+LEFT\n");
        }
        lastSwipeTime = currentTime; // Update last swipe time
    } else if (prevPositionX - currPositionX > swipeThreshold) {
        printf("Swipe LEFT detected.\n");
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

    for (;;) {
        LEAP_TRACKING_EVENT *frame = GetFrame();
        if (frame && (frame->tracking_frame_id > lastFrameID)) {
            lastFrameID = frame->tracking_frame_id;

            for (uint32_t h = 0; h < frame->nHands; h++) {
                LEAP_HAND* hand = &frame->pHands[h];

                float currentHandPositionX = hand->palm.position.x;

                // Detect swipe
                detectSwipe(lastHandPositionX, currentHandPositionX);

                // Update last position for the next frame
                lastHandPositionX = currentHandPositionX;
            }
        }
    } // ctrl-c to exit
    return 0;
}
// End-of-Sample
