#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NUM_TEAMS 6
#define TEAM_NAME_LEN 9
#define NUM_GAME_MODES 3

typedef struct {
    char name[TEAM_NAME_LEN];
    unsigned long bestTime;
    int gamesPlayed;
    unsigned long totalTime;
    int fastReactions;
} TeamData;

typedef struct {
    int minDelay;
    int maxDelay;
    int countdownSpeed;
    const char* name;
} GameModeSettings;

extern TeamData teams[NUM_TEAMS];
extern int currentTeamIndex;
extern unsigned long reactionTime;
extern bool gameRunning;
extern int gameMode;

extern const GameModeSettings gameModes[NUM_GAME_MODES];

void initializeTeams(void);
void nextTeam(void);
void processReactionTime(void);

#ifdef __cplusplus
}
#endif

#endif
