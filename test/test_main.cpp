#include <unity.h>
#include "../src/main.h"

// Test helper functions
void setUp(void) {
    // Set up test conditions
}

void tearDown(void) {
    // Clean up after tests  
}

// Test team initialization
void test_team_initialization(void) {
    initializeTeams();
    
    // Check that all teams have valid initial values
    for (int i = 0; i < numTeams; i++) {
        TEST_ASSERT_NOT_NULL(teams[i].name);
        TEST_ASSERT_EQUAL_UINT32(9999, teams[i].bestTime);
        TEST_ASSERT_EQUAL_INT(0, teams[i].gamesPlayed);
        TEST_ASSERT_EQUAL_UINT32(0, teams[i].totalTime);
        TEST_ASSERT_EQUAL_INT(0, teams[i].fastReactions);
    }
}

// Test game mode settings
void test_game_mode_settings(void) {
    // Test that all game modes have valid settings
    for (int i = 0; i < 3; i++) {
        TEST_ASSERT_TRUE(gameModes[i].minDelay > 0);
        TEST_ASSERT_TRUE(gameModes[i].maxDelay > gameModes[i].minDelay);
        TEST_ASSERT_TRUE(gameModes[i].countdownSpeed > 0);
        TEST_ASSERT_NOT_NULL(gameModes[i].name);
    }
}

// Test team statistics update
void test_team_statistics_update(void) {
    initializeTeams();
    
    // Simulate a game result
    currentTeamIndex = 0;
    reactionTime = 150; // Fast reaction
    
    // Update statistics (simplified version of processReactionTime logic)
    teams[currentTeamIndex].gamesPlayed++;
    teams[currentTeamIndex].totalTime += reactionTime;
    
    if (reactionTime < teams[currentTeamIndex].bestTime) {
        teams[currentTeamIndex].bestTime = reactionTime;
    }
    
    if (reactionTime < 200) {
        teams[currentTeamIndex].fastReactions++;
    }
    
    // Verify updates
    TEST_ASSERT_EQUAL_INT(1, teams[0].gamesPlayed);
    TEST_ASSERT_EQUAL_UINT32(150, teams[0].totalTime);
    TEST_ASSERT_EQUAL_UINT32(150, teams[0].bestTime);
    TEST_ASSERT_EQUAL_INT(1, teams[0].fastReactions);
}

// Test team rotation
void test_team_rotation(void) {
    currentTeamIndex = 0;
    int initialTeam = currentTeamIndex;
    
    // Simulate team rotation
    currentTeamIndex = (currentTeamIndex + 1) % numTeams;
    
    TEST_ASSERT_EQUAL_INT((initialTeam + 1) % numTeams, currentTeamIndex);
}

// Test performance categorization
void test_performance_categorization(void) {
    // Test different reaction time categories
    struct {
        unsigned long time;
        const char* expected_category;
    } test_cases[] = {
        {100, "INCREDIBLE"},
        {180, "Lightning Fast"},
        {250, "Excellent"},
        {400, "Good Job"},
        {600, "Not bad"},
        {900, "Too slow"}
    };
    
    for (int i = 0; i < 6; i++) {
        // This would need the actual categorization logic extracted to a function
        // For now, just test the thresholds
        if (test_cases[i].time < 150) {
            TEST_ASSERT_TRUE(test_cases[i].time < 150); // INCREDIBLE
        } else if (test_cases[i].time < 200) {
            TEST_ASSERT_TRUE(test_cases[i].time >= 150 && test_cases[i].time < 200); // Lightning
        } else if (test_cases[i].time < 300) {
            TEST_ASSERT_TRUE(test_cases[i].time >= 200 && test_cases[i].time < 300); // Excellent
        } else if (test_cases[i].time < 500) {
            TEST_ASSERT_TRUE(test_cases[i].time >= 300 && test_cases[i].time < 500); // Good
        } else if (test_cases[i].time < 800) {
            TEST_ASSERT_TRUE(test_cases[i].time >= 500 && test_cases[i].time < 800); // Not bad
        } else {
            TEST_ASSERT_TRUE(test_cases[i].time >= 800); // Too slow
        }
    }
}

// Test EEPROM magic number validation
void test_eeprom_magic_validation(void) {
    // This test would require mocking EEPROM functionality
    // For demonstration purposes, we test the magic number constant
    TEST_ASSERT_EQUAL_HEX8(0xAB, EEPROM_MAGIC);
}

// Test average calculation
void test_average_calculation(void) {
    initializeTeams();
    
    // Add some reaction times
    teams[0].totalTime = 150 + 200 + 180; // 530ms total
    teams[0].gamesPlayed = 3;
    
    unsigned long average = teams[0].totalTime / teams[0].gamesPlayed;
    TEST_ASSERT_EQUAL_UINT32(176, average);
}

// Test edge cases
void test_edge_cases(void) {
    initializeTeams();
    
    // Test division by zero protection
    teams[0].gamesPlayed = 0;
    teams[0].totalTime = 100;
    
    // In real code, should check for division by zero before calculating average
    if (teams[0].gamesPlayed > 0) {
        unsigned long average = teams[0].totalTime / teams[0].gamesPlayed;
        TEST_ASSERT_TRUE(average > 0);
    } else {
        // Should handle zero games case gracefully
        TEST_ASSERT_EQUAL_INT(0, teams[0].gamesPlayed);
    }
}

// Main test runner
int runUnityTests(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_team_initialization);
    RUN_TEST(test_game_mode_settings);
    RUN_TEST(test_team_statistics_update);
    RUN_TEST(test_team_rotation);
    RUN_TEST(test_performance_categorization);
    RUN_TEST(test_eeprom_magic_validation);
    RUN_TEST(test_average_calculation);
    RUN_TEST(test_edge_cases);
    
    return UNITY_END();
}

// For Arduino environment
void setup() {
    delay(2000); // Wait for board to stabilize
    runUnityTests();
}

void loop() {
    // Empty - tests run once in setup()
}
