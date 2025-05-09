#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_ATTEMPTS 1000
#define FILE_NAME_LEADERBOARD "leaderboard.txt"
#define FILE_NAME_HISTORY "history.txt"

#define COLOR_PINK "\033[38;2;255;182;193m"
#define COLOR_BLUE "\033[38;2;137;207;240m"
#define COLOR_YELLOW "\033[38;2;255;255;0m"
#define COLOR_RESET "\033[0m"

typedef struct {
    char username[50];
    float score;
} Attempt;

typedef struct {
    Attempt attempts[MAX_ATTEMPTS];
    int count;
} Leaderboard;

Leaderboard leaderboard = { .count = 0 };

void clearScreen() {
    printf("\033[H\033[J");
}

void showMainMenu();
void startGame();
void viewLeaderboard();
void viewHistory();
void askQuestions(Attempt *currentAttempt);
void saveAttempt(Attempt *attempt);
void saveAttemptToHistory(Attempt *attempt);
void loadAttempts();
void sortLeaderboard();
void saveLeaderboardToFile();

int main() {
    srand(time(NULL));
    loadAttempts();
    int choice;
    while (1) {
        showMainMenu();
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                startGame();
                break;
            case 2:
                viewLeaderboard();
                break;
            case 3:
                viewHistory();
                break;
            case 4:
                printf("Exiting application...\n");
                saveLeaderboardToFile();
                exit(0);
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }
    return 0;
}

void showMainMenu() {
    clearScreen();
    printf(COLOR_PINK "==============================\n");
    printf("        MAIN MENU             \n");
    printf("==============================\n" COLOR_RESET);
    printf("1. Start Game\n");
    printf("2. View Leaderboard\n");
    printf("3. View User History\n");
    printf("4. Exit\n");
    printf(COLOR_PINK "==============================\n" COLOR_RESET);
}

void startGame() {
    clearScreen();
    Attempt currentAttempt;
    printf("Enter your username: ");
    scanf("%49s", currentAttempt.username);
    getchar();
    currentAttempt.score = 0.0f;
    clearScreen();
    printf(COLOR_PINK "======== START GAME ========\n" COLOR_RESET);
    askQuestions(&currentAttempt);
    saveAttempt(&currentAttempt);
    printf("Press Enter to return to the main menu...");
    getchar();
}

void viewLeaderboard() {
    clearScreen();
    printf(COLOR_PINK "====== LEADERBOARD ======\n" COLOR_RESET);
    sortLeaderboard();
    int shown = 0;
    char shownUsers[MAX_ATTEMPTS][50];

    for (int i = 0; i < leaderboard.count; i++) {
        int alreadyShown = 0;
        for (int j = 0; j < shown; j++) {
            if (strcmp(shownUsers[j], leaderboard.attempts[i].username) == 0) {
                alreadyShown = 1;
                break;
            }
        }
        if (!alreadyShown) {
            printf("%d. %s - %.1f points\n", shown + 1, leaderboard.attempts[i].username, leaderboard.attempts[i].score);
            strcpy(shownUsers[shown], leaderboard.attempts[i].username);
            shown++;
        }
        if (shown >= 10) break;
    }
    printf("Press Enter to return to the main menu...");
    getchar();
}

void viewHistory() {
    clearScreen();
    printf(COLOR_PINK "======= GAME HISTORY =======\n" COLOR_RESET);
    printf("Enter username to view history: ");
    char username[50];
    scanf("%49s", username);
    getchar();

    FILE *fp = fopen(FILE_NAME_HISTORY, "r");
    if (!fp) {
        printf("No history available.\n");
        printf("Press Enter to return to the main menu...");
        getchar();
        return;
    }

    int found = 0;
    char fileUsername[50];
    float score;
    while (fscanf(fp, "%49s %f", fileUsername, &score) == 2) {
        if (strcmp(fileUsername, username) == 0) {
            printf("Attempt %d: %.1f points\n", ++found, score);
        }
    }
    fclose(fp);

    if (!found) {
        printf("No history found for user: %s\n", username);
    }

    printf("Press Enter to return to the main menu...");
    getchar();
}

void askQuestions(Attempt *currentAttempt) {
    int answer;
    char use5050;
    float earnedPoints;

    struct {
        char question[256];
        char options[4][100];
        int correctAnswer;
    } questions[] = {
        {"What is the name of the empire that ruled the Middle East from 883 to 627 BC?", {"The Babylonian Empire", "The Merovingian Empire", "The Persian Empire", "The Assyrian Empire"}, 4},
        {"Whose adopted son was Emperor Augustus?", {"Diogenes", "Julius Caesar", "Nero", "Mark Antony"}, 2},
        {"Which major city put the first urban tram line into service in 1832?", {"New York", "London", "Berlin", "Vienna"}, 1},
        {"In what year did the reunification of Germany take place?", {"1989", "1990", "1991", "1992"}, 2},
        {"Which was the last country to join the European Union in 2013?", {"Croatia", "Romania", "Bulgaria", "Estonia"}, 1},
    };

    int totalQuestions = sizeof(questions) / sizeof(questions[0]);

    for (int i = 0; i < totalQuestions; i++) {
        printf(COLOR_BLUE "%d. " COLOR_RESET "%s\n", i + 1, questions[i].question);
        for (int j = 0; j < 4; j++) {
            printf("   " COLOR_YELLOW "%d. " COLOR_RESET "%s\n", j + 1, questions[i].options[j]);
        }

        // Ask if user wants 50-50, with validation
        do {
            printf("Do you want to use 50-50? (y/n): ");
            scanf(" %c", &use5050);
            getchar();
            if (use5050 != 'y' && use5050 != 'Y' && use5050 != 'n' && use5050 != 'N') {
                printf("Invalid input. Please enter 'y' or 'n'.\n");
            }
        } while (use5050 != 'y' && use5050 != 'Y' && use5050 != 'n' && use5050 != 'N');

        if (use5050 == 'y' || use5050 == 'Y') {
            int correct = questions[i].correctAnswer;
            int second = correct == 1 ? 2 : 1; // Just pick any one wrong answer for demo
            printf("Remaining options: %d. %s and %d. %s\n",
                correct, questions[i].options[correct - 1],
                second, questions[i].options[second - 1]);
        }

        // Validate user answer
        do {
            printf("Your answer (1-4): ");
            if (scanf("%d", &answer) != 1) {
                while (getchar() != '\n'); // Flush input
                answer = 0; // Force re-entry
            }
            if (answer < 1 || answer > 4) {
                printf("Invalid input. Please choose between 1, 2, 3, or 4.\n");
            }
        } while (answer < 1 || answer > 4);
        getchar(); // Clear newline

        // Check correctness
        if (answer == questions[i].correctAnswer) {
            earnedPoints = (use5050 == 'y' || use5050 == 'Y') ? 0.5f : 1.0f;
            printf("Correct! You earned %.1f points.\n", earnedPoints);
            currentAttempt->score += earnedPoints;
        } else {
            printf("Incorrect! Correct answer was %d. %s\n",
                questions[i].correctAnswer,
                questions[i].options[questions[i].correctAnswer - 1]);
            printf("Game Over! Your final score is: %.1f\n", currentAttempt->score);
            break;
        }
    }
}


void saveAttempt(Attempt *attempt) {
    saveAttemptToHistory(attempt);
    for (int i = 0; i < leaderboard.count; i++) {
        if (strcmp(leaderboard.attempts[i].username, attempt->username) == 0) {
            if (attempt->score > leaderboard.attempts[i].score) {
                leaderboard.attempts[i].score = attempt->score;
            }
            return;
        }
    }
    if (leaderboard.count < MAX_ATTEMPTS) {
        leaderboard.attempts[leaderboard.count++] = *attempt;
    }
}

void saveAttemptToHistory(Attempt *attempt) {
    FILE *fp = fopen(FILE_NAME_HISTORY, "a");
    if (!fp) {
        perror("Failed to open history file");
        return;
    }
    fprintf(fp, "%s %.1f\n", attempt->username, attempt->score);
    fclose(fp);
}

void sortLeaderboard() {
    for (int i = 0; i < leaderboard.count - 1; i++) {
        for (int j = i + 1; j < leaderboard.count; j++) {
            if (leaderboard.attempts[i].score < leaderboard.attempts[j].score) {
                Attempt temp = leaderboard.attempts[i];
                leaderboard.attempts[i] = leaderboard.attempts[j];
                leaderboard.attempts[j] = temp;
            }
        }
    }
}

void saveLeaderboardToFile() {
    FILE *fp = fopen(FILE_NAME_LEADERBOARD, "w");
    if (!fp) {
        perror("Failed to open leaderboard file");
        return;
    }
    for (int i = 0; i < leaderboard.count; i++) {
        fprintf(fp, "%s %.1f\n", leaderboard.attempts[i].username, leaderboard.attempts[i].score);
    }
    fclose(fp);
}

void loadAttempts() {
    FILE *fp = fopen(FILE_NAME_LEADERBOARD, "r");
    if (!fp) return;
    while (fscanf(fp, "%49s %f", leaderboard.attempts[leaderboard.count].username, &leaderboard.attempts[leaderboard.count].score) == 2) {
        leaderboard.count++;
        if (leaderboard.count >= MAX_ATTEMPTS) break;
    }
    fclose(fp);
}
