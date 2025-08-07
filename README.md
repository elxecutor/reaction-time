# Enhanced Reaction Time Game – Arduino Project v2.0

This is an advanced multi-team Reaction Time Game built using Arduino with enhanced features including persistent data storage, multiple difficulty modes, comprehensive statistics, and improved user experience.

## 🚀 New Features in v2.0

### ✨ Enhanced Gameplay
- **3 Difficulty Modes**: Easy, Normal, and Hard with different timing parameters
- **Improved Visual Feedback**: Dynamic RGB LED colors and animated responses
- **Better Audio**: Multi-pulse tones and celebration sounds
- **Performance Categories**: From "INCREDIBLE!" to "Too slow!" with appropriate feedback

### 📊 Advanced Statistics
- **Persistent Data Storage**: Team statistics saved in EEPROM
- **Comprehensive Tracking**: Best time, average time, games played, and fast reactions count
- **Individual Team Records**: Each team maintains separate statistics
- **New Record Celebrations**: Special animations and sounds for new best times

### 🎮 Enhanced User Interface
- **Interactive Menus**: Navigate between game modes and statistics
- **Long Press Functions**: Hold button for 2 seconds to view team statistics
- **Auto-rotation**: Automatic team switching after each game
- **Startup Animation**: Beautiful RGB color cycle on boot
- **Timeout Protection**: Auto-return to main menu after inactivity

### 🔧 Technical Improvements
- **Fixed Pin Naming**: Corrected blue pin naming (was incorrectly named yellowPin)
- **Better Code Organization**: Structured functions and clear separation of concerns
- **Enhanced Error Handling**: Better early-press detection and user feedback
- **Memory Efficient**: Optimized data structures for EEPROM storage

## ⚙️ Hardware Requirements

### 🧩 Components Used:
- **Arduino UNO**
- **16x2 I2C LCD Display** (address 0x27)
- **RGB LED** (Common Cathode)
  - Red: Pin 11 (PWM)
  - Green: Pin 10 (PWM) 
  - Blue: Pin 9 (PWM)
- **Push Button** (Pin 2)
- **Buzzer** (Pin 8)
- **Resistors**: 3x 220Ω (for RGB LED), 1x 10kΩ (pull-up for button)

## 🖥️ Setup Instructions

### 🔧 Prerequisites
- [VS Code](https://code.visualstudio.com/)
- [PlatformIO extension](https://platformio.org/install)
- [Wokwi extension](https://docs.wokwi.com/guides/vscode) (for simulation)

### 🛠️ Installation

1. **Clone the Repository**
   ```bash
   git clone https://github.com/noiz-x/reaction-time
   cd reaction-time
   ```

2. **Open in VS Code**
   Launch VS Code and open the project folder.

3. **Build the Project**
   ```bash
   pio run
   ```

4. **Run Simulation** (Optional)
   Use `Wokwi: Start Simulator` from the Command Palette.

## 🎮 How to Play

### 📱 Navigation
- **Short Press**: Select/Start game
- **Long Press** (2 seconds): View team statistics
- **In Game Mode Menu**: Press to cycle through Easy → Normal → Hard

### 🎯 Game Flow

1. **Main Menu**
   - Shows current team name
   - Press button to enter game mode selection
   - Hold button to view statistics

2. **Mode Selection**
   - **Easy Mode**: Longer delays (2-6s), slower countdown (1200ms)
   - **Normal Mode**: Standard delays (1-5s), normal countdown (1000ms)
   - **Hard Mode**: Quick delays (0.5-3s), fast countdown (800ms)

3. **Game Round**
   - 3-second countdown with RGB visual feedback
   - Random delay period (varies by difficulty)
   - "GO!" signal with white LED and high-pitched tone
   - Press button as quickly as possible!

4. **Results & Feedback**
   - **< 150ms**: "INCREDIBLE!" (Cyan LED, celebration)
   - **150-199ms**: "Lightning Fast!" (Green LED)
   - **200-299ms**: "Excellent!" (Light Green LED)
   - **300-499ms**: "Good Job!" (Blue LED)
   - **500-799ms**: "Not bad!" (Yellow LED)
   - **800ms+**: "Too slow!" (Red LED)

5. **Statistics Tracking**
   - Best reaction time recorded
   - Running average calculated
   - Fast reactions (< 200ms) counted
   - All data automatically saved to EEPROM

### 🏆 Team Management

- **6 Default Teams**: Alpha, Beta, Gamma, Delta, Echo, Foxtrot
- **Automatic Rotation**: Teams cycle after each game
- **Individual Records**: Each team maintains separate statistics
- **Persistent Storage**: Data survives power cycles

## 📊 Statistics Features

### 📈 Tracked Metrics
- **Best Time**: Fastest reaction time ever recorded
- **Games Played**: Total number of games completed
- **Average Time**: Running average of all reaction times
- **Fast Reactions**: Count of reactions under 200ms

### 📋 Viewing Statistics
- Hold button for 2 seconds in main menu
- Cycles through all teams automatically
- Shows best time and games played on LCD
- Detailed statistics available via Serial Monitor

## 🔧 Customization Options

### 👥 Team Configuration
```cpp
// Modify team names (max 8 characters for LCD display)
const char* defaultNames[] = {"Team1", "Team2", "Team3", "Team4", "Team5", "Team6"};
```

### ⚡ Game Mode Tuning
```cpp
const GameModeSettings gameModes[] = {
  {minDelay, maxDelay, countdownSpeed, "ModeName"},
  // Customize timing parameters as needed
};
```

### 🎨 RGB Color Schemes
```cpp
// Add new performance categories with custom colors
void processReactionTime() {
  // Add custom time ranges and RGB colors
}
```

### 🔊 Audio Customization
```cpp
// Modify tone frequencies and patterns
void playTone(int frequency, int duration, int pulses);
```

## 🧠 Technical Details

### 💾 EEPROM Usage
- **Magic Number**: 0xAB for data validation
- **Team Data Structure**: Name, best time, games played, total time, fast reactions
- **Auto-Save**: Data automatically saved after each game
- **Data Recovery**: Initializes defaults if EEPROM data is invalid

### 🔌 Pin Configuration
```cpp
const int redPin = 11;    // RGB Red (PWM)
const int greenPin = 10;  // RGB Green (PWM)
const int bluePin = 9;    // RGB Blue (PWM)
const int buttonPin = 2;  // Push Button
const int buzzerPin = 8;  // Buzzer
```

### 📡 Serial Communication
- **Baud Rate**: 9600
- **Debug Output**: Game events, statistics, and system status
- **Team Information**: Detailed performance data logging

## 🚨 Troubleshooting

### Common Issues
1. **LCD Not Displaying**: Check I2C address (default 0x27)
2. **Button Not Responsive**: Verify pull-up resistor (10kΩ)
3. **RGB LED Issues**: Ensure PWM-capable pins and correct wiring
4. **Statistics Not Saving**: Check EEPROM functionality
5. **Timing Issues**: Verify crystal oscillator on Arduino

### 🐛 Debug Mode
Enable verbose serial output for troubleshooting:
```cpp
#define DEBUG_MODE 1  // Add to top of main.cpp
```

## 🛡️ Safety Notes

- Use appropriate resistors to prevent LED damage
- Ensure proper power supply (5V for Arduino UNO)
- Verify all connections before powering on
- Use debounced button handling to prevent false triggers

## 📚 Libraries Used

- **LiquidCrystal_I2C**: v1.1.4 - LCD display control
- **EEPROM**: Built-in - Persistent data storage
- **Arduino**: Standard Arduino framework

## 🤝 Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Original concept inspired by classic reaction time games
- Enhanced with modern Arduino capabilities
- Community feedback and testing

---

**Happy Gaming! 🎮⚡**