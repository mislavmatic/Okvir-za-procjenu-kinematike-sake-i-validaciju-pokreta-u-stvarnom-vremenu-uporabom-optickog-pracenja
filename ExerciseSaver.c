#include "ExampleConnection.h" // ovo će includati LeapC.h
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

/*
gcc ExerciseSaver.c build/ExampleConnection.o -Wall -Wextra -o build/ExerciseSaver -L/usr/lib/ultraleap-hand-tracking-service -lLeapC
*/

int endLoop = 0;

void endRecording(int sig)
{
   if (sig == SIGINT)
      endLoop = 1;
}

int main(int argc, char** argv)
{
   if(argc != 2) {
      fprintf(stderr, "Usage: %s nameOfFile\n", argv[0]);
      exit(EXIT_FAILURE);
   }

   struct sigaction sact;
   sact.sa_handler = endRecording;
   sact.sa_flags = SA_RESTART; // ili 0 ili ta god, RESTART znači da će se fje pokušat nastavit izvodit ako se handler pozvao usred izvođenja tih fja.
   sigaction(SIGINT, &sact, NULL);

   const char* filePath = argv[1];

   OpenConnection();
   while(!IsConnected) {
      millisleep(100);
   }
   printf("Connected with leap motion device\nPress Ctrl-c to stop recording\nPress Enter to start recording\n");
   getchar();

   LEAP_RECORDING recordingHandle;
   LEAP_RECORDING_PARAMETERS params;

   params.mode = eLeapRecordingFlags_Writing;
   eLeapRS result = LeapRecordingOpen(&recordingHandle, filePath, params);
   if (LEAP_SUCCEEDED(result)) {
      int64_t lastFrameID = 0; //The last frame received
      int frameCount = 0;
      while(!endLoop) {
         LEAP_TRACKING_EVENT *frame = GetFrame();
         if(frame && (frame->tracking_frame_id > lastFrameID)) {
            lastFrameID = frame->tracking_frame_id;
            frameCount++;
            uint64_t dataWritten = 0;
            result = LeapRecordingWrite(recordingHandle, frame, &dataWritten);
            if(!LEAP_SUCCEEDED(result)) {
               fprintf(stderr, "Error when writing to disk\n");
               //exit(EXIT_FAILURE);
            } else if (dataWritten == 0) {
               fprintf(stderr, "Data written == 0!\n");
               // exit(EXIT_FAILURE);
            } else if (frame->nHands != 1) {
               fprintf(stderr, "Number of hands must always be 1!\n");
               // exit(EXIT_FAILURE);
            }
            // printf("Recorded %"PRIu64" bytes for frame %"PRIu64" with %i hands.\n", dataWritten, frame->tracking_frame_id, frame->nHands);
            for(size_t i = 0; i < frame->nHands; i++) {
               printf("Hand %zu, Palm coordinates: %f, %f, %f\n", i, frame->pHands->palm.position.x, frame->pHands->palm.position.y, frame->pHands->palm.position.z); // umjesto i mogu frame->pHands->id
            }
         }
      }
      result = LeapRecordingClose(&recordingHandle);
      if(!LEAP_SUCCEEDED(result)) {
         fprintf(stderr, "Failed to close recording: %s\n", ResultString(result));
         exit(EXIT_FAILURE);
      }

      printf("Number of frames: %d\n", frameCount);
   } else {
      fprintf(stderr, "Failed to open recording for writing: %s\n", ResultString(result));
      exit(EXIT_FAILURE);
   }

   CloseConnection();
   DestroyConnection();
   exit(EXIT_SUCCESS);
}
