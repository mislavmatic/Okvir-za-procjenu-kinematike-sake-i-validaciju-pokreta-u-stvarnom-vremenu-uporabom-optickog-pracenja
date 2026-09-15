#include "ExampleConnection.h"
#include "laLeap.h" // ovo icluda LeapC.h i raylib.h
#include <rlgl.h>
#include <inttypes.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

/*
gcc ExerciseReader.c build/ExampleConnection.o build/laLeap.o -Wall -Wextra -g -o build/ExerciseReader -L/usr/lib/ultraleap-hand-tracking-service -lLeapC -lm -I./raylib-5.5/include -L./raylib-5.5/lib -l:libraylib.a
gcc ExerciseReader.c build/ExampleConnection.o build/laLeap.o -Wall -Wextra -g -fsanitize=address -o build/ExerciseReader -L/usr/lib/ultraleap-hand-tracking-service -lLeapC -lm -I./raylib-5.5/include -L./raylib-5.5/lib -l:libraylib.a
gcc ExerciseReader.c build/ExampleConnection.o build/laLeap.o -O3 -march=native -flto -o build/ExerciseReaderO -L/usr/lib/ultraleap-hand-tracking-service -lLeapC -lm -I./raylib-5.5/include -L./raylib-5.5/lib -l:libraylib.a
*/

#define max(a,b)                \
   ({ __typeof__ (a) _a = (a);  \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define min(a,b)                \
   ({ __typeof__ (a) _a = (a);  \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

float min3f(float x, float y, float z)
{
   return fminf(fminf(x, y), z);
}

float max3f(float x, float y, float z)
{
   return fmaxf(fmaxf(x, y), z);
}

typedef struct {
   float x_min, x_max;
   float y_min, y_max;
   float z_min, z_max;
} Extremes;

typedef struct {
   LEAP_TRACKING_EVENT **frames; 
   size_t count;
   size_t capacity;
} Frames;

#define da_append_p(da, x)                                                           \
   do {                                                                              \
      if((da)->count >= (da)->capacity) {                                            \
         if((da)->capacity == 0) {                                                   \
            (da)->capacity = 100;                                                    \
         }                                                                           \
         else {                                                                      \
            (da)->capacity *= 2;                                                     \
         }                                                                           \
         (da)->frames = realloc((da)->frames, sizeof(**(da)->frames)*(da)->capacity);\
      }                                                                              \
      (da)->frames[(da)->count++] = (x);                                             \
   } while(0)

void freeFrames(Frames* frame)
{
   for(size_t i = 0; i < frame->count; i++) {
      free(frame->frames[i]);
   }
   free(frame->frames);
}

// ako datoteka nije .lmt samo će doći do UnkownError i prestat će i onda se ništa neće staviti u dinamičko polje, zato treba provjeravat jel polje prazno, ako je onda nešto ne valja
void readFrames(Frames* frames, const char* fileName)
{
   if (!frames || !fileName)
      return;

   LEAP_RECORDING recordingHandle;
   LEAP_RECORDING_PARAMETERS params;

   params.mode = eLeapRecordingFlags_Reading;
   eLeapRS result = LeapRecordingOpen(&recordingHandle, fileName, params);

   if(LEAP_SUCCEEDED(result)) {
      int frameCount = 0;
      while(1) {
         uint64_t nextFrameSize = 0;
         result = LeapRecordingReadSize(recordingHandle, &nextFrameSize);

         if(!LEAP_SUCCEEDED(result)) {
            fprintf(stderr, "Couldn't get next frame size: %s\n", ResultString(result));
            // break;
         }

         if(nextFrameSize > 0) {
            frameCount++;
            LEAP_TRACKING_EVENT *frame = malloc((size_t)nextFrameSize);
            result = LeapRecordingRead(recordingHandle, frame, nextFrameSize);
            if(LEAP_SUCCEEDED(result)) {
               da_append_p(frames, frame);

            } else {
              fprintf(stderr, "Could not read frame: %s\n", ResultString(result));
            }
         }
         else {
            fprintf(stderr, "nextFrameSize is <= 0: %"PRIu64"\n", nextFrameSize);
            break;
         }
      }
   printf("Number of frames: %d\n", frameCount);
   result = LeapRecordingClose(&recordingHandle);
   if(!LEAP_SUCCEEDED(result))
         fprintf(stderr, "Failed to close recording: %s\n", ResultString(result));
   } else {
       fprintf(stderr, "Couldn't open file, %s\n", ResultString(result));
   }
}

#define BIGGERSET(x, y) if((x) > (y)) (y) = (x)
#define SMALLERSET(x, y) if((x) < (y)) (y) = (x)
#define MINIMAXING(x, y, z, x_min, x_max, y_min, y_max, z_min, z_max) \
      SMALLERSET((x), (x_min));                                       \
      BIGGERSET((x), (x_max));                                        \
      SMALLERSET((y), (y_min));                                       \
      BIGGERSET((y), (y_max));                                        \
      SMALLERSET((z), (z_min));                                       \
      BIGGERSET((z), (z_max));

void normalize(Frames* f)
{
   if (!f)
      return;

   LEAP_HAND *hand = &f->frames[0]->pHands[0];
   Extremes position_mm = {
      .x_min = hand->arm.prev_joint.x, .x_max = hand->arm.prev_joint.x,
      .y_min = hand->arm.prev_joint.y, .y_max = hand->arm.prev_joint.y,
      .z_min = hand->arm.prev_joint.z, .z_max = hand->arm.prev_joint.z,
   };
   MINIMAXING(hand->arm.next_joint.x, hand->arm.next_joint.y, hand->arm.next_joint.z, position_mm.x_min, position_mm.x_max, position_mm.y_min, position_mm.y_max, position_mm.z_min, position_mm.z_max);
   MINIMAXING(hand->palm.position.x , hand->palm.position.y , hand->palm.position.z , position_mm.x_min, position_mm.x_max, position_mm.y_min, position_mm.y_max, position_mm.z_min, position_mm.z_max);
   for(size_t j = 0; j < 5; j++) {
      for(size_t k = 0; k < 4; k++) {
         MINIMAXING(hand->digits[j].bones[k].prev_joint.x, hand->digits[j].bones[k].prev_joint.y, hand->digits[j].bones[k].prev_joint.z, position_mm.x_min, position_mm.x_max, position_mm.y_min, position_mm.y_max, position_mm.z_min, position_mm.z_max);
         MINIMAXING(hand->digits[j].bones[k].next_joint.x, hand->digits[j].bones[k].next_joint.y, hand->digits[j].bones[k].next_joint.z, position_mm.x_min, position_mm.x_max, position_mm.y_min, position_mm.y_max, position_mm.z_min, position_mm.z_max);
      }
   }

   for(size_t i = 1; i < f->count; i++) {
      if (f->frames[i]->nHands != 1)
         continue;
      hand = &f->frames[i]->pHands[0];
      MINIMAXING(hand->arm.prev_joint.x, hand->arm.prev_joint.y, hand->arm.prev_joint.z, position_mm.x_min, position_mm.x_max, position_mm.y_min, position_mm.y_max, position_mm.z_min, position_mm.z_max);
      MINIMAXING(hand->arm.next_joint.x, hand->arm.next_joint.y, hand->arm.next_joint.z, position_mm.x_min, position_mm.x_max, position_mm.y_min, position_mm.y_max, position_mm.z_min, position_mm.z_max);
      MINIMAXING(hand->palm.position.x , hand->palm.position.y , hand->palm.position.z , position_mm.x_min, position_mm.x_max, position_mm.y_min, position_mm.y_max, position_mm.z_min, position_mm.z_max);
      for(size_t j = 0; j < 5; j++) {
         for(size_t k = 0; k < 4; k++) {
            MINIMAXING(hand->digits[j].bones[k].prev_joint.x, hand->digits[j].bones[k].prev_joint.y, hand->digits[j].bones[k].prev_joint.z, position_mm.x_min, position_mm.x_max, position_mm.y_min, position_mm.y_max, position_mm.z_min, position_mm.z_max);
            MINIMAXING(hand->digits[j].bones[k].next_joint.x, hand->digits[j].bones[k].next_joint.y, hand->digits[j].bones[k].next_joint.z, position_mm.x_min, position_mm.x_max, position_mm.y_min, position_mm.y_max, position_mm.z_min, position_mm.z_max);
            
         }
      }
   }

   float x_mean = position_mm.x_min + position_mm.x_max / 2;
   float y_mean = position_mm.y_min + position_mm.y_max / 2;
   float z_mean = position_mm.z_min + position_mm.z_max / 2;
   LEAP_VECTOR mean = (LEAP_VECTOR) {.x = x_mean, .y = y_mean, .z = z_mean};
   float M = max3f(position_mm.x_max - position_mm.x_min, position_mm.y_max - position_mm.y_min, position_mm.z_max - position_mm.z_min);

   // presilkaj sve za -_mean i skaliraj s 2/M
   for(size_t i = 0; i < f->count; i++) {
      if (f->frames[i]->nHands != 1)
         continue;
      hand = &f->frames[i]->pHands[0];
      hand->arm.prev_joint = vec3ScalMul(vec3Sub(hand->arm.prev_joint, mean), 2/M);
      hand->arm.next_joint = vec3ScalMul(vec3Sub(hand->arm.next_joint, mean), 2/M);

      hand->palm.position = vec3ScalMul(vec3Sub(hand->palm.position, mean), 2/M);
      for(size_t j = 0; j < 5; j++) {
         for(size_t k = 0; k < 4; k++) {
            hand->digits[j].bones[k].prev_joint = vec3ScalMul(vec3Sub(hand->digits[j].bones[k].prev_joint, mean), 2/M);
            hand->digits[j].bones[k].next_joint = vec3ScalMul(vec3Sub(hand->digits[j].bones[k].next_joint, mean), 2/M);
         }
      }
   }
}

float digit_dist(const LEAP_DIGIT *s, const LEAP_DIGIT *t)
{
   if (!s || !t)
      return 0;
   enum { DIGIT_SEGMENT_COUNT = 4};
   assert(sizeof(s->bones)/sizeof(s->bones[0]) == DIGIT_SEGMENT_COUNT);

   float bone_dist = 0;
   for(int i = 0; i < DIGIT_SEGMENT_COUNT; i++) {
      bone_dist += vec3Dist(s->bones[i].prev_joint, t->bones[i].prev_joint);
   }
   bone_dist += vec3Dist(s->bones[DIGIT_SEGMENT_COUNT-1].next_joint, t->bones[DIGIT_SEGMENT_COUNT-1].next_joint);
   return bone_dist;
}

float distanceA(const LEAP_TRACKING_EVENT *s, const LEAP_TRACKING_EVENT *t)
{
   if (!s || !t)
      return 0;

   if (s->nHands != 1 || t->nHands != 1) {
      return 0;
   }

   const LEAP_HAND *hand_s = &s->pHands[0];
   const LEAP_HAND *hand_t = &t->pHands[0];

   float palm_dist = vec3Dist(hand_s->palm.position, hand_t->palm.position); 

   float digits_dist = 0;
   for(int i = 0; i < 5; i++) {
      digits_dist += digit_dist(&hand_s->digits[i], &hand_t->digits[i]);
   }

   float arm_dist = 0;
   arm_dist  = vec3Dist(hand_s->arm.prev_joint, hand_t->arm.prev_joint);
   arm_dist += vec3Dist(hand_s->arm.next_joint, hand_t->arm.next_joint);

   float velocity_dist = vec3Dist(hand_s->palm.velocity, hand_t->palm.velocity);
   return palm_dist + digits_dist + arm_dist + velocity_dist;   
}

float distanceB(const LEAP_TRACKING_EVENT *s, const LEAP_TRACKING_EVENT *t)
{
   if (!s || !t)
      return 0;

   if (s->nHands != 1 || t->nHands != 1) {
      return 0;
   }

   const LEAP_HAND *hand_s = &s->pHands[0];
   const LEAP_HAND *hand_t = &t->pHands[0];

   float pinch_dist = fabs(hand_s->pinch_distance - hand_t->pinch_distance);
   float pinch_str  = fabs(hand_s->pinch_strength - hand_t->pinch_strength);
   float grab_ang   = fabs(hand_s->grab_angle - hand_t->grab_angle);
   float grab_str   = fabs(hand_s->grab_strength - hand_t->grab_strength);

   float palm_dist = vec3Dist(hand_s->palm.position, hand_t->palm.position); 

   float digits_dist = 0;
   for(int i = 0; i < 5; i++) {
      digits_dist += digit_dist(&hand_s->digits[i], &hand_t->digits[i]);
   }

   float arm_dist = 0;
   arm_dist  = vec3Dist(hand_s->arm.prev_joint, hand_t->arm.prev_joint);
   arm_dist += vec3Dist(hand_s->arm.next_joint, hand_t->arm.next_joint);

   float velocity_dist = vec3Dist(hand_s->palm.velocity, hand_t->palm.velocity);

   return palm_dist + digits_dist + arm_dist + velocity_dist + pinch_dist + pinch_str + grab_ang + grab_str;
}

float DTW_test(Frames* s, Frames* t, float (*distance)(const LEAP_TRACKING_EVENT*, const LEAP_TRACKING_EVENT*))
{
   if (!s || !t) {
      fprintf(stderr, "Nullptr...\n");
      exit(EXIT_FAILURE);
   }

   float DTW[s->count + 1][t->count + 1];
   for(size_t i = 1; i < s->count + 1; i++) {
      DTW[i][0] = INFINITY;
   }
   for(size_t j = 1; j < t->count + 1; j++) {
      DTW[0][j] = INFINITY;
   }
   DTW[0][0] = 0;

   for(size_t i = 0; i < s->count; i++) {
      for(size_t j = 0; j < t->count; j++) {
         float cost = distance(s->frames[i], t->frames[j]);
         DTW[i+1][j+1] = cost + min3f(DTW[i][j+1], DTW[i+1][j], DTW[i][j]);
      }
   }
   
   return DTW[s->count][t->count] / (s->count + t->count);
}

void renderHand(LEAP_TRACKING_EVENT *lte, Color color)
{
   if (lte->nHands != 1) {
      return;
   }

   LEAP_HAND *hand = &lte->pHands[0];
   const Vector3 *palm_pos = (Vector3*)&hand->palm.position;

   const Vector3* digits[5][4][2];
   for(size_t i = 0; i < 5; i++) {
      for(size_t j = 0; j < 4; j++) {
         digits[i][j][0] = (Vector3 *)&hand->digits[i].bones[j].prev_joint;
         digits[i][j][1] = (Vector3 *)&hand->digits[i].bones[j].next_joint;
      }
   }

   const Vector3 * arms_pos[2] = {(Vector3 *)&hand->arm.prev_joint, (Vector3 *)&hand->arm.next_joint};

   float cube_size = 0.05f;

   DrawSphere(*palm_pos, cube_size/2, color);

   for(size_t i = 0; i < 5; i++) {
      for(size_t j = 0; j < 4; j++) {
         DrawSphere(*digits[i][j][0], cube_size/2, color);
         DrawSphere(*digits[i][j][1], cube_size/2, color);
      }
   }
   DrawSphere(*arms_pos[0], cube_size/2, color);
   DrawSphere(*arms_pos[1], cube_size/2, color);
}

int64_t timeeval_to_us(struct timeval *tv2, struct timeval *tv1)
{
   int64_t us2 = tv2->tv_sec * 1000*1000 + tv2->tv_usec;
   int64_t us1 = tv1->tv_sec * 1000*1000 + tv1->tv_usec;
   return us2 - us1;
}

size_t find_idx(Frames *frames, int64_t time_us, int64_t *a)
{
   LEAP_TRACKING_EVENT *lte = frames->frames[0];
   *a += time_us;
   int64_t b = lte->info.timestamp + *a;
   size_t i = 0;
   while(i < frames->count - 1 && frames->frames[i+1]->info.timestamp < b) {
      i++;
   }
   return i;
}

const char *get_file_name(const char *path)
{
   const char *last_slash = strrchr(path, '/');

   if (last_slash != NULL) {
      return last_slash + 1;
   }

   return path;
}

int main(int argc, char** argv)
{
   int opt;
   bool render = false;
   
   while ((opt = getopt(argc, argv, "r")) != -1) {
      switch (opt) {
      case 'r':
         render = true;
         break;
      default: /* '?' */
         fprintf(stderr, "Usage: %s nameOfFirstFile nameOfSecondFile [-r]\n", argv[0]);
         exit(EXIT_FAILURE);
      }
   }
   if (argc - optind != 2) {
      fprintf(stderr, "Usage: %s nameOfFirstFile nameOfSecondFile [-r]\n", argv[0]);
      exit(EXIT_FAILURE);
   }

   const char* file_path1 = argv[optind];
   const char* file_path2 = argv[optind + 1];
   Frames frames1 = {0};
   Frames frames2 = {0};

   readFrames(&frames1, file_path1);
   readFrames(&frames2, file_path2);
   if (frames1.count == 0 || frames2.count == 0) {
      fprintf(stderr, "Unknown error (maybe one of the files is not an .lmt file)\n");
      exit(EXIT_FAILURE);
   }

   normalize(&frames1);
   normalize(&frames2);

   // float dtw_distanceA = DTW_test(&frames1, &frames2, distanceA);
   float dtw_distanceB = DTW_test(&frames1, &frames2, distanceB);
   // printf("DTW   distanca A između %s i %s je %f\n", filePath1, filePath2, dtw_distanceA);
   printf("DTW   distanca B između %s i %s je %f\n", file_path1, file_path2, dtw_distanceB);

   if (render) {
      int screen_width = 1900, screen_height = 1000;
      SetTraceLogLevel(LOG_NONE);
      SetConfigFlags(FLAG_MSAA_4X_HINT);
      InitWindow(screen_width, screen_height, "Visualization of .lmt files");

      bool paused = false;

      const char* pauza_str = "PAUZIRANO";
      Font def_font = GetFontDefault();
      int pauza_fnt_sz = 50;
      int spacing = 2;
      Vector2 fontPosition1 = { screen_width/2.0f - MeasureTextEx(def_font, pauza_str, (float)pauza_fnt_sz, spacing).x/2,
                                screen_height/2.0f - pauza_fnt_sz/2.0f - screen_height / 4.0f };

      Camera3D camera   = {0};
      camera.position   = (Vector3) {0, 10.0f, 10.0f};
      camera.target     = (Vector3) {0, 0, 0};
      camera.up         = (Vector3) {0, 1, 0};
      camera.fovy       = 45.0f;
      camera.projection = CAMERA_PERSPECTIVE;
      int camera_mode   = CAMERA_THIRD_PERSON;

      size_t frame1_position = 0;
      size_t frame2_position = 0;

      int64_t a1 = 0;
      int64_t a2 = 0;
      size_t frame1_idx_ts = 0;
      size_t frame2_idx_ts = 0;
      bool frames1_anim_done = false;
      bool frames2_anim_done = false;

      DisableCursor();
      SetTargetFPS(60);

      struct timeval tv0 = {0};
      gettimeofday(&tv0, NULL);
      while (!WindowShouldClose()) {
         struct timeval tv1;
         gettimeofday(&tv1, NULL);
         int64_t time_us = timeeval_to_us(&tv1, &tv0);
         tv0 = tv1;

         if (!paused) {
            if (!frames1_anim_done)
               frame1_idx_ts = find_idx(&frames1, time_us, &a1);
            else frame1_idx_ts = frames1.count - 1;
            if (!frames2_anim_done)
               frame2_idx_ts = find_idx(&frames2, time_us, &a2);
            else frame2_idx_ts = frames2.count - 1;
         }

         if (IsKeyPressed(KEY_P))
            paused = !paused;

         UpdateCamera(&camera, camera_mode);

         BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode3D(camera);

               rlPushMatrix();
                  rlTranslatef(-1, 0, 0);
                  renderHand(frames1.frames[frame1_idx_ts], RED);
               rlPopMatrix();

               rlPushMatrix();
                  rlTranslatef(1, 0, 0);
                  renderHand(frames2.frames[frame2_idx_ts], BLUE);
               rlPopMatrix();

               DrawGrid(10, 1);
            EndMode3D();

            DrawText(TextFormat("DTW distanca izmedu %s i %s je %f\n", file_path1, file_path2, dtw_distanceB), 10, 10, 20, BLACK);
            DrawText(TextFormat("1: %zu, 2: %zu", frame1_idx_ts, frame2_idx_ts), 10, 30, 20, BLACK);

            if (paused) {
               DrawTextEx(def_font, pauza_str, fontPosition1, pauza_fnt_sz, spacing, BLACK);
            }
         EndDrawing();

         if (!paused) {
            frame1_position++;
            frame2_position++;

            if (frame1_position >= frames1.count && frame2_position < frames2.count)
               frame1_position--;
            if (frame2_position >= frames2.count && frame1_position < frames1.count)
               frame2_position--;
            if (frame1_position >= frames1.count && frame2_position >= frames2.count) {
               frame1_position = frame2_position = 0;
            }

            if (frame1_idx_ts >= frames1.count - 1)
               frames1_anim_done = true;
            if (frame2_idx_ts >= frames2.count - 1)
               frames2_anim_done = true;
            if (frames1_anim_done && frames2_anim_done) {
               frames1_anim_done = frames2_anim_done = false;
               a1 = a2 = 0;
               frame1_idx_ts = frame2_idx_ts = 0;
            }
         }
      }

      CloseWindow();
   }

   char csvFileName[256] = {0};
   const char* file_name1 = get_file_name(file_path1);
   const char* file_name2 = get_file_name(file_path2);

   snprintf(csvFileName, sizeof(csvFileName),
             "./csv/%s_%s.csv", file_name1, file_name2);

   FILE *csv_file = fopen(csvFileName, "w");
   if (!csv_file) {
      fprintf(stderr, "Could not create a csv file\n");
      perror("csv file");
      exit(EXIT_FAILURE);
   }
   fprintf(csv_file, "Referentna snimka,Ispitna snimka,broj okvira referentne snimke,broj okvira ispitne snimke,DTW udaljenost\n");
   fprintf(csv_file, "%s,%s,%zu,%zu,%f\n", file_path1, file_path2, frames1.count, frames2.count, dtw_distanceB);

   fclose(csv_file);
   
   freeFrames(&frames1);
   freeFrames(&frames2);
}
