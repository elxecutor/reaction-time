#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "main.h"

#ifdef __AVR__
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#endif

#ifdef __AVR__
// Arduino pin -> AVR port mapping
#define PIN2PORT(p) ((p) < 8 ? &PORTD : ((p) < 14 ? &PORTB : &PORTC))
#define PIN2DDR(p)  ((p) < 8 ? &DDRD  : ((p) < 14 ? &DDRB  : &DDRC))
#define PIN2PIN(p)  ((p) < 8 ? &PIND  : ((p) < 14 ? &PINB  : &PINC))
#define PIN2BIT(p)  ((p) < 8 ? (p) : ((p) < 14 ? (p) - 8 : (p) - 14))

#define PIN_WRITE(p, v) do { \
    if (v) *PIN2PORT(p) |= (1 << PIN2BIT(p)); \
    else   *PIN2PORT(p) &= ~(1 << PIN2BIT(p)); \
} while(0)
#define PIN_READ(p)  ((*PIN2PIN(p) >> PIN2BIT(p)) & 1)
#define PIN_OUTPUT(p) (*PIN2DDR(p) |= (1 << PIN2BIT(p)))
#define PIN_INPUT(p)  (*PIN2DDR(p) &= ~(1 << PIN2BIT(p)))
#define PIN_PULLUP(p) (*PIN2PORT(p) |= (1 << PIN2BIT(p)))

static void uart_init(void) {
    unsigned int ubrr = F_CPU / 16 / SERIAL_BAUD_RATE - 1;
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

static void uart_putchar(char c) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = c;
}

static void uart_print(const char* s) {
    while (*s) uart_putchar(*s++);
}

static void uart_println(const char* s) {
    uart_print(s);
    uart_putchar('\r');
    uart_putchar('\n');
}

static void uart_print_ulong(unsigned long val) {
    char buf[11];
    char* p = buf + 10;
    *p = '\0';
    if (val == 0) *--p = '0';
    else while (val > 0) { *--p = '0' + (val % 10); val /= 10; }
    uart_print(p);
}

static void uart_print_int(int val) {
    if (val < 0) { uart_putchar('-'); val = -val; }
    uart_print_ulong((unsigned long)val);
}

static void i2c_init(void) {
    TWBR = (unsigned char)((F_CPU / 100000UL - 16) / 2);
    TWSR = 0;
}

static unsigned char i2c_start(unsigned char addr) {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    TWDR = addr;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    return (TWSR & 0xF8);
}

static void i2c_stop(void) {
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

static unsigned char i2c_write(unsigned char data) {
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    return (TWSR & 0xF8);
}

// PCF8574 -> HD44780: P7=D7, P6=D6, P5=D5, P4=D4, P3=BL, P2=E, P1=RW(GND), P0=RS
#define LCD_BL 0x08
#define LCD_E  0x04
#define LCD_RS 0x01

static unsigned char lcd_addr;
static unsigned char lcd_bl_state = LCD_BL;

static void lcd_pcf_write(unsigned char data) {
    i2c_start(lcd_addr);
    i2c_write(data);
    i2c_stop();
}

static void lcd_nibble(unsigned char data, unsigned char rs) {
    unsigned char byte = (data & 0xF0) | rs | lcd_bl_state;
    lcd_pcf_write(byte | LCD_E);
    _delay_us(2);
    lcd_pcf_write(byte & ~LCD_E);
    _delay_us(2);
}

static void lcd_byte(unsigned char data, unsigned char rs) {
    lcd_nibble(data, rs);
    lcd_nibble(data << 4, rs);
}

static void lcd_command(unsigned char cmd) {
    lcd_byte(cmd, 0);
    if (cmd == 0x01 || cmd == 0x02) _delay_ms(2);
    else _delay_us(50);
}

static void lcd_data(unsigned char data) {
    lcd_byte(data, LCD_RS);
    _delay_us(50);
}

static void lcd_init(void) {
    lcd_addr = LCD_ADDRESS << 1;
    _delay_ms(50);
    lcd_nibble(0x30, 0); _delay_ms(5);
    lcd_nibble(0x30, 0); _delay_us(150);
    lcd_nibble(0x30, 0); _delay_us(150);
    lcd_nibble(0x20, 0); _delay_us(150);
    lcd_command(0x28);
    lcd_command(0x08);
    lcd_command(0x01);
    lcd_command(0x06);
    lcd_command(0x0C);
}

static void lcd_clear(void) {
    lcd_command(0x01);
}

static void lcd_set_cursor(unsigned char col, unsigned char row) {
    unsigned char addr[] = {0x00, 0x40};
    lcd_command(0x80 | (addr[row] + col));
}

static void lcd_print(const char* s) {
    while (*s) lcd_data((unsigned char)*s++);
}

static void lcd_backlight(unsigned char on) {
    lcd_bl_state = on ? LCD_BL : 0;
    lcd_pcf_write(lcd_bl_state);
}

static volatile unsigned long timer0_millis = 0;

ISR(TIMER0_COMPA_vect) {
    timer0_millis++;
}

static void millis_init(void) {
    TCCR0A = (1 << WGM01);
    TCCR0B = (1 << CS01) | (1 << CS00);
    OCR0A = 249;
    TIMSK0 = (1 << OCIE0A);
}

static unsigned long millis(void) {
    unsigned long m;
    cli();
    m = timer0_millis;
    sei();
    return m;
}

static void delay(unsigned long ms) {
    unsigned long start = millis();
    while (millis() - start < ms);
}

static void pwm_init(void) {
    TCCR1A = (1 << WGM10) | (1 << COM1A1) | (1 << COM1B1);
    TCCR1B = (1 << CS10);
    TCCR2A = (1 << WGM21) | (1 << WGM20) | (1 << COM2A1);
    TCCR2B = (1 << CS20);
}

static void pwm_write(unsigned char pin, unsigned char val) {
    switch (pin) {
        case 9:  OCR1AL = val; break;
        case 10: OCR1BL = val; break;
        case 11: OCR2A  = val; break;
    }
}

static void set_rgb(unsigned char r, unsigned char g, unsigned char b) {
    pwm_write(RED_PIN, r);
    pwm_write(GREEN_PIN, g);
    pwm_write(BLUE_PIN, b);
}

static void play_tone(unsigned int freq, unsigned int duration_ms, unsigned char pulses) {
    if (freq == 0) return;
    unsigned int period_us = 1000000UL / freq;
    unsigned int half_us = period_us / 2;
    unsigned int pulse_ms = duration_ms / pulses;
    unsigned long cycles = (unsigned long)pulse_ms * 1000UL / period_us;
    for (unsigned char p = 0; p < pulses; p++) {
        for (unsigned long i = 0; i < cycles; i++) {
            PIN_WRITE(BUZZER_PIN, 1);
            _delay_loop_2((unsigned int)((unsigned long)half_us * 4 / 5));
            PIN_WRITE(BUZZER_PIN, 0);
            _delay_loop_2((unsigned int)((unsigned long)half_us * 4 / 5));
        }
        if (p < pulses - 1) {
            unsigned int pause = 50;
            while (pause--) _delay_loop_2(4000);
        }
    }
}

static void eeprom_save_teams(void) {
    eeprom_write_byte((uint8_t*)EEPROM_START_ADDRESS, EEPROM_MAGIC);
    for (int i = 0; i < NUM_TEAMS; i++) {
        int addr = EEPROM_START_ADDRESS + 1 + (i * sizeof(TeamData));
        eeprom_write_block((const void*)&teams[i], (void*)(uintptr_t)addr, sizeof(TeamData));
    }
    uart_println("Team data saved to EEPROM");
}

static void eeprom_load_teams(void) {
    if (eeprom_read_byte((const uint8_t*)EEPROM_START_ADDRESS) == EEPROM_MAGIC) {
        for (int i = 0; i < NUM_TEAMS; i++) {
            int addr = EEPROM_START_ADDRESS + 1 + (i * sizeof(TeamData));
            eeprom_read_block((void*)&teams[i], (const void*)(uintptr_t)addr, sizeof(TeamData));
        }
        uart_println("Team data loaded from EEPROM");
    } else {
        initializeTeams();
        eeprom_save_teams();
    }
}
#endif

static unsigned long rng_state = 1;

static void rng_seed(unsigned long seed) {
    rng_state = seed ? seed : 1;
}

static long rng_range(long min, long max) {
    rng_state = rng_state * 1103515245UL + 12345UL;
    return min + (long)(rng_state % (unsigned long)(max - min));
}

TeamData teams[NUM_TEAMS];
int currentTeamIndex = 0;
unsigned long reactionTime;
bool gameRunning = false;
int gameMode = 0;

const GameModeSettings gameModes[NUM_GAME_MODES] = {
    {2000, 6000, 1200, "Easy"},
    {1000, 5000, 1000, "Normal"},
    {500,  3000, 800,  "Hard"}
};

void initializeTeams(void) {
    const char* defaultNames[NUM_TEAMS] = {"Alpha", "Beta", "Gamma", "Delta", "Echo", "Foxtrot"};
    for (int i = 0; i < NUM_TEAMS; i++) {
        strcpy(teams[i].name, defaultNames[i]);
        teams[i].bestTime = 9999;
        teams[i].gamesPlayed = 0;
        teams[i].totalTime = 0;
        teams[i].fastReactions = 0;
    }
}

#ifdef __AVR__
static void startup_animation(void) {
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("REACTION TIMER");
    lcd_set_cursor(0, 1);
    lcd_print("Enhanced v2.0");
    for (int i = 0; i <= 255; i += 15) { set_rgb(i, 0, 255 - i); _delay_loop_2(20000); }
    for (int i = 0; i <= 255; i += 15) { set_rgb(255 - i, i, 0); _delay_loop_2(20000); }
    for (int i = 0; i <= 255; i += 15) { set_rgb(0, 255 - i, i); _delay_loop_2(20000); }
    set_rgb(0, 0, 0);
    play_tone(TONE_STARTUP, DURATION_MEDIUM, 3);
    delay(1000);
}

static void show_main_menu(void) {
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("Team: ");
    lcd_print(teams[currentTeamIndex].name);
    lcd_set_cursor(0, 1);
    lcd_print("Press=Play Hold=Stats");
    set_rgb(COLOR_MENU);
    uart_print("Main Menu - Current Team: ");
    uart_println(teams[currentTeamIndex].name);
}

static void show_game_mode_menu(void) {
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("Mode: ");
    lcd_print(gameModes[gameMode].name);
    lcd_set_cursor(0, 1);
    lcd_print("Press to change");
    switch (gameMode) {
        case 0: set_rgb(COLOR_EASY_MODE);   break;
        case 1: set_rgb(COLOR_NORMAL_MODE);  break;
        case 2: set_rgb(COLOR_HARD_MODE);    break;
    }
    uart_print("Game Mode: ");
    uart_println(gameModes[gameMode].name);
}

static void show_team_stats(void) {
    static int currentStatTeam = 0;
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print(teams[currentStatTeam].name);
    lcd_print(" Stats");
    lcd_set_cursor(0, 1);
    if (teams[currentStatTeam].gamesPlayed > 0) {
        lcd_print("Best:");
        {   char buf[8];
            unsigned long t = teams[currentStatTeam].bestTime;
            char* p = buf + 7;
            *p = '\0';
            do { *--p = '0' + (t % 10); t /= 10; } while (t);
            lcd_print(p); }
        lcd_print("ms");
    } else {
        lcd_print("No games played");
    }
    set_rgb(COLOR_STATS);
    uart_print("Stats for ");
    uart_print(teams[currentStatTeam].name);
    uart_print(": Best=");
    uart_print_ulong(teams[currentStatTeam].bestTime);
    uart_print("ms, Games=");
    uart_print_int(teams[currentStatTeam].gamesPlayed);
    if (teams[currentStatTeam].gamesPlayed > 0) {
        uart_print(", Avg=");
        uart_print_ulong(teams[currentStatTeam].totalTime / teams[currentStatTeam].gamesPlayed);
        uart_print("ms, Fast=");
        uart_print_int(teams[currentStatTeam].fastReactions);
    }
    uart_print("\r\n");
    currentStatTeam = (currentStatTeam + 1) % NUM_TEAMS;
    delay(3000);
}

static void show_countdown(void) {
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("Get Ready...");
    for (int i = 3; i > 0; i--) {
        lcd_set_cursor(7, 1);
        lcd_print(" ");
        lcd_set_cursor(7, 1);
        {   char c[2] = {'0' + i, '\0'};
            lcd_print(c); }
        uart_print_int(i);
        uart_println("...");
        switch (i) {
            case 3: set_rgb(COLOR_COUNTDOWN_3); break;
            case 2: set_rgb(COLOR_COUNTDOWN_2); break;
            case 1: set_rgb(COLOR_COUNTDOWN_1); break;
        }
        play_tone(TONE_COUNTDOWN_BASE + (unsigned int)(i * 200), DURATION_MEDIUM, 1);
        delay(gameModes[gameMode].countdownSpeed);
        if (PIN_READ(BUTTON_PIN)) {
            lcd_clear();
            lcd_set_cursor(0, 0);
            lcd_print("Too Soon!");
            lcd_set_cursor(0, 1);
            lcd_print("Wait for GO!");
            uart_print("Team ");
            uart_print(teams[currentTeamIndex].name);
            uart_println(": Too Soon during countdown!");
            set_rgb(COLOR_ERROR);
            play_tone(TONE_ERROR, DURATION_ERROR, 3);
            delay(3000);
            gameRunning = false;
            nextTeam();
            return;
        }
    }
}

static void start_game(void) {
    lcd_clear();
    uart_print("Game Started! Team ");
    uart_print(teams[currentTeamIndex].name);
    uart_print(" - Mode: ");
    uart_println(gameModes[gameMode].name);
    lcd_set_cursor(0, 0);
    lcd_print("Team: ");
    lcd_print(teams[currentTeamIndex].name);
    lcd_set_cursor(0, 1);
    lcd_print("Mode: ");
    lcd_print(gameModes[gameMode].name);
    delay(2000);
    show_countdown();
    if (!gameRunning) return;
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("Get Ready...");
    uart_println("Get Ready...");
    set_rgb(COLOR_READY);
    unsigned long delayStart = millis();
    unsigned long randomDelay = (unsigned long)rng_range(gameModes[gameMode].minDelay, gameModes[gameMode].maxDelay);
    while (millis() - delayStart < randomDelay) {
        if (PIN_READ(BUTTON_PIN)) {
            lcd_clear();
            lcd_set_cursor(0, 0);
            lcd_print("Too Soon!");
            lcd_set_cursor(0, 1);
            lcd_print("Try again...");
            uart_print("Team ");
            uart_print(teams[currentTeamIndex].name);
            uart_println(": Too Soon!");
            set_rgb(COLOR_ERROR);
            play_tone(TONE_ERROR, DURATION_ERROR, 3);
            delay(3000);
            gameRunning = false;
            nextTeam();
            return;
        }
        delay(10);
    }
    set_rgb(COLOR_GO);
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("*** GO! ***");
    uart_println("GO!");
    play_tone(TONE_GO, 150, 1);
    unsigned long startTime = millis();
    while (PIN_READ(BUTTON_PIN) == 0);
    reactionTime = millis() - startTime;
    processReactionTime();
}

void processReactionTime(void) {
    teams[currentTeamIndex].gamesPlayed++;
    teams[currentTeamIndex].totalTime += reactionTime;
    if (reactionTime < teams[currentTeamIndex].bestTime) {
        teams[currentTeamIndex].bestTime = reactionTime;
    }
    if (reactionTime < 200) {
        teams[currentTeamIndex].fastReactions++;
    }
    lcd_clear();
    lcd_set_cursor(0, 0);
    if (reactionTime < INCREDIBLE_THRESHOLD) {
        lcd_print("INCREDIBLE!");
        set_rgb(COLOR_INCREDIBLE);
        play_tone(TONE_INCREDIBLE, DURATION_LONG, 3);
    } else if (reactionTime < LIGHTNING_THRESHOLD) {
        lcd_print("Lightning Fast!");
        set_rgb(COLOR_LIGHTNING);
        play_tone(TONE_LIGHTNING, 300, 2);
    } else if (reactionTime < EXCELLENT_THRESHOLD) {
        lcd_print("Excellent!");
        set_rgb(COLOR_EXCELLENT);
        play_tone(TONE_EXCELLENT, 250, 2);
    } else if (reactionTime < GOOD_THRESHOLD) {
        lcd_print("Good Job!");
        set_rgb(COLOR_GOOD);
        play_tone(TONE_GOOD, DURATION_MEDIUM, 1);
    } else if (reactionTime < OKAY_THRESHOLD) {
        lcd_print("Not bad!");
        set_rgb(COLOR_OKAY);
        play_tone(TONE_OKAY, 300, 1);
    } else {
        lcd_print("Too slow!");
        set_rgb(COLOR_SLOW);
        play_tone(TONE_SLOW, 500, 1);
    }
    lcd_set_cursor(0, 1);
    lcd_print(teams[currentTeamIndex].name);
    lcd_print(": ");
    {   char buf[8];
        unsigned long t = reactionTime;
        char* p = buf + 7;
        *p = '\0';
        do { *--p = '0' + (t % 10); t /= 10; } while (t);
        lcd_print(p); }
    lcd_print("ms");
    uart_print("Team ");
    uart_print(teams[currentTeamIndex].name);
    uart_print(": ");
    uart_print_ulong(reactionTime);
    uart_print("ms (Best: ");
    uart_print_ulong(teams[currentTeamIndex].bestTime);
    uart_print("ms, Avg: ");
    if (teams[currentTeamIndex].gamesPlayed > 0) {
        uart_print_ulong(teams[currentTeamIndex].totalTime / teams[currentTeamIndex].gamesPlayed);
    }
    uart_println("ms)");
    delay(RESULT_DISPLAY_MS);
    if (reactionTime == teams[currentTeamIndex].bestTime && teams[currentTeamIndex].gamesPlayed > 1) {
        lcd_clear();
        lcd_set_cursor(0, 0);
        lcd_print("NEW BEST TIME!");
        lcd_set_cursor(0, 1);
        lcd_print("Congratulations!");
        for (int i = 0; i < 5; i++) {
            set_rgb(COLOR_CELEBRATION_1);
            delay(200);
            set_rgb(COLOR_CELEBRATION_2);
            delay(200);
        }
        play_tone(TONE_CELEBRATION, DURATION_SHORT, 5);
        delay(CELEBRATION_MS);
    }
    set_rgb(COLOR_OFF);
    gameRunning = false;
    eeprom_save_teams();
    nextTeam();
}

void nextTeam(void) {
    currentTeamIndex = (currentTeamIndex + 1) % NUM_TEAMS;
    delay(1000);
    show_main_menu();
}

int main(void) {
    PIN_OUTPUT(RED_PIN);
    PIN_OUTPUT(GREEN_PIN);
    PIN_OUTPUT(BLUE_PIN);
    PIN_OUTPUT(BUZZER_PIN);
    PIN_INPUT(BUTTON_PIN);
    PIN_PULLUP(BUTTON_PIN);
    set_rgb(0, 0, 0);
    PIN_WRITE(BUZZER_PIN, 0);
    uart_init();
    i2c_init();
    pwm_init();
    millis_init();
    lcd_init();
    lcd_backlight(1);
    rng_seed(millis());
    eeprom_load_teams();
    startup_animation();
    show_main_menu();
    sei();

    int menuState = 0;
    unsigned long lastButtonPress = 0;
    unsigned long buttonPressTime = 0;
    unsigned long lastActivity = millis();

    while (1) {
        unsigned char btn = PIN_READ(BUTTON_PIN);
        unsigned long now = millis();
        if (btn && (now - lastButtonPress > BUTTON_DEBOUNCE_MS)) {
            lastButtonPress = now;
            buttonPressTime = now;
            switch (menuState) {
                case 0:
                    menuState = 1;
                    show_game_mode_menu();
                    break;
                case 1:
                    gameMode = (gameMode + 1) % 3;
                    show_game_mode_menu();
                    delay(1000);
                    menuState = 3;
                    gameRunning = true;
                    start_game();
                    break;
                case 2:
                    menuState = 0;
                    show_main_menu();
                    break;
                case 3:
                    if (!gameRunning) {
                        gameRunning = true;
                        start_game();
                    }
                    break;
            }
        }
        if (menuState == 0 && btn && buttonPressTime > 0 &&
            (now - buttonPressTime > LONG_PRESS_MS)) {
            buttonPressTime = 0;
            menuState = 2;
            show_team_stats();
        }
        if (btn || gameRunning) {
            lastActivity = now;
        }
        if (menuState != 0 && !gameRunning && (now - lastActivity > AUTO_TIMEOUT_MS)) {
            menuState = 0;
            show_main_menu();
        }
    }
}
#else
void processReactionTime(void) {}
void nextTeam(void) {}
#endif
