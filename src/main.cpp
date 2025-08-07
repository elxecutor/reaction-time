#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

// Initialize LCD (address 0x27, 16 columns, 2 rows)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pin definitions
const int redPin = 11;      // PWM - Red
const int greenPin = 10;    // PWM - Green
const int bluePin = 9;      // PWM - Blue (fixed naming)
const int buttonPin = 2;    // Button
const int buzzerPin = 8;    // Buzzer

// Game variables
unsigned long startTime;    // Timer start
unsigned long reactionTime; // Reaction time in ms
bool gameRunning = false;   // Game state
bool gameStarted = false;   // Whether any game has started
int gameMode = 0;          // 0=Easy, 1=Normal, 2=Hard

// Team management
struct TeamData {
  char name[9];           // Team name (8 chars + null terminator)
  unsigned long bestTime; // Best reaction time
  int gamesPlayed;        // Number of games played
  unsigned long totalTime; // Total reaction time for average
  int fastReactions;      // Count of reactions < 200ms
};

const int numTeams = 6;
TeamData teams[numTeams];
int currentTeamIndex = 0;

// EEPROM addresses for persistent storage
const int EEPROM_MAGIC = 0xAB; // Magic number to verify valid data
const int EEPROM_START = 0;    // Start address for team data

// Game mode settings
struct GameModeSettings {
  int minDelay;    // Minimum delay before "GO!"
  int maxDelay;    // Maximum delay before "GO!"
  int countdownSpeed; // Countdown speed in ms
  const char* name;
};

const GameModeSettings gameModes[] = {
  {2000, 6000, 1200, "Easy"},   // Longer delays, slower countdown
  {1000, 5000, 1000, "Normal"}, // Original settings
  {500, 3000, 800, "Hard"}      // Shorter delays, faster countdown
};

// Function declarations
void initializeTeams();
void saveTeamData();
void loadTeamData();
void resetDisplay();
void setRGB(int red, int green, int blue);
void showMainMenu();
void showGameModeMenu();
void showTeamStats();
void playTone(int frequency, int duration, int pulses = 1);
void showCountdown();
bool checkEarlyPress();
void processReactionTime();
void nextTeam();
void showCalibration();
void startupAnimation();
void startGame();

void setup() {
  // Initialize pins
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  setRGB(0, 0, 0); // RGB off
  digitalWrite(buzzerPin, LOW);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  
  // Initialize serial communication
  Serial.begin(9600);

  
  // Load team data from EEPROM or initialize defaults
  loadTeamData();
  
  // Show startup animation
  startupAnimation();
  
  // Show main menu
  showMainMenu();

}

void loop() {
  static unsigned long lastButtonPress = 0;
  static int menuState = 0; // 0=main, 1=game_mode, 2=stats, 3=playing
  static unsigned long buttonPressTime = 0;
  
  // Debounce button
  if (digitalRead(buttonPin) == HIGH && millis() - lastButtonPress > 300) {
    lastButtonPress = millis();
    buttonPressTime = millis();
    
    switch (menuState) {
      case 0: // Main menu
        menuState = 1;
        showGameModeMenu();
        break;
        
      case 1: // Game mode selection
        gameMode = (gameMode + 1) % 3;
        showGameModeMenu();
        // Auto-start game after mode selection
        delay(1000);
        menuState = 3;
        gameStarted = true;
        startGame();
        break;
        
      case 2: // Stats view
        menuState = 0;
        showMainMenu();
        break;
        
      case 3: // Game running
        if (!gameRunning) {
          gameRunning = true;
          startGame();
        }
        break;
    }
  }
  
  // Long press for stats (hold button for 2 seconds in main menu)
  if (menuState == 0 && digitalRead(buttonPin) == HIGH && 
      buttonPressTime > 0 && millis() - buttonPressTime > 2000) {
    buttonPressTime = 0;
    menuState = 2;
    showTeamStats();
  }
  
  // Auto-return to main menu after inactivity
  static unsigned long lastActivity = millis();
  if (digitalRead(buttonPin) == HIGH || gameRunning) {
    lastActivity = millis();
  }
  
  if (menuState != 0 && !gameRunning && millis() - lastActivity > 30000) {
    menuState = 0;
    showMainMenu();
  }
}

// Enhanced setRGB function with blue pin fix
void setRGB(int red, int green, int blue) {
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue); // Fixed from yellowPin
}

// Initialize team data with default values
void initializeTeams() {
  const char* defaultNames[] = {"Alpha", "Beta", "Gamma", "Delta", "Echo", "Foxtrot"};
  
  for (int i = 0; i < numTeams; i++) {
    strcpy(teams[i].name, defaultNames[i]);
    teams[i].bestTime = 9999;
    teams[i].gamesPlayed = 0;
    teams[i].totalTime = 0;
    teams[i].fastReactions = 0;
  }
}

// Save team data to EEPROM
void saveTeamData() {
  EEPROM.write(EEPROM_START, EEPROM_MAGIC); // Magic number
  
  for (int i = 0; i < numTeams; i++) {
    int addr = EEPROM_START + 1 + (i * sizeof(TeamData));
    EEPROM.put(addr, teams[i]);
  }
  
  Serial.println("Team data saved to EEPROM");
}

// Load team data from EEPROM
void loadTeamData() {
  if (EEPROM.read(EEPROM_START) == EEPROM_MAGIC) {
    // Valid data exists, load it
    for (int i = 0; i < numTeams; i++) {
      int addr = EEPROM_START + 1 + (i * sizeof(TeamData));
      EEPROM.get(addr, teams[i]);
    }
    Serial.println("Team data loaded from EEPROM");
  } else {
    // No valid data, initialize defaults
    initializeTeams();
    saveTeamData();
    Serial.println("Initialized default team data");
  }
}

// Startup animation
void startupAnimation() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("REACTION TIMER");
  lcd.setCursor(0, 1);
  lcd.print("Enhanced v2.0");
  
  // RGB color cycle
  for (int i = 0; i <= 255; i += 15) {
    setRGB(i, 0, 255 - i);
    delay(50);
  }
  for (int i = 0; i <= 255; i += 15) {
    setRGB(255 - i, i, 0);
    delay(50);
  }
  for (int i = 0; i <= 255; i += 15) {
    setRGB(0, 255 - i, i);
    delay(50);
  }
  
  setRGB(0, 0, 0);
  playTone(1000, 200, 3);
  delay(1000);
}

// Show main menu
void showMainMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Team: ");
  lcd.print(teams[currentTeamIndex].name);
  lcd.setCursor(0, 1);
  lcd.print("Press=Play Hold=Stats");
  
  setRGB(0, 100, 200); // Soft blue
  
  Serial.print("Main Menu - Current Team: ");
  Serial.println(teams[currentTeamIndex].name);
}

// Show game mode selection
void showGameModeMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Mode: ");
  lcd.print(gameModes[gameMode].name);
  lcd.setCursor(0, 1);
  lcd.print("Press to change");
  
  // Different colors for different modes
  switch (gameMode) {
    case 0: setRGB(0, 255, 0); break;   // Green for Easy
    case 1: setRGB(255, 255, 0); break; // Yellow for Normal
    case 2: setRGB(255, 0, 0); break;   // Red for Hard
  }
  
  Serial.print("Game Mode: ");
  Serial.println(gameModes[gameMode].name);
}

// Show team statistics
void showTeamStats() {
  static int currentStatTeam = 0;
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(teams[currentStatTeam].name);
  lcd.print(" Stats");
  
  lcd.setCursor(0, 1);
  if (teams[currentStatTeam].gamesPlayed > 0) {
    lcd.print("Best:");
    lcd.print(teams[currentStatTeam].bestTime);
    lcd.print("ms");
  } else {
    lcd.print("No games played");
  }
  
  setRGB(100, 0, 100); // Purple for stats
  
  Serial.print("Stats for ");
  Serial.print(teams[currentStatTeam].name);
  Serial.print(": Best=");
  Serial.print(teams[currentStatTeam].bestTime);
  Serial.print("ms, Games=");
  Serial.print(teams[currentStatTeam].gamesPlayed);
  if (teams[currentStatTeam].gamesPlayed > 0) {
    Serial.print(", Avg=");
    Serial.print(teams[currentStatTeam].totalTime / teams[currentStatTeam].gamesPlayed);
    Serial.print("ms, Fast=");
    Serial.print(teams[currentStatTeam].fastReactions);
  }
  Serial.println();
  
  currentStatTeam = (currentStatTeam + 1) % numTeams;
  delay(3000); // Show each team for 3 seconds
}

// Enhanced tone function with pulse capability
void playTone(int frequency, int duration, int pulses) {
  for (int i = 0; i < pulses; i++) {
    tone(buzzerPin, frequency, duration / pulses);
    delay(duration / pulses + 50);
  }
}

// Start a game round
void startGame() {
  lcd.clear();
  Serial.print("Game Started! Team ");
  Serial.print(teams[currentTeamIndex].name);
  Serial.print(" - Mode: ");
  Serial.println(gameModes[gameMode].name);
  
  // Show team and mode
  lcd.setCursor(0, 0);
  lcd.print("Team: ");
  lcd.print(teams[currentTeamIndex].name);
  lcd.setCursor(0, 1);
  lcd.print("Mode: ");
  lcd.print(gameModes[gameMode].name);
  delay(2000);
  
  // Countdown
  showCountdown();
  
  // Check for early press during countdown
  if (checkEarlyPress()) {
    return; // Early press detected, game over
  }
  
  // Random delay phase
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Get Ready...");
  Serial.println("Get Ready...");
  setRGB(255, 255, 0); // Yellow for ready
  
  unsigned long delayStart = millis();
  unsigned long randomDelay = random(gameModes[gameMode].minDelay, gameModes[gameMode].maxDelay);
  
  while (millis() - delayStart < randomDelay) {
    if (digitalRead(buttonPin) == HIGH) {
      // Too early!
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Too Soon!");
      lcd.setCursor(0, 1);
      lcd.print("Try again...");
      
      Serial.print("Team ");
      Serial.print(teams[currentTeamIndex].name);
      Serial.println(": Too Soon!");
      
      setRGB(255, 100, 0); // Orange for error
      playTone(300, 800, 3);
      delay(3000);
      
      gameRunning = false;
      nextTeam();
      return;
    }
    delay(10); // Small delay to prevent excessive checking
  }
  
  // GO! signal
  setRGB(255, 255, 255); // White for GO
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("*** GO! ***");
  Serial.println("GO!");
  playTone(2000, 150, 1);
  startTime = millis();
  
  // Wait for button press
  while (digitalRead(buttonPin) == LOW) {
    delay(1); // Minimal delay for responsiveness
  }
  
  reactionTime = millis() - startTime;
  processReactionTime();
}

// Show countdown with visual and audio feedback
void showCountdown() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Get Ready...");
  
  for (int i = 3; i > 0; i--) {
    lcd.setCursor(0, 1);
    lcd.print("    ");
    lcd.setCursor(7, 1);
    lcd.print(i);
    
    Serial.print(i);
    Serial.println("...");
    
    // Visual countdown with colors
    switch (i) {
      case 3: setRGB(255, 0, 0); break;   // Red
      case 2: setRGB(255, 255, 0); break; // Yellow  
      case 1: setRGB(0, 255, 0); break;   // Green
    }
    
    playTone(800 + (i * 200), 200, 1);
    delay(gameModes[gameMode].countdownSpeed);
    
    // Check for early button press
    if (digitalRead(buttonPin) == HIGH) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Too Soon!");
      lcd.setCursor(0, 1);
      lcd.print("Wait for GO!");
      
      Serial.print("Team ");
      Serial.print(teams[currentTeamIndex].name);
      Serial.println(": Too Soon during countdown!");
      
      setRGB(255, 100, 0); // Orange
      playTone(300, 800, 3);
      delay(3000);
      
      gameRunning = false;
      nextTeam();
      return;
    }
  }
}

// Check for early button press
bool checkEarlyPress() {
  return digitalRead(buttonPin) == HIGH;
}

// Process and display reaction time results
void processReactionTime() {
  teams[currentTeamIndex].gamesPlayed++;
  teams[currentTeamIndex].totalTime += reactionTime;
  
  if (reactionTime < teams[currentTeamIndex].bestTime) {
    teams[currentTeamIndex].bestTime = reactionTime;
  }
  
  if (reactionTime < 200) {
    teams[currentTeamIndex].fastReactions++;
  }
  
  // Visual and audio feedback based on performance
  lcd.clear();
  lcd.setCursor(0, 0);
  
  if (reactionTime < 150) {
    lcd.print("INCREDIBLE!");
    setRGB(0, 255, 255); // Cyan
    playTone(2500, 400, 3);
  } else if (reactionTime < 200) {
    lcd.print("Lightning Fast!");
    setRGB(0, 255, 0); // Green
    playTone(2000, 300, 2);
  } else if (reactionTime < 300) {
    lcd.print("Excellent!");
    setRGB(100, 255, 100); // Light green
    playTone(1800, 250, 2);
  } else if (reactionTime < 500) {
    lcd.print("Good Job!");
    setRGB(0, 100, 255); // Blue
    playTone(1500, 200, 1);
  } else if (reactionTime < 800) {
    lcd.print("Not bad!");
    setRGB(255, 255, 0); // Yellow
    playTone(1200, 300, 1);
  } else {
    lcd.print("Too slow!");
    setRGB(255, 0, 0); // Red
    playTone(800, 500, 1);
  }
  
  lcd.setCursor(0, 1);
  lcd.print(teams[currentTeamIndex].name);
  lcd.print(": ");
  lcd.print(reactionTime);
  lcd.print("ms");
  
  Serial.print("Team ");
  Serial.print(teams[currentTeamIndex].name);
  Serial.print(": ");
  Serial.print(reactionTime);
  Serial.print("ms (Best: ");
  Serial.print(teams[currentTeamIndex].bestTime);
  Serial.print("ms, Avg: ");
  if (teams[currentTeamIndex].gamesPlayed > 0) {
    Serial.print(teams[currentTeamIndex].totalTime / teams[currentTeamIndex].gamesPlayed);
  }
  Serial.println("ms)");
  
  delay(4000);
  
  // Show best time if it's a new record
  if (reactionTime == teams[currentTeamIndex].bestTime && teams[currentTeamIndex].gamesPlayed > 1) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("NEW BEST TIME!");
    lcd.setCursor(0, 1);
    lcd.print("Congratulations!");
    
    // Celebration animation
    for (int i = 0; i < 5; i++) {
      setRGB(255, 0, 255); // Magenta
      delay(200);
      setRGB(0, 255, 255); // Cyan
      delay(200);
    }
    
    playTone(2000, 100, 5);
    delay(3000);
  }
  
  setRGB(0, 0, 0);
  gameRunning = false;
  
  // Save data and move to next team
  saveTeamData();
  nextTeam();
}

// Move to next team
void nextTeam() {
  currentTeamIndex = (currentTeamIndex + 1) % numTeams;
  delay(1000);
  showMainMenu();
}

// Reset display to default state
void resetDisplay() {
  showMainMenu();
}