//#include "main.h"
//
//#define G3  196
//#define GS3 208
//#define C4  262
//#define D4  294
//#define DS4 311
//
//#define LOW_FREQ 50
//#define HIGH_FREQ 300
//#define DURATION 100
//#define IMPACT_FREQ 800
//#define VIBRATION_FREQ 200
//#define IMPACT_DURATION 50
//#define VIBRATION_DURATION 450
//
//uint16_t melody[] = {
//    G3, C4, DS4, G3, C4, DS4, G3, C4, DS4, G3, C4, DS4,
//    DS4, DS4, D4, D4, C4, C4, GS3, GS3, G3, G3, G3, G3
//};
//
//uint16_t durations[] = {
//    500, 500, 1000, 500, 500, 1000, 500, 500, 1000, 500, 500, 1000,
//    500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500
//};
//
//
//void playTankHitSound() {
//    PWM_Change_Tone(IMPACT_FREQ, 100);
//    HAL_Delay(IMPACT_DURATION);
//    PWM_Change_Tone(VIBRATION_FREQ, 100);
//    HAL_Delay(VIBRATION_DURATION);
//    PWM_Change_Tone(0, 0);
//}
//
//
//
//void playMelody() {
//    for (int i = 0; i < sizeof(melody) / sizeof(melody[0]); i++) {
//        PWM_Change_Tone(melody[i], 100);
//        //HAL_Delay(durations[i]);
//    }
//}
//
//uint32_t noteStartTime = 0;
//int currentNote = 0;
//
//void playMelodyNonBlocking() {
//    uint32_t currentTime = HAL_GetTick();  // Get the current time
//
//    // If the duration of the current note has passed
//    if (currentTime - noteStartTime >= durations[currentNote]) {
//        // Stop the previous note
//        PWM_Change_Tone(0, 0);
//
//        printf("Playing note number: %d\n", currentNote);  // Debug print
//
//        // If we've played all the notes in the melody, start over
//        if (currentNote >= sizeof(melody) / sizeof(melody[0])) {
//            currentNote = 0;
//        }
//
//        // Play the next note
//        PWM_Change_Tone(melody[currentNote], 100);
//
//        // Update the start time and move to the next note
//        noteStartTime = currentTime;
//        currentNote++;
//
//        printf("Next note number: %d\n", currentNote);  // Debug print
//    }
//}
//
//
//
//void playExplosionSound() {
//    for (int i = 0; i < DURATION / 10; i++) {
//        PWM_Change_Tone(HIGH_FREQ, 100);
//        //HAL_Delay(5);
//        PWM_Change_Tone(LOW_FREQ, 100);
//        //HAL_Delay(5);
//    }
//    PWM_Change_Tone(0, 0);
//}
