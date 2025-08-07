#ifndef CONFIG_H
#define CONFIG_H

// ===== HARDWARE CONFIGURATION =====

// Pin definitions
#define RED_PIN 11      // RGB Red LED (PWM capable)
#define GREEN_PIN 10    // RGB Green LED (PWM capable) 
#define BLUE_PIN 9      // RGB Blue LED (PWM capable)
#define BUTTON_PIN 2    // Push button input
#define BUZZER_PIN 8    // Buzzer output

// LCD Configuration
#define LCD_ADDRESS 0x27  // I2C address for LCD
#define LCD_COLS 16       // LCD columns
#define LCD_ROWS 2        // LCD rows

// ===== GAME CONFIGURATION =====

// Team settings
#define MAX_TEAMS 6
#define TEAM_NAME_LENGTH 9  // 8 characters + null terminator

// Default team names
#define DEFAULT_TEAM_NAMES {"Alpha", "Beta", "Gamma", "Delta", "Echo", "Foxtrot"}

// Game mode settings (minDelay, maxDelay, countdownSpeed)
#define EASY_MODE_MIN_DELAY 2000
#define EASY_MODE_MAX_DELAY 6000
#define EASY_MODE_COUNTDOWN_SPEED 1200

#define NORMAL_MODE_MIN_DELAY 1000
#define NORMAL_MODE_MAX_DELAY 5000
#define NORMAL_MODE_COUNTDOWN_SPEED 1000

#define HARD_MODE_MIN_DELAY 500
#define HARD_MODE_MAX_DELAY 3000
#define HARD_MODE_COUNTDOWN_SPEED 800

// Performance thresholds (in milliseconds)
#define INCREDIBLE_THRESHOLD 150
#define LIGHTNING_THRESHOLD 200
#define EXCELLENT_THRESHOLD 300
#define GOOD_THRESHOLD 500
#define OKAY_THRESHOLD 800

// ===== SYSTEM CONFIGURATION =====

// EEPROM settings
#define EEPROM_MAGIC 0xAB
#define EEPROM_START_ADDRESS 0

// Timing settings
#define BUTTON_DEBOUNCE_MS 300
#define LONG_PRESS_MS 2000
#define AUTO_TIMEOUT_MS 30000
#define RESULT_DISPLAY_MS 4000
#define CELEBRATION_MS 3000

// Serial communication
#define SERIAL_BAUD_RATE 9600

// ===== RGB COLOR DEFINITIONS =====

// Performance feedback colors (R, G, B values 0-255)
#define COLOR_INCREDIBLE 0, 255, 255    // Cyan
#define COLOR_LIGHTNING 0, 255, 0       // Green
#define COLOR_EXCELLENT 100, 255, 100   // Light Green
#define COLOR_GOOD 0, 100, 255          // Blue
#define COLOR_OKAY 255, 255, 0          // Yellow
#define COLOR_SLOW 255, 0, 0            // Red

// System state colors
#define COLOR_MENU 0, 100, 200          // Soft Blue
#define COLOR_EASY_MODE 0, 255, 0       // Green
#define COLOR_NORMAL_MODE 255, 255, 0   // Yellow
#define COLOR_HARD_MODE 255, 0, 0       // Red
#define COLOR_STATS 100, 0, 100         // Purple
#define COLOR_ERROR 255, 100, 0         // Orange
#define COLOR_READY 255, 255, 0         // Yellow
#define COLOR_GO 255, 255, 255          // White
#define COLOR_OFF 0, 0, 0               // Off

// Countdown colors
#define COLOR_COUNTDOWN_3 255, 0, 0     // Red
#define COLOR_COUNTDOWN_2 255, 255, 0   // Yellow
#define COLOR_COUNTDOWN_1 0, 255, 0     // Green

// Celebration colors
#define COLOR_CELEBRATION_1 255, 0, 255 // Magenta
#define COLOR_CELEBRATION_2 0, 255, 255 // Cyan

// ===== AUDIO CONFIGURATION =====

// Tone frequencies (Hz)
#define TONE_STARTUP 1000
#define TONE_COUNTDOWN_BASE 800
#define TONE_GO 2000
#define TONE_INCREDIBLE 2500
#define TONE_LIGHTNING 2000
#define TONE_EXCELLENT 1800
#define TONE_GOOD 1500
#define TONE_OKAY 1200
#define TONE_SLOW 800
#define TONE_ERROR 300
#define TONE_CELEBRATION 2000

// Tone durations (ms)
#define DURATION_SHORT 100
#define DURATION_MEDIUM 200
#define DURATION_LONG 400
#define DURATION_ERROR 800

// ===== DEBUG CONFIGURATION =====

// Uncomment to enable debug mode
// #define DEBUG_MODE 1

#ifdef DEBUG_MODE
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

#endif // CONFIG_H
