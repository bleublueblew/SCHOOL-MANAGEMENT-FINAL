#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>

/* ---------- GLOBALS ---------- */
char lastN[50], firstN[100], middleN[50];
char birthD[20], sex[10];
char house[80], barangay[50], city[50], province[50];

char roleChoice[50];
char programChoice[100];

void loginPage();
void forgotPasswordPage();
void enrollMent();
void goToDashboard();
void enrollMent();
void profileCard();   // declare that this function exists
void inbox_Page();


/* Account info globals (used across functions) */
char email[300];
char contactN[30];
char passW[200];
char passConfirm[200];
char profMajor[200];
char addressFull[512];
char Fullname[256];
char scheduleType[100];
char yearLevel[5];
char facultyType[20];

char currentRole[20];


/* -------------------------------------------------------------------
   Console helpers
   ------------------------------------------------------------------- */
void gotoxy(int x, int y) {
    COORD c = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

void showCursor(int visible) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO ci;
    GetConsoleCursorInfo(hConsole, &ci);
    ci.bVisible = visible ? TRUE : FALSE;
    SetConsoleCursorInfo(hConsole, &ci);
}

void clearBox(int x, int y, int width) {
    gotoxy(x, y);
    for (int i = 0; i < width; i++) putchar(' ');
    gotoxy(x, y);
}

/* -------------------------------------------------------------------
   Simple ASCII helpers
   ------------------------------------------------------------------- */
void toLowerStr(char *s) {
    for (int i = 0; s[i]; i++)
        if (s[i] >= 'A' && s[i] <= 'Z') s[i] += 32;
}

void capitalize(char *s) {
    if (!s || !s[0]) return;
    if (s[0] >= 'a' && s[0] <= 'z') s[0] -= 32;
    for (int i = 1; s[i]; i++)
        if (s[i] >= 'A' && s[i] <= 'Z') s[i] += 32;
}

/* -------------------------------------------------------------------
   Loading screen
   ------------------------------------------------------------------- */
void loading_screen() {
    const int width = 40;
    char block = 178;
    srand((unsigned)time(NULL));

    system("cls");
    gotoxy(10, 6); printf("Loading...\n\n");

    gotoxy(10, 9);
    printf("[");
    for (int i = 0; i < width; i++) putchar(' ');
    printf("] 0%%");

    for (int i = 0; i <= width; i++) {
        gotoxy(11, 9);
        for (int j = 0; j < i; j++) putchar(block);
        for (int j = i; j < width; j++) putchar(' ');

        gotoxy(11 + width + 2, 9);
        printf("%3.0f%%", (i / (float)width) * 100);

        Sleep(25 + (rand() % 20));
    }

    gotoxy(10, 11);
    printf("Loading complete!");
    Sleep(500);
}

/* -------------------------------------------------------------------
   Basic input box (single-line, ENTER to finish)
   ------------------------------------------------------------------- */
void inputBox(int x, int y, char *buf, int len) {
    int i = 0;
    int ch;
    memset(buf, 0, len);

    gotoxy(x, y);
    showCursor(1);

    while (1) {
        ch = getch();
        if (ch == 13) break;        // ENTER
        if (ch == 8) {              // BACKSPACE
            if (i > 0) {
                i--;
                buf[i] = 0;
                gotoxy(x + i, y);
                putchar(' ');
                gotoxy(x + i, y);
            }
        } else if (ch == 0 || ch == 224) {
            // extended key; consume and ignore
            getch();
        } else if (ch >= 32 && ch <= 126 && i < len - 1) {
            buf[i++] = (char)ch;
            putchar(ch);
        }
    }

    buf[i] = '\0';
    showCursor(0);
}

/* -------------------------------------------------------------------
   Password input with masking
   ------------------------------------------------------------------- */
void inputPassword(int x, int y, char *buffer, int maxLen) {
    int index = 0;
    int ch;

    memset(buffer, 0, maxLen);
    gotoxy(x, y);
    showCursor(1);

    while (1) {
        ch = getch();

        /* ENTER = finish input */
        if (ch == 13) {
            buffer[index] = '\0';
            break;
        }

        /* BACKSPACE */
        else if (ch == 8) {
            if (index > 0) {
                index--;
                buffer[index] = '\0';

                gotoxy(x + index, y);
                putchar(' ');
                gotoxy(x + index, y);
            }
        }

        /* ignore extended keys */
        else if (ch == 0 || ch == 224) {
            getch();
        }

        /* NORMAL CHARACTER */
        else if (index < maxLen - 1 && ch >= 32 && ch <= 126) {
            buffer[index++] = (char)ch;
            putchar('*');   // mask character
        }
        /* allow quick C to jump to create account while typing */
        else if (ch == 'c' || ch == 'C') {
            /* put cursor back (we'll handle createAccount in caller) */
            buffer[index] = '\0';
            break;
        }
    }

    showCursor(0);
}



typedef struct {
    char sectionName[50];
    char program[20];
    int yearLevel;
} Section;

// Sort sections by year then name
int compareSectionsByYear(const void *a, const void *b) {
    Section *secA = (Section *)a;
    Section *secB = (Section *)b;
    if (secA->yearLevel != secB->yearLevel)
        return secA->yearLevel - secB->yearLevel;
    return strcmp(secA->sectionName, secB->sectionName);
}




/* -------------------------------------------------------------------
   Validation helpers
   ------------------------------------------------------------------- */
int validateName(const char *s) { return s && strlen(s) >= 2; }

int validateDate(int m, int d, int y) {
    if (y < 1900 || y > 2100) return 0;
    if (m < 1 || m > 12) return 0;
    if (d < 1 || d > 31) return 0;
    if (m == 2 && d > 29) return 0;
    if ((m == 4 || m == 6 || m == 9 || m == 11) && d > 30) return 0;
    return 1;
}

int validateSex(const char *s) {
    return (strcmp(s, "Male") == 0 || strcmp(s, "Female") == 0);
}


int countStudentsInSection(const char *section) {
    FILE *f = fopen("schedules.txt", "r");
    if (!f) return 0;

    char line[512];
    char storedEmail[300], storedName[256], storedProgram[128], storedSection[64], storedDate[64];
    int count = 0;

    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = 0;

        if (sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^\n]",
                   storedEmail, storedName, storedProgram, storedSection, storedDate) >= 4) {

            if (strcmp(storedSection, section) == 0)
                count++;
        }
    }

    fclose(f);
    return count;
}

/* Finds the next available section number. Each section has a capacity of 30. */
int getNextSectionNumber(int year, const char *shift) {
    char sectionName[32];
    int secNum = 1;

    while (1) {
        snprintf(sectionName, sizeof(sectionName), "%d%s%d", year, shift, secNum);
        if (countStudentsInSection(sectionName) < 30)
            return secNum;
        secNum++;
    }
}

/* Generate section name based on schedule type */
void generateSection(char *output, int year, const char *scheduleType, const char *shift) {
    if (strcmp(scheduleType, "Sunday Class") == 0) {
        char progShort[32];

        if (strstr(profMajor, "Computer"))
            strcpy(progShort, "BSCS");
        else if (strstr(profMajor, "Office"))
            strcpy(progShort, "BSOA");
        else if (strstr(profMajor, "TVT") || strstr(profMajor, "Teacher"))
            strcpy(progShort, "BTVTED");
        else
            strcpy(progShort, profMajor);

        snprintf(output, 64, "%s %d - Sunday Class", progShort, year);
        return;
    }

    int nextNum = getNextSectionNumber(year, shift);
    snprintf(output, 64, "%d%s%d", year, shift, nextNum);
}

/* Saves the schedule to schedules.txt, using EMAIL as the unique ID */
void saveScheduleToFile(const char *sectionName) {
    if(strcmp(roleChoice,"Student") != 0) return;  // only save for students

    FILE *file = fopen("schedules.txt", "a"); // append mode
    if(!file){
        printf("\n(System): Error opening schedules.txt!\n");
        printf("Press any key to go back...\n");   // <-- IMPORTANT
        getch();                                 // <-- prevents instant exit
        goToDashboard();                         // <-- safely return to menu
        return;
    }

    fprintf(file, "%s|%s|%s|%s|%s\n",
            email, Fullname, profMajor, scheduleType, sectionName);

    fclose(file);
}





// Load only name and role from users.txt
void loadUserInfo() {
    FILE *fu = fopen("users.txt", "r");
    if (!fu) return;

    char fields[20][300];
    char line[1024];

    while (fgets(line, sizeof(line), fu)) {
        int idx = 0;
        char *token = strtok(line, "|");

        while (token && idx < 20) {
            strcpy(fields[idx++], token);
            token = strtok(NULL, "|");
        }

        if (idx < 6) continue; // skip malformed lines

        char full[256];
        snprintf(full, sizeof(full), "%s, %s %s", fields[5], fields[3], fields[4]); 
        // last, first middle (matches Fullname format)

        if (strcmp(full, Fullname) == 0) {
            strcpy(roleChoice, fields[2]); // <-- field 2 is role
            break;
        }
    }

    fclose(fu);
}



// Save attendance with just name and role
void saveTodayAttendance(const char *date, const char *roleChoice,
                         const char *timeIn, const char *timeOut)
{
    FILE *fr = fopen("attendance_today.txt", "r");
    FILE *fw = fopen("attendance_tmp.txt", "w");

    char fileDate[20], fileRole[50], fileName[100], fileTI[10], fileTO[10];
    int found = 0;

    if (fw) {
        if (fr) {
            while (fscanf(fr, "%19[^|]|%49[^|]|%99[^|]|%9[^|]|%9[^\n]\n",
                          fileDate, fileRole, fileName, fileTI, fileTO) == 5)
            {
                if (strcmp(fileName, Fullname) == 0) {
                    fprintf(fw, "%s|%s|%s|%s|%s\n", date, roleChoice, Fullname, timeIn, timeOut);
                    found = 1;
                } else {
                    fprintf(fw, "%s|%s|%s|%s|%s\n", fileDate, fileRole, fileName, fileTI, fileTO);
                }
            }
            fclose(fr);
        }

        if (!found) {
            fprintf(fw, "%s|%s|%s|%s|%s\n", date, roleChoice, Fullname, timeIn, timeOut);
        }

        fclose(fw);
        remove("attendance_today.txt");
        rename("attendance_tmp.txt", "attendance_today.txt");
    }

    FILE *log = fopen("attendance_master.txt", "a");
    if (log) {
        fprintf(log, "%s|%s|%s|%s|%s\n", date, roleChoice, Fullname, timeIn, timeOut);
        fclose(log);
    }
}

// Attendance page simplified
void attendancePage(const char *roleChoice) {
    char todayDate[20], now[20];
    char timeIn[10] = "---", timeOut[10] = "---";
    int hasIn = 0, hasOut = 0;

    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(todayDate, sizeof(todayDate), "%Y-%m-%d", tm_info);

    // Check today's attendance
    FILE *fr = fopen("attendance_today.txt", "r");
    if (fr) {
        char fdate[20], frole[50], fname[100], fti[10], fto[10];
        while (fscanf(fr, "%19[^|]|%49[^|]|%99[^|]|%9[^|]|%9[^\n]\n",
                      fdate, frole, fname, fti, fto) == 5)
        {
            if (strcmp(fname, Fullname) == 0) {
                strcpy(timeIn, fti);
                strcpy(timeOut, fto);
                hasIn = strcmp(timeIn, "---") != 0;
                hasOut = strcmp(timeOut, "---") != 0;
                break;
            }
        }
        fclose(fr);
    }

    while (1) {
        system("cls");
        printf("                     +--------------------------------------------------------------+\n");
        printf("                     |                        P H I L T E C H                       |\n");
        printf("                     +--------------------------------------------------------------+\n");
        printf("                     +--------------------------------------------------------------+\n");
        printf("                     | EMPLOYEE : %-48s |\n", Fullname);
        printf("                     | POSITION : %-48s |\n", roleChoice);
        printf("                     +--------------------------------------------------------------+\n");
        printf("                     DATE TODAY : %s\n", todayDate);
        printf("                     TIME IN    : %s\n", timeIn);
        printf("                     TIME OUT   : %s\n", timeOut);
        printf("\n                    +-------------------+   +-------------------+   +------------------+\n");
          printf("                    |       [ Q ]       |   |       [ W ]       |   |       [ 9 ]      |\n");
          printf("                    |       Time In     |   |      Time Out     |   |       Back       |\n");
          printf("                    +-------------------+   +-------------------+   +------------------+");

        char key = getch();
        t = time(NULL);
        tm_info = localtime(&t);
        strftime(now, sizeof(now), "%H:%M", tm_info);

        if ((key == 'q' || key == 'Q') && !hasIn) {
            strcpy(timeIn, now);
            hasIn = 1;
            saveTodayAttendance(todayDate, roleChoice, timeIn, timeOut);
        } else if ((key == 'w' || key == 'W') && hasIn && !hasOut) {
            strcpy(timeOut, now);
            hasOut = 1;
            saveTodayAttendance(todayDate, roleChoice, timeIn, timeOut);
        } else if (key == '9') {
            saveTodayAttendance(todayDate, roleChoice, timeIn, timeOut);
            goToDashboard();
        }
    }
}



void universalHeader() {
    system("cls");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |                         PHILTECH PORTAL                     |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |  [9] Back                                                   |\n");
    printf("                             +-------------------------------------------------------------+\n\n\n\n");

}






/* =============================================================
   PHILTECH — 36 Unique Schedule Placeholder Templates 
   ============================================================= */

void printSchedulePlaceholder(const char *program, const char *sectionName, int year) {
    char ch;  // localized variable for getch

    /* ---------- SUNDAY CLASS (12 unique) ---------- */
    if (strstr(sectionName, "Sunday") || strstr(sectionName, "Sunday Class")) {
        if (strcmp(program, "BSCS") == 0) {
            if (year == 1) {
                universalHeader();
                printf("                    ________________________________________________________________________|\n");
				printf("                    |                                 WEEKLY SCHEDULE                       |\n");
				printf("                    |                                 COURSE: BSCS                          |\n");
				printf("                    |-----------------------------------------------------------------------|\n");
				printf("                    |-----------------------------------------------------------------------|\n");
				printf("                    | TIME        |                        SUNDAY                           |\n");
				printf("                    |-------------+--------+--------+-----------+---------+--------+--------|\n");
				printf("                    | 7:00-8:00   |                                                         |\n");
				printf("                    |-------------+--------+--------+-----------+---------+------+----------|\n");
				printf("                    | 8:00-8:45   |                    PLS   406-MAIN                       |\n");
				printf("                    |-------------+--------+--------+-----------+---------+------+----------|\n");
				printf("                    | 8:45-9:00   |                      BREAKTIME                          |\n");
				printf("                    |-------------+--------+--------+-----------+----------+------+---------|\n");
				printf("                    | 9:00-10:00  |                         CC112                           |\n");
				printf("                    |-------------+                                                         |\n");
				printf("                    | 10:00-10:45 |                       CL1-MAIN                          |\n");
				printf("                    |-------------+--------+--------+-----------+----------+------+---------|\n");
				printf("                    | 10:45-11:15 |                      LUNCHBREAK                         |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 11:15-12:00 |                        GE111                            |\n");
				printf("                    |-------------+                                                         |\n");
				printf("                    | 12:00-1:00  |                       403-MAIN                          |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 1:00-2:00   |                         CC111                           |\n");
				printf("                    |-------------+                                                         |\n");
				printf("                    | 2:00-2:45   |                       CL2-MAIN                          |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 2:45-3:00   |                      BREAKTIME                          |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 3:00-4:00   |                        GE112                            |\n");
				printf("                    |-------------+                                                         |\n");
				printf("                    | 4:00-4:45   |                       403-MAIN                          |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 4:45-5:00   |                       BREAKTIME                         |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 5:15-6:00   |                                                         |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 6:00-7:00   |                                                         |\n");
				printf("                    |-------------+--------+--------+-----------+---------+--------+--------|\n\n\n");
 
                ch = getch();
                if (ch == '9') { goToDashboard(); return; }
            }
            if (year == 2) {
                universalHeader();
                printf("                    ________________________________________________________________________|\n");
				printf("                    |                                 WEEKLY SCHEDULE                       |\n");
				printf("                    |                                 COURSE: BSCS                          |\n");
				printf("                    |-----------------------------------------------------------------------|\n");
				printf("                    |-----------------------------------------------------------------------|\n");
				printf("                    | TIME        |                        SUNDAY                           |\n");
				printf("                    |-------------+--------+--------+-----------+---------+--------+--------|\n");
				printf("                    | 7:00-8:00   |                        CC214                            |\n");
				printf("                    |-------------+--------+--------+-----------+---------+------+----------|\n");
				
				printf("                    | 8:00-8:45   |                       CL1-MAIN                          |\n");
				printf("                    |-------------+--------+--------+-----------+---------+------+----------|\n");
				printf("                    | 8:45-9:00   |                      BREAKTIME                          |\n");
				printf("                    |-------------+--------+--------+-----------+----------+------+---------|\n");
				printf("                    | 9:00-10:00  |                         GE113                           |\n");
				printf("                    |-------------+                                                         |\n");
				printf("                    | 10:00-10:45 |                       402-MAIN                          |\n");
				printf("                    |-------------+--------+--------+-----------+----------+------+---------|\n");
				printf("                    | 10:45-11:15 |                      LUNCHBREAK                         |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 11:15-12:00 |                        DS121                            |\n");
				printf("                    |-------------+                                                         |\n");
				printf("                    | 12:00-1:00  |                       CL2-MAIN                          |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 1:00-2:00   |                         GE117                           |\n");
				printf("                    |-------------+                                                         |\n");
				printf("                    | 2:00-2:45   |                       406-MAIN                          |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 2:45-3:00   |                      BREAKTIME                          |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 3:00-4:00   |                        GE116                            |\n");
				printf("                    |-------------+                                                         |\n");
				printf("                    | 4:00-4:45   |                       403-MAIN                          |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 4:45-5:00   |                       BREAKTIME                         |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 5:15-6:00   |                                                         |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 6:00-7:00   |                                                         |\n");
				printf("                    |-------------+--------+--------+-----------+---------+--------+--------|\n\n\n");

                ch = getch();
                if (ch == '9') { goToDashboard(); return; }
            }
            if (year == 3) {
                universalHeader();
                printf("                                    +----------------------------------+\n");
				printf("                                    |     BSCS 3 - Sunday Schedule     |\n");
				printf("                                    +----------------------------------+\n");
				printf("                                    |   Placeholder — Schedule TBD     |\n");
				printf("                                    +----------------------------------+\n\n");

                ch = getch();
                if (ch == '9') { goToDashboard(); return; }
            }
            if (year == 4) {
                universalHeader();
                printf("                                    +----------------------------------+\n");
				printf("                                    |     BSCS 4 - Sunday Schedule     |\n");
				printf("                                    +----------------------------------+\n");
				printf("                                    |   Placeholder — Schedule TBD     |\n");
				printf("                                    +----------------------------------+\n\n");

                ch = getch();
                if (ch == '9') { goToDashboard(); return; }
            }
        }

        if (strcmp(program, "BSOA") == 0) {
            if (year == 1) {
                universalHeader();
                printf("                ________________________________________________________________________|\n");
				printf("                |                                 WEEKLY SCHEDULE                       |\n");
				printf("                |                                 COURSE: BSOA                          |\n");
				printf("                |-----------------------------------------------------------------------|\n");
				printf("                |-----------------------------------------------------------------------|\n");
				printf("                | TIME        |                        SUNDAY                           |\n");
				printf("                |-------------+--------+--------+-----------+---------+--------+--------|\n");
				printf("                | 7:00-8:00   |                        OACC101                          |\n");
				printf("                |-------------+                                                         |\n");
				printf("                | 8:00-8:45   |                       411-MAIN                          |\n");
				printf("                |-------------+--------+--------+-----------+---------+------+----------|\n");
				printf("                | 8:45-9:00   |                      BREAKTIME                          |\n");
				printf("                |-------------+--------+--------+-----------+----------+------+---------|\n");
				printf("                | 9:00-10:00  |                         GEE1                            |\n");
				printf("                |-------------+                                                         |\n");
				printf("                | 10:00-10:45 |                       403-MAIN                          |\n");
				printf("                |-------------+--------+--------+-----------+----------+------+---------|\n");
				printf("                | 10:45-11:15 |                      LUNCHBREAK                         |\n");
				printf("                |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                | 11:15-12:00 |                        GE111                            |\n");
				printf("                |-------------+                                                         |\n");
				printf("                | 12:00-1:00  |                       403-MAIN                          |\n");
				printf("                |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                | 1:00-2:00   |                        OACC103                          |\n");
				printf("                |-------------+                                                         |\n");
				printf("                | 2:00-2:45   |                       411-MAIN                          |\n");
				printf("                |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                | 2:45-3:00   |                      BREAKTIME                          |\n");
				printf("                |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                | 3:00-4:00   |                        GE112                            |\n");
				printf("                |-------------+                                                         |\n");
				printf("                | 4:00-4:45   |                       403-MAIN                          |\n");
				printf("                |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                | 4:45-5:00   |                       BREAKTIME                         |\n");
				printf("                |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                | 5:15-6:00   |                                                         |\n");
				printf("                |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                | 6:00-7:00   |                                                         |\n");
				printf("                |-------------+--------+--------+-----------+---------+--------+--------|\n\n\n");

                ch = getch();
                if (ch == '9') { goToDashboard(); return; }
            }
            if (year == 2) {
                universalHeader();
	            printf("                    |_______________________________________________________________________|\n");
				printf("                    |                                 WEEKLY SCHEDULE                       |\n");
				printf("                    |                                 COURSE: BSOA                          |\n");
				printf("                    |-----------------------------------------------------------------------|\n");
				printf("                    |-----------------------------------------------------------------------|\n");
				printf("                    | TIME        |                        SUNDAY                           |\n");
				printf("                    |-------------+--------+--------+-----------+---------+--------+--------|\n");
				printf("                    | 7:00-8:00   |                                                         |\n");
				printf("                    |-------------+                                                         |\n");
				printf("                    | 8:00-8:45   |                                                         |\n");
				printf("                    |-------------+--------+--------+-----------+---------+------+----------|\n");
				printf("                    | 8:45-9:00   |                      BREAKTIME                          |\n");
				printf("                    |-------------+--------+--------+-----------+----------+------+---------|\n");
				printf("                    | 9:00-10:00  |                         GEE1                            |\n");
				printf("                    |-------------+                                                         |\n");
				printf("                    | 10:00-10:45 |                       403-MAIN                          |\n");
				printf("                    |-------------+--------+--------+-----------+----------+------+---------|\n");
				printf("                    | 10:45-11:15 |                      LUNCHBREAK                         |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 11:15-12:00 |                          VAL                            |\n");
				printf("                    |-------------+                                                         |\n");
				printf("                    | 12:00-1:00  |                       404-MAIN                          |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 1:00-2:00   |                        OACC207                          |\n");
				printf("                    |-------------+                                                         |\n");
				printf("                    | 2:00-2:45   |                       404-MAIN                          |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 2:45-3:00   |                      BREAKTIME                          |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 3:00-4:00   |                        GE117                            |\n");
				printf("                    |-------------+                                                         |\n");
				printf("                    | 4:00-4:45   |                       404-MAIN                          |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 4:45-5:00   |                       BREAKTIME                         |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 5:15-6:00   |                        GE116                            |\n");
				printf("                    |-------------+                                                         |\n");
				printf("                    | 6:00-7:00   |                        402-MAIN                         |\n");
				printf("                    |-------------+--------+--------+-----------+---------+--------+--------|\n\n\n");


                ch = getch();
                if (ch == '9') { goToDashboard(); return; }
            }
            if (year == 3) {
                universalHeader();
                printf("                                    +----------------------------------+\n");
				printf("                                    |     BSOA 3 - Sunday Schedule     |\n");
				printf("                                    +----------------------------------+\n");
				printf("                                    |   Placeholder — Schedule TBD     |\n");
				printf("                                    +----------------------------------+\n\n");

                ch = getch();
                if (ch == '9') { goToDashboard(); return; }
            }
            if (year == 4) {
                universalHeader();
                printf("                                    +----------------------------------+\n");
				printf("                                    |     BSOA 4 - Sunday Schedule     |\n");
				printf("                                    +----------------------------------+\n");
				printf("                                    |   Placeholder — Schedule TBD     |\n");
				printf("                                    +----------------------------------+\n\n");

                ch = getch();
                if (ch == '9') { goToDashboard(); return; }
            }
        }

        if (strcmp(program, "BTVTED") == 0) {
            if (year == 1) {
                universalHeader();
	            printf("                    ________________________________________________________________________|\n");
				printf("                    |                                 WEEKLY SCHEDULE                       |\n");
				printf("                    |                                 COURSE: BTVTED                        |\n");
				printf("                    |-----------------------------------------------------------------------|\n");
				printf("                    |-----------------------------------------------------------------------|\n");
				printf("                    | TIME        |                        SUNDAY                           |\n");
				printf("                    |-------------+--------+--------+-----------+---------+--------+--------|\n");
				printf("                    | 7:00-8:00   |                        TLE101                           |\n");
				printf("                    |-------------+                                                         |\n");
				printf("                    | 8:00-8:45   |                       CL2-MAIN                          |\n");
				printf("                    |-------------+--------+--------+-----------+---------+------+----------|\n");
				printf("                    | 8:45-9:00   |                      BREAKTIME                          |\n");
				printf("                    |-------------+--------+--------+-----------+----------+------+---------|\n");
				printf("                    | 9:00-10:00  |                         GE102                           |\n");
				printf("                    |-------------+                                                         |\n");
				printf("                    | 10:00-10:45 |                       402-MAIN                          |\n");
				printf("                    |-------------+--------+--------+-----------+----------+------+---------|\n");
				printf("                    | 10:45-11:15 |                      LUNCHBREAK                         |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 11:15-12:00 |                        FCC101                           |\n");
				printf("                    |-------------+                                                         |\n");
				printf("                    | 12:00-1:00  |                       402-MAIN                          |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 1:00-2:00   |                         GEE1                            |\n");
				printf("                    |-------------+                                                         |\n");
				printf("                    | 2:00-2:45   |                       402-MAIN                          |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 2:45-3:00   |                      BREAKTIME                          |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 3:00-4:00   |                        NSTP1                            |\n");
				printf("                    |-------------+                                                         |\n");
				printf("                    | 4:00-4:45   |                       402-MAIN                          |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 4:45-5:00   |                       BREAKTIME                         |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 5:15-6:00   |                        GE101                            |\n");
				printf("                    |-------------+                                                         |\n");
				printf("                    | 6:00-7:00   |                      403-MAIN                           |\n");
				printf("                    |-------------+--------+--------+-----------+---------+--------+--------|\n\n\n");

                ch = getch();
                if (ch == '9') { goToDashboard(); return; }
            }
            if (year == 2) {
                universalHeader();
                printf("                    ________________________________________________________________________|\n");
				printf("                    |                                 WEEKLY SCHEDULE                       |\n");
				printf("                    |                                 COURSE: BTVTED                        |\n");
				printf("                    |-----------------------------------------------------------------------|\n");
				printf("                    |-----------------------------------------------------------------------|\n");
				printf("                    | TIME        |                        SUNDAY                           |\n");
				printf("                    |-------------+--------+--------+-----------+---------+--------+--------|\n");
				printf("                    | 7:00-8:00   |                        PCK104                           |\n");
				printf("                    |-------------+                                                         |\n");
				printf("                    | 8:00-8:45   |                       405-MAIN                          |\n");
				printf("                    |-------------+--------+--------+-----------+---------+------+----------|\n");
				printf("                    | 8:45-9:00   |                      BREAKTIME                          |\n");
				printf("                    |-------------+--------+--------+-----------+----------+------+---------|\n");
				printf("                    | 9:00-10:00  |                         GE102                           |\n");
				printf("                    |-------------+                                                         |\n");
				printf("                    | 10:00-10:45 |                       402-MAIN                          |\n");
				printf("                    |-------------+--------+--------+-----------+----------+------+---------|\n");
				printf("                    | 10:45-11:15 |                      LUNCHBREAK                         |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 11:15-12:00 |                        GE104                            |\n");
				printf("                    |-------------+                                                         |\n");
				printf("                    | 12:00-1:00  |                       403-MAIN                          |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 1:00-2:00   |                         GE105                           |\n");
				printf("                    |-------------+                                                         |\n");
				printf("                    | 2:00-2:45   |                       406-MAIN                          |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 2:45-3:00   |                      BREAKTIME                          |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 3:00-4:00   |                        FCC104                           |\n");
				printf("                    |-------------+                                                         |\n");
				printf("                    | 4:00-4:45   |                       405-MAIN                          |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 4:45-5:00   |                       BREAKTIME                         |\n");
				printf("                    |-------------+--------+--------+-----------+---------+-------+---------|\n");
				printf("                    | 5:15-6:00   |                        PCK103                           |\n");
				printf("                    |-------------+                                                         |\n");
				printf("                    | 6:00-7:00   |                      405-MAIN                           |\n");
				printf("                    |-------------+--------+--------+-----------+---------+--------+--------|\n\n\n");

                ch = getch();
                if (ch == '9') { goToDashboard(); return; }
            }
            if (year == 3) {
                universalHeader();
                printf("                                    +----------------------------------+\n");
                printf("                                    |   BTVTED 3 - Sunday Schedule     |\n");
                printf("                                    +----------------------------------+\n");
                printf("                                    |   Placeholder — Schedule TBD     |\n");
                printf("                                    +----------------------------------+\n\n"); 
                ch = getch();
                if (ch == '9') { goToDashboard(); return; }
            }
            if (year == 4) {
                universalHeader();
                printf("                                    +----------------------------------+\n");
				printf("                                    |   BTVTED 4 - Sunday Schedule     |\n");
				printf("                                    +----------------------------------+\n");
				printf("                                    |   Placeholder — Schedule TBD     |\n");
				printf("                                    +----------------------------------+\n\n");

                ch = getch();
                if (ch == '9') { goToDashboard(); return; }
            }
        }
    }

    /* ---------- REGULAR CLASSES (24 unique) ---------- */
    /* BSCS regular */
    if (strcmp(program, "BSCS") == 0) {
        if (strcmp(sectionName, "1M1") == 0) {
            universalHeader();
                printf("                  |_______________________________________________________________________________|\n");
			    printf("                  |                                  WEEKLY SCHEDULE                              |\n");
			    printf("                  |                                  COURSE: BSCS1M1                              |\n");
			    printf("                  |-------------------------------------------------------------------------------|\n");
			    printf("                  |---------------+---------+---------+-----------+-----------+--------+----------|\n");
			    printf("                  |      TIME     |MONDAY   |TUESDAY  |WEDNESDAY  |THURSDAY   |FRIDAY  |SATURDAY  |\n");
			    printf("                  |---------------+---------+---------+-----------+-----------+--------+----------|\n");
			    printf("                  |7:00-8:00      |         |         |           |           |        |          |\n");
			    printf("                  |8:00-8:45      |         |         |           |           |        |          |\n");
			    printf("                  |---------------+---------+---------+-----------+-----------+--------+----------|\n");
			    printf("                  |8:45-9:00      |BREAKTIME                                                      |\n");
			    printf("                  |---------------+---------+---------+-----------+-----------+--------+----------|\n");
			    printf("                  |9:00-10:00     |         |         |           |           |        |          |\n");
			    printf("                  |10:00-10:45    |         |         |           |           |        |          |\n");
			    printf("                  |---------------+---------+---------+-----------+-----------+--------+----------|\n");
			    printf("                  |10:45-11:15    |LUNCHBREAK                                                     |\n");
			    printf("                  |---------------+---------+---=-----+-----------+-----------+---------+---------|\n");
			    printf("                  |11:15-12:00    |CC111-LEC|         |           |           |CC112    |GEE111   |\n");
			    printf("                  |               |405-MAIN |         |           |           |405-MAIN |405-MAIN |\n");
			    printf("                  |---------------+---------+---------+-----------+-----------+         +         |\n");
			    printf("                  |12:00-1:00     |         |         |           |           |         |         |\n");
			    printf("                  |---------------+  CC112  +---------+-----------+-----------+---------+         |\n");
			    printf("                  |1:00-2:00      |CL1-MAIN |         |           |           |CC111    |         |\n");
			    printf("                  |               |         |         |           |           |405-MAIN |         |\n");
			    printf("                  |---------------+         +---------+-----------+-----------+---------+---------|\n");
			    printf("                  |2:00-2:45      |         |GEE112   |           |           |GEE113   |NSTP1    |\n");
			    printf("                  |               |         |405-MAIN |           |           |405-MAIN |405-MAIN |\n");
			    printf("                  |---------------+---------+---------+-----------+-----------+---------+---------|\n");
			    printf("                  |2:45-3:00      |BREAKTIME                                                      |\n");
			    printf("                  |---------------+---------+---------+-----------+-----------+---------+---------|\n");
			    printf("                  |3:00-4:00      |         |GEE112   |           |           |  CC111  |   NSTP1 |\n");
			    printf("                  |---------------+---------+405-MAIN +-----------+-----------+ 405-MAIN+ 405-MAIN|\n");
			    printf("                  |4:00-4:45      |PLS-105  |         |           |           |         |         |\n");
			    printf("                  |               |MAIN     |         |           |           |         |         |\n");
			    printf("                  |---------------+---------+---------+-----------+-----------+---------+---------|\n");
			    printf("                  |4:45-5:15      |BREAKTIME                                                      |\n");
			    printf("                  |---------------+---------+---------+-----------+-----------+---------+---------|\n");
			    printf("                  |5:15-6:00      |GEE113   |PATHFIT1 |           |           |CC111    |         |\n");
			    printf("                  |               |105-ANNEX|405-MAIN |           |           |405-MAIN |         |\n");
			    printf("                  |---------------+         +         +-----------+-----------+---------+---------|\n");
			    printf("                  |6:00-7:00      |         |         |           |           |         |         |\n");
			    printf("                  |---------------+---------+---------+-----------+-----------+---------+---------|\n\n\n");

            ch = getch();
            if (ch == '9') { goToDashboard(); return; }
        }
        if (strcmp(sectionName, "1N1") == 0) {
            universalHeader();
            printf("                                    +----------------------------------+\n");
			printf("                                    |        BSCS 1N1 Schedule         |\n");
			printf("                                    +----------------------------------+\n");
			printf("                                    |   Placeholder — Schedule TBD     |\n");
			printf("                                    +----------------------------------+\n\n");

            ch = getch();
            if (ch == '9') { goToDashboard(); return; }
        }
        if (strcmp(sectionName, "2M1") == 0) {
            universalHeader();
             	printf("                  |______________________________________________________________________________|\n");
				printf("                  |                               WEEKLY SCHEDULE                                |\n");
				printf("                  |                               COURSE: BSCS2M1                                |\n");
				printf("                  |------------------------------------------------------------------------------|\n");
				printf("                  |---------------+----------+---------+----------+----------+--------+----------|\n");
				printf("                  |TIME           |MONDAY    |TUESDAY  |WEDNESDAY|THURSDAY  |FRIDAY  |SATURDAY  |\n");
				printf("                  |---------------+----------+---------+----------+----------+--------+----------|\n");
				printf("                  |7:00-8:00      |          |         |          |          |GAD211  |          |\n");
				printf("                  |---------------+----------+---------+----------+----------+CL1-MAIN+----------|\n");
				printf("                  |8:00-8:45      |          |         |          |          |        |          |\n");
				printf("                  |               |          |         |          |          |        |          |\n");
				printf("                  |---------------+----------+---------+----------+----------+--------+----------|\n");
				printf("                  |8:45-9:00      |BREAKTIME                                                     |\n");
				printf("                  |---------------+----------+---------+----------+----------+--------+----------|\n");
				printf("                  |9:00-10:00     |          |         |          |          |GAD211  |          |\n");
				printf("                  |10:00-10:45    |          |         |          |          |404-MAIN|          |\n");
				printf("                  |               |          |         |          |          |        |          |\n");
				printf("                  |---------------+----------+---------+----------+----------+--------+----------|\n");
				printf("                  |10:45-11:15    |LUNCHBREAK                                                    |\n");
				printf("                  |---------------+----------+---------+----------+----------+--------+----------|\n");
				printf("                  |11:15-12:00    |          |CC214    |GE116     |          |GE116   |          |\n");
				printf("                  |               |          |405-MAIN |404-MAIN  |          |404-MAIN|          |\n");
				printf("                  |---------------+----------+---------+----------+----------+        +----------|\n");
				printf("                  |12:00-1:00     |          |  CC214  |          |          |        |          |\n");
				printf("                  |---------------+----------+CL1-MAIN |          +          +--------+----------|\n");
				printf("                  |1:00-2:00      |          |         |GEE228    | GAD211   |CC22    |          |\n");
				printf("                  |---------------+----------+---------+406-MAIN  + 404-MAIN +CL1-MAIN+----------|\n");   
				printf("                  |2:00-2:45      |          |         |          |          |        |          |\n");
				printf("                  |               |          |         |          |          |        |          |\n");
				printf("                  |---------------+----------+---------+----------+----------+--------+----------|\n");
				printf("                  |2:45-3:00      |BREAKTIME                                                     |\n");
				printf("                  |---------------+----------+---------+----------+----------+--------+----------|\n");
				printf("                  |3:00-4:00      |          |PATHFIT  | GEE228   |DS121     | CC225  |          |\n");
				printf("                  |               |          |411- MAIN|406-MAIN  |404-MAIN  |CL1-MAIN|          |\n");
				printf("                  |---------------+----------+         +----------+          +        +----------|\n");
				printf("                  |4:00-4:45      |          |         |GEE1      |          |        |          |\n");
				printf("                  |               |          |         |405-MAIN  |          |        |          |\n");
				printf("                  |---------------+----------+---------+----------+----------+--------+----------|\n");
				printf("                  |4:45-5:15      |BREAKTIME                                                     |\n");
				printf("                  |---------------+----------+---------+----------+----------+--------+----------|\n");
				printf("                  |5:15-6:00      |          |CC225    |GEE1      | DS121    |        |          |\n");
				printf("                  |               |          |411-MAIN |405-MAIN  |404-MAIN  |        |          |\n");
				printf("                  |---------------+----------+         +          +----------+--------+----------|\n");
				printf("                  |6:00-7:00      |          |         |          |          |        |          |\n");
				printf("                  |---------------+----------+---------+----------+----------+--------+----------|\n\n\n");


            ch = getch();
            if (ch == '9') { goToDashboard(); return; }
        }
        if (strcmp(sectionName, "2N1") == 0) {
            universalHeader();
            printf("                                    +----------------------------------+\n");
			printf("                                    |        BSCS 2N1 Schedule         |\n");
			printf("                                    +----------------------------------+\n");
			printf("                                    |   Placeholder — Schedule TBD     |\n");
			printf("                                    +----------------------------------+\n\n");
            ch = getch();
            if (ch == '9') { goToDashboard(); return; }
        }
        if (strcmp(sectionName, "3M1") == 0) {
            universalHeader();
            printf("                  |___________________________________________________________________________|\n");
			printf("                  |                               WEEKLY SCHEDULE                             |\n");
			printf("                  |                               COURSE: BSCS3M1                             |\n");
			printf("                  |---------------------------------------------------------------------------|\n");
			printf("                  |---------------------------------------------------------------------------|\n");
			printf("                  |TIME          |MONDAY  |TUESDAY |WEDNESDAY  |THURSDAY  |FRIDAY  |SATURDAY  |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  |7:00-8:00     |        |        |           |          | SP321  |IAS311    |\n");
			printf("                  |--------------+--------+--------+-----------+----------+403-MAIN+CL1-MAIN  |\n");
			printf("                  |8:00-8:45     |        |        |           |          |        |          |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  |8:45-9:00    BREAKTIME                                                     |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  |9:00-10:00    |        |        |CC316      |          |SP321   |IAS311    |\n");
			printf("                  |              |        |        |403-MAIN   |          |403-MAIN|CL1-MAIN  |\n");
			printf("                  |--------------+--------+--------+           +----------+--------+----------|\n");
			printf("                  |10:00-10:45   |        |        |           |          |CC316   |SE321     |\n");
			printf("                  |              |        |        |           |          |CL1-MAIN|CL1-MAIN  |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  |10:45-11:15  LUNCHBREAK                                                    |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  |11:15-12:00   |        |        |           |          |CC316   |          |\n");
			printf("                  |--------------+--------+--------+           +----------+CL1-MAIN+          |\n");
			printf("                  |12:00-1:00    |        |        |ELEC311    |          |        |SE321     |\n");
			printf("                  |--------------+--------+--------+CL2-MAIN  +----------+---------+CL1-MAIN  |\n");
			printf("                  |1:00-2:00     |        |        |           |          |        |          |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  |2:00-2:45     |        |        |           |          |        |AL312     |\n");
			printf("                  |              |        |        |           |          |        |CL2-MAIN  |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  |2:45-3:00    BREAKTIME                                                     |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  |3:00-4:00     |        |        |           |          |        |AL312     |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+CL2-MAIN  |\n");
			printf("                  |4:00-4:45     |        |        |           |          |        |          |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  |4:45-5:15    BREAKTIME                                                     |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  |5:15-6:00     |        |        |           |          |        |          |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  |6:00-7:00     |        |        |           |          |        |          |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n\n\n");


            ch = getch();
            if (ch == '9') { goToDashboard(); return; }
        }
        if (strcmp(sectionName, "3N1") == 0) {
            universalHeader();
            printf("                                    +----------------------------------+\n");
			printf("                                    |        BSCS 3N1 Schedule         |\n");
			printf("                                    +----------------------------------+\n");
			printf("                                    |   Placeholder — Schedule TBD     |\n");
			printf("                                    +----------------------------------+\n\n");

            ch = getch();
            if (ch == '9') { goToDashboard(); return; }
        }
        if (strcmp(sectionName, "4M1") == 0) {
            universalHeader();
            printf("                  ____________________________________________________________________________|\n");
			printf("                  |                               WEEKLY SCHEDULE                             |\n");
			printf("                  |                               COURSE: BSCS4M1                             |\n");
			printf("                  |---------------------------------------------------------------------------|\n");
			printf("                  |---------------------------------------------------------------------------|\n");
			printf("                  |TIME          |MONDAY  |TUESDAY |WEDNESDAY  |THURSDAY  |FRIDAY  |SATURDAY  |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  |7:00-8:00     |        |        |           |          |ELEC413 |          |\n");
			printf("                  |--------------+--------+--------+-----------+----------+411-MAIN+----------|\n");
			printf("                  |8:00-8:45     |        |        |           |          |        |          |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  |8:45-9:00    BREAKTIME                                                     |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  |9:00-10:00    |        |        |           |          |ELEC413 |          |\n");
			printf("                  |              |        |        |           |          |411-MAIN|          |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  |10:00-10:45   |        |        |           |          |NC421   |          |\n");
			printf("                  |              |        |        |           |          |411-MAIN|          |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  |10:45-11:15  LUNCHBREAK                                                    |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  |11:15-12:00   |        |        |           |          |NC421   |          |\n");
			printf("                  |              |        |        |           |          |411-MAIN|          |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+  OS411   |\n");
			printf("                  |12:00-1:00    |        |        |           |          |        | CL2-MAIN |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+          |\n");
			printf("                  |1:00-2:00     |        |        |AL222      |          |        |          |\n");
			printf("                  |--------------+--------+--------+411-MAIN   +----------+--------+----------|\n");
			printf("                  |2:00-2:45     |        |        |           |          |        |          |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  |2:45-3:00    BREAKTIME                                                     |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  |3:00-4:00     |        |        |  AL222    |          |        |SAP421    |\n");
			printf("                  |4:00-4:45     |        |        | 411-MAIN  |          |        |CL1-MAIN  |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+          |\n");
			printf("                  |4:00-4:45     |        |        | NC421     |          |        |          |\n");
			printf("                  |              |        |        | CL1-MAIN  |          |        |          |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  |4:45-5:15    BREAKTIME                                                     |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  |5:15-6:00     |        |        |NC421      |          |        |SAP421    |\n");
			printf("                  |              |        |        |CL1-MAIN   |          |        |CL1-MAIN  |\n");
			printf("                  |--------------+--------+--------+           +----------+--------+----------|\n");
			printf("                  |6:00-7:00     |        |        |           |          |        |          |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n\n\n");



            ch = getch();
            if (ch == '9') { goToDashboard(); return; }
        }
        if (strcmp(sectionName, "4N1") == 0) {
            universalHeader();
            printf("                                    +----------------------------------+\n");
			printf("                                    |        BSCS 4N1 Schedule         |\n");
			printf("                                    +----------------------------------+\n");
			printf("                                    |   Placeholder — Schedule TBD     |\n");
			printf("                                    +----------------------------------+\n\n");

            ch = getch();
            if (ch == '9') { goToDashboard(); return; }
        }
    }

    /*     /* BSOA regular */
    if (strcmp(program, "BSOA") == 0) {

        if (strcmp(sectionName, "1M1") == 0) {
            universalHeader();
            printf("                  |____________________________________________________________________________|\n");
			printf("                  |                         WEEKLY SCHEDULE                                    |\n");
			printf("                  |                         COURSE: BSOA1M1                                    |\n");
			printf("                  |----------------------------------------------------------------------------|\n");
			printf("                  | TIME         | MONDAY | TUESDAY | WEDNESDAY | THURSDAY | FRIDAY | SATURDAY |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 7:00-8:00    |        |         |           |          |        |          |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 8:00-8:45    |        |         |           |          |        | NSTP1    |\n");
			printf("                  |              |        |         |           |          |        |404-MAIN  |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 8:45-9:00    BREAKTIME                                                     |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 9:00-10:00   |        |         |           |          |        | NSTP1    |\n");
			printf("                  | 10:00-10:45  |        |         |           |          |        |404-MAIN  |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 10:45-11:15  LUNCHBREAK                                                    |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 11:15-12:00  |        | GE104   |           |          |        |          |\n");
			printf("                  |              |        |403-MAIN |           |          |        |          |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 12:00-1:00   |        |         |           |          |        |          |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 1:00-2:00    | GEE104 | OACC102 |           | GE2      |        |          |\n");
			printf("                  |              |104-MAIN|411-MAIN |           |408-MAIN  |        |          |\n");
			printf("                  |--------------+        +         +-----------+          +--------+----------|\n");
			printf("                  | 2:00-2:45    |        |         |           |          |        | GE104    |\n");
			printf("                  |              |        |         |           |          |        |404-MAIN  |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 2:45-3:00    BREAKTIME                                                     |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 3:00-4:00    |        |OACC102  |           | GE2      |        | GE104    |\n");
			printf("                  |              |        |CL2-MAIN |           |408-MAIN  |        | 404-MAIN |\n");
			printf("                  |--------------+--------+         +-----------+----------+--------+          |\n");
			printf("                  | 4:00-4:45    |OACC101 |         |           |PATHFIT1  |        |          |\n");
			printf("                  |              |  102   |         |           |408-MAIN  |        |          |\n");
			printf("                  |              | ANNEX  |         |           |          |        |          |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 4:45-5:15    BREAKTIME                                                     |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 5:15-6:00    |OACC101 |OACC102  |           |PATHFIT1  |        |          |\n");
			printf("                  |              |  102   |CL2-MAIN |           |408-MAIN  |        |          |\n");
			printf("                  |--------------+ ANNEX  +         +-----------+----------+--------+----------|\n");
			printf("                  | 6:00-7:00    |        |         |           | PLS408   |        |          |\n");
			printf("                  |              |        |         |           |  MAIN    |        |          |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n\n\n");

            char ch = getch();
            if (ch == '9') { goToDashboard(); return; }
            return;
        }

        if (strcmp(sectionName, "1N1") == 0) {
            universalHeader();
            printf("                                    +----------------------------------+\n");
			printf("                                    |        BSOA 1N1 Schedule         |\n");
			printf("                                    +----------------------------------+\n");
			printf("                                    |   Placeholder — Schedule TBD     |\n");
			printf("                                    +----------------------------------+\n\n");

            char ch = getch();
            if (ch == '9') { goToDashboard(); return; }
            return;
        }

        if (strcmp(sectionName, "2M1") == 0) {
            universalHeader();
            printf("                                    +----------------------------------+\n");
			printf("                                    |        BSOA 2M1 Schedule         |\n");
			printf("                                    +----------------------------------+\n");
			printf("                                    |   Placeholder — Schedule TBD     |\n");
			printf("                                    +----------------------------------+\n\n");


            char ch = getch();
            if (ch == '9') { goToDashboard(); return; }
            return;
        }

        if (strcmp(sectionName, "2N1") == 0) {
            universalHeader();
                printf("                  |___________________________________________________________________________|\n");
				printf("                  |                         WEEKLY SCHEDULE                                   |\n");
				printf("                  |                         COURSE: BSOA2N1                                   |\n");
				printf("                  |---------------------------------------------------------------------------|\n");
				printf("                  |------------+--------+---------+-----------+-----------+--------+----------|\n");
				printf("                  |    TIME    | MONDAY | TUESDAY | WEDNESDAY | THURSDAY  | FRIDAY | SATURDAY |\n");
				printf("                  |------------+--------+---------+-----------+-----------+--------+----------|\n");
				printf("                  | 7:00-8:00  |        |         |           |           |        |          |\n");
				printf("                  | 8:00-8:45  |        |         |           |           |        |          |\n");
				printf("                  |------------+--------+---------+-----------+-----------+--------+----------|\n");
				printf("                  | 8:45-9:00  |                       BREAKTIME                              |\n");
				printf("                  |------------+--------+---------+-----------+-----------+--------+----------|\n");
				printf("                  | 9:00-10:00 |        | OACC205 |           |           |        |          |\n");
				printf("                  |------------+--------+402-MAIN +-----------+-----------+--------+----------|\n");
				printf("                  |10:00-10:45 |        |         |           |           |        |          |\n");
				printf("                  |------------+--------+---------+-----------+-----------+--------+----------|\n");
				printf("                  |10:45-11:15 |                        LUNCHBREAK                            |\n");
				printf("                  |------------+--------+---------+-----------+-----------+--------+----------|\n");
				printf("                  |11:15-12:00 |        | OACC205 |           |           |        |          |\n");
				printf("                  |            |        |402-MAIN |           |           |        |          |\n");
				printf("                  |------------+--------+---------+-----------+-----------+--------+----------|\n");
				printf("                  |12:00-1:00  |        |         |           |           |        |          |\n");
				printf("                  |------------+--------+   GE5   +-----------+-----------+--------+----------|\n");
				printf("                  |1:00-2:00   |        |403-MAIN |           |           |  CBME1 |          |\n");
				printf("                  |------------+--------+         +-----------+-----------+403-MAIN+----------|\n");
				printf("                  |2:00-2:45   |        |         |   OACC104 |           |        |          |\n");
				printf("                  |            |        |         |  403-MAIN |           |        |          |\n");
				printf("                  |------------+--------+---------+-----------+-----------+--------+----------|\n");
				printf("                  |2:45-3:00   |                       BREAKTIME                              |\n");
				printf("                  |------------+--------+---------+-----------+-----------+--------+----------|\n");
				printf("                  |3:00-4:00   |        |         |           |           |  CBME1 |          |\n");
				printf("                  |            |        |         |  OACC104  |           |403-MAIN|          |\n");
				printf("                  |------------+--------+---------+  403-MAIN +-----------+--------+----------|\n");
				printf("                  |4:00-4:45   |        |         |           |           |   GE6  |          |\n");
				printf("                  |            |        |         |           |           |403-MAIN|          |\n");
				printf("                  |------------+--------+---------+-----------+-----------+--------+----------|\n");
				printf("                  |4:45-5:15   |                         BREAKTIME                            |\n");
				printf("                  |------------+--------+---------+-----------+-----------+--------+----------|\n");
				printf("                  |5:15-6:00   |        | PATHFIT3|    VAL    |           |  GE6   |          |\n");
				printf("                  |------------+--------+405-MAIN +  403-MAIN +-----------+403-MAIN+----------|\n");
				printf("                  |6:00-7:00   |        |         |           |           |        |          |\n");
				printf("                  |------------+--------+---------+-----------+-----------+--------+----------|\n\n\n");



            char ch = getch();
            if (ch == '9') { goToDashboard(); return; }
            return;
        }

        if (strcmp(sectionName, "3M1") == 0) {
            universalHeader();
            printf("                  |___________________________________________________________________________|\n");
			printf("                  |                          WEEKLY SCHEDULE                                   |\n");
			printf("                  |                          COURSE: BSOA3M1                                   |\n");
			printf("                  |----------------------------------------------------------------------------|\n");
			printf("                  | TIME         | MONDAY | TUESDAY | WEDNESDAY | THURSDAY | FRIDAY | SATURDAY |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 7:00-8:00    |        |         |           |          |        | GE8      |\n");
			printf("                  |              |        |         |           |          |        |406-MAIN  |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 8:00-8:45    |        |         |           |          |        | GE8      |\n");
			printf("                  |              |        |         |           |          |        |406-MAIN  |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 8:45-9:00    BREAKTIME                                                     |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 9:00-10:00   |        |         |           |          |        | GE8      |\n");
			printf("                  |              |        |         |           |          |        |406-MAIN  |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 10:00-10:45  |        |         |           |          | ELEC1  |          |\n");
			printf("                  |              |        |         |           |          |402-MAIN|          |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 10:45-11:15  LUNCHBREAK                                                    |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 11:15-12:00  | ELEC2  |         |           |          | ELEC1  | OACC311  |\n");
			printf("                  |              |CL2-MAIN|         |           |          |402-MAIN|406-MAIN  |\n");
			printf("                  |--------------+        +---------+-----------+----------+--------+----------|\n");
			printf("                  | 12:00-1:00   |        |         |           |          |        |OACC311   |\n");
			printf("                  |              |        |         |           |          |        |406-MAIN  |\n");
			printf("                  |--------------+        +---------+-----------+----------+--------+----------|\n");
			printf("                  | 1:00-2:00    |        |         |           |          |        |OACC311   |\n");
			printf("                  |              |        |         |           |          |        |406-MAIN  |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 2:00-2:45    | ELEC2  |OACC312  |           |          |OACC312 |          |\n");
			printf("                  |              |CL2-MAIN|405-MAIN |           |          |402-MAIN|          |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 2:45-3:00    BREAKTIME                                                     |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 3:00-4:00    | ELEC2  |OACC312  |           |          |OACC312 |          |\n");
			printf("                  |              |CL2-MAIN|405-MAIN |           |          |402-MAIN|          |\n");
			printf("                  |--------------+--------+         +-----------+----------+        +----------|\n");
			printf("                  | 4:00-4:45    | ELEC2  |         |           |          |        |          |\n");
			printf("                  |              |403-MAIN|         |           |          |        |          |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 4:45-5:15    BREAKTIME                                                     |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 5:15-6:00    |        |         |           |          |        |          |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 6:00-7:00    |        |         |           |          |        |          |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n\n\n");


            char ch = getch();
            if (ch == '9') { goToDashboard(); return; }
            return;
        }

        if (strcmp(sectionName, "3N1") == 0) {
            universalHeader();
            printf("                                    +----------------------------------+\n");
			printf("                                    |        BSOA 3N1 Schedule         |\n");
			printf("                                    +----------------------------------+\n");
			printf("                                    |   Placeholder — Schedule TBD     |\n");
			printf("                                    +----------------------------------+\n\n");

            char ch = getch();
            if (ch == '9') { goToDashboard(); return; }
            return;
        }

        if (strcmp(sectionName, "4M1") == 0) {
            universalHeader();
            printf("                  |____________________________________________________________________________|\n");
			printf("                  |                          WEEKLY SCHEDULE                                   |\n");
			printf("                  |                          COURSE: BSOA4M1                                   |\n");
			printf("                  |----------------------------------------------------------------------------|\n");
			printf("                  | TIME         | MONDAY | TUESDAY | WEDNESDAY | THURSDAY | FRIDAY | SATURDAY |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 7:00-8:00    |        |         |           |          | ELEC5  |          |\n");
			printf("                  |--------------+--------+---------+-----------+----------+404-MAIN+----------|\n");
			printf("                  | 8:00-8:45    |        |         |           |          |        |          |\n");
			printf("                  |              |        |         |           |          |        |          |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 8:45-9:00    BREAKTIME                                                     |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 9:00-10:00   |        |         |           |          | ELEC5  |          |\n");
			printf("                  |              |        |         |           |          |404-MAIN|          |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 10:00-10:45  |        |         |           |          |        |          |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 10:45-11:15  LUNCHBREAK                                                    |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 11:15-12:00  |        |         |           |          |        |          |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 12:00-1:00   |        |         |           |          |        |          |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 1:00-2:00    |        |         |           |          |        |          |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 2:00-2:45    |        |         |   CBME2   |          |        |          |\n");
			printf("                  |              |        |         | 404-MAIN  |          |        |          |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 2:45-3:00    BREAKTIME                                                     |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 3:00-4:00    |        |         |   CBME2   | ELEC4    |        |          |\n");
			printf("                  |              |        |         | 404-MAIN  | 403-MAIN |        |          |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 4:00-4:45    |        |         |           | OACC415  |        |          |\n");
			printf("                  |              |        |         |           |403-MAIN  |        |          |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 4:45-5:15    BREAKTIME                                                     |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n");
			printf("                  | 6:00-7:00    |        |         |   ELEC4   | OACC415  |        |          |\n");
			printf("                  |              |        |         | 404-MAIN  |403-MAIN  |        |          |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+----------|\n\n\n");


            char ch = getch();
            if (ch == '9') { goToDashboard(); return; }
            return;
        }

        if (strcmp(sectionName, "4N1") == 0) {
            universalHeader();
            printf("                                    +----------------------------------+\n");
			printf("                                    |        BSOA 4N1 Schedule         |\n");
			printf("                                    +----------------------------------+\n");
			printf("                                    |   Placeholder — Schedule TBD     |\n");
			printf("                                    +----------------------------------+\n\n");

            char ch = getch();
            if (ch == '9') { goToDashboard(); return; }
            return;
        }
    }
    /* BTVTED regular */
    if (strcmp(program, "BTVTED") == 0) {

        if (strcmp(sectionName, "1M1") == 0) {
            universalHeader();
            printf("                  |____________________________________________________________________________|\n");
			printf("                  |                               WEEKLY SCHEDULE                              |\n");
			printf("                  |                               EDUC1M1/FIRST YEAR                           |\n");
			printf("                  |------------+--------+---------+-------------+------------+--------+--------|\n");
			printf("                  |     TIME   | MONDAY | TUESDAY | WEDNESDAY |  THURSDAY  | FRIDAY | SATURDAY |\n");
			printf("                  |------------+--------+---------+-----------+------------+--------+----------|\n");
			printf("                  | 8:00-8:45  | TLE101 |         |           |            |        |FCC101    |\n");
			printf("                  |            |  LAB   |         |           |            |        |  4O3     |\n");
			printf("                  |            |CL2-MAIN|         |           |            |        | MAIN     |\n");
			printf("                  |------------+--------+---------+-----------+------------+--------+----------|\n");
			printf("                  |                              BREAK TIME                                    |\n");
			printf("                  |------------+--------+---------+-----------+------------+--------+----------|\n");
			printf("                  | 9:00-10:45 |TLE101  |         |           |            |        |FCC101    |\n");
			printf("                  |            |LAB MAIN|         |           |            |        | 403 MAI  |\n");
			printf("                  |------------+--------+---------+-----------+------------+--------+----------|\n");
			printf("                  |   10:45-11:15                              LUNCH TIME                      |\n");
			printf("                  |------------+--------+---------+-----------+------------+--------+----------|\n");
			printf("                  | 11:15-12:00|        |         | GE102 104 |            |        |NSTP1 403 |\n");
			printf("                  |------------+--------+---------+   ANNEX   +------------+--------+  MAIN    |\n");
			printf("                  | 12:00-2:00 |        |         |           |            |        |          |\n");
			printf("                  |            | TLE102 | GE101   |           |            |        |          |\n");
			printf("                  |------------+   403  |  104    +-----------+------------+--------+----------|\n");
			printf("                  | 2:00-2:45  |  MAIN  | MAIN    |  PATHFIT  |            |        |FCC102 403|\n");
			printf("                  |            |        |         | 104 ANNEX |            |        |   MAIN   |\n");
			printf("                  |------------+--------+---------+-----------+------------+--------+----------|\n");
			printf("                  |  2:45-3:00                     BREAK TIME                                  |\n");
			printf("                  |------------+--------+--------+------------+------------+--------+----------|\n");
			printf("                  | 3:00-4:00  | PLS    |        |  PATHFIT 1 |            |        |          |\n");
			printf("                  |            | 102    | TLE101 |  104 ANNEX |            |        | FCC102   |\n");
			printf("                  |-----------+---------+   LEC  +------------+------------+--------+   403    |\n");
			printf("                  | 4:00-4:45 |         |   403  |  GE103 104 |            |        |   MAIN   |\n");
			printf("                  |           |         |  MAIN  |   ANNEX    |            |        |          |\n");
			printf("                  |-----------+---------+--------+------------+------------+--------+----------|\n");
			printf("                  |                                 BREAK TIME                                 |\n");
			printf("                  |-----------+---------+--------+------------+------------+--------+----------|\n");
			printf("                  | 5:15-6:00 |         |        | GE103 104  |            |        |          |\n");
			printf("                  |-----------+---------+--------+------------+------------+--------+----------|\n");
			printf("                  | 6:00-7:00 |         |        |    ANNEX   |            |        |          |\n");
			printf("                  |           |         |        |            |            |        |          |\n");
			printf("                  |-----------+---------+--------+------------+------------+--------+----------|\n");
			printf("                  |____________________________________________________________________________|\n\n\n");



            char ch = getch();
            if (ch == '9') { goToDashboard(); return; }
            return;
        }

        if (strcmp(sectionName, "1N1") == 0) {
            universalHeader();
            printf("                                    +----------------------------------+\n");
			printf("                                    |       BTVTED 1N1 Schedule        |\n");
			printf("                                    +----------------------------------+\n");
			printf("                                    |   Placeholder — Schedule TBD     |\n");
			printf("                                    +----------------------------------+\n\n");


            char ch = getch();
            if (ch == '9') { goToDashboard(); return; }
            return;
        }

        if (strcmp(sectionName, "2M1") == 0) {
            universalHeader();
            printf("                                    +----------------------------------+\n");
			printf("                                    |       BTVTED 2M1 Schedule        |\n");
			printf("                                    +----------------------------------+\n");
			printf("                                    |   Placeholder — Schedule TBD     |\n");
			printf("                                    +----------------------------------+\n\n");

            char ch = getch();
            if (ch == '9') { goToDashboard(); return; }
            return;
        }

        if (strcmp(sectionName, "2N1") == 0) {
            universalHeader();
            printf("                  |___________________________________________________________________________|\n");
			printf("                  |                          WEEKLY SCHEDULE                                  |\n");
			printf("                  |                    COURSE: EDUC2N1/SECOND YEAR                            |\n");
			printf("                  |---------------------------------------------------------------------------|\n");
			printf("                  |---------------------------------------------------------------------------|\n");
			printf("                  | TIME         | MONDAY | TUESDAY| WEDNESDAY | THURSDAY | FRIDAY | SATURDAY |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  | 7:00-8:00    |        |        |           |          |        |  TLE104  |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+   405    |\n");
			printf("                  | 8:00-8:45    |        |        |           |          |        |   MAIN   |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  | 8:45-9:00                      BREAKTIME                                  |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  |              |        |        |           |          |        |  TLE104  |\n");
			printf("                  | 9:00-10:00   |        |        |           |          |        |   405    |\n");
			printf("                  |              |        |        |           |          |        |   MAIN   |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  | 10:00-10:45  |        |        |           |          |        |  GE2 405 |\n");
			printf("                  |              |        |        |           |          |        |   MAIN   |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  | 10:45-11:15                    LUNCHBREAK                                 |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  | 11:15-12:00  |        |        |           |          |        |   GE2    |\n");
			printf("                  |--------------+--------+--------+-----------+  PCK105  +--------+   405    |\n");
			printf("                  | 12:00-1:00   |        |        |           |    402   |        |   MAIN   |\n");
			printf("                  |--------------+--------+--------+-----------+   MAIN   +--------+----------|\n");
			printf("                  | 1:00-2:00    |        |        |           |          |        |          |\n");
			printf("                  |--------------+--------+ PCK104 +-----------+----------+--------+   GE1    |\n");
			printf("                  |              | MAJOR2 |  104   |           |  PCK103  |        |   405    |\n");
			printf("                  | 2:00-2:45    | LAB HE |  ANNEX |           | 402 MAIN |        |  MAIN    |\n");
			printf("                  |              |RM1ANNEX|        |           |          |        |          |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  | 2:45-3:00                       BREAKTIME                                 |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  |              |        | PCK104 |           |          |        |   GE11   |\n");
			printf("                  | 3:00-4:00    | MAJOR2 |  104   |           |  PCK103  |        |   405    |\n");
			printf("                  |              |  LAB   | ANNEX  |           |   402    |        |  MAIN    |\n");
			printf("                  |--------------+ HE- RM1+--------+-----------+   MAIN   +--------+----------|\n");
			printf("                  | 4:00-4:45    |  ANNEX | FCC104 |           |          |        |          |\n");
			printf("                  |              |        |104ANNEX|           |          |        |          |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  | 4:45-5:15                      BREAKTIME                                  |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n");
			printf("                  | 5:15-6:00    |PATFHIT3|        |  FCC104   |          |        |          |\n");
			printf("                  |--------------+  104   | FCC104 |   104     |          |        |          |\n");
			printf("                  |  6:00-7:00   |  ANNEX |        |  ANNEX    |          |        |          |\n");
			printf("                  |              |        |        |           |          |        |          |\n");
			printf("                  |--------------+--------+--------+-----------+----------+--------+----------|\n\n\n");



            char ch = getch();
            if (ch == '9') { goToDashboard(); return; }
            return;
        }

        if (strcmp(sectionName, "3M1") == 0) {
            universalHeader();
           printf("                  |_____________________________________________________________________________|\n");
			printf("                  |                          COURSE: EDUC3M1/THIRD YEAR                         |\n");
			printf("                  |                              WEEKLY SCHEDULE                                |\n");
			printf("                  |-------------+--------+---------+-----------+----------+----------+----------|\n");
			printf("                  |     TIME    | MONDAY | TUESDAY | WEDNESDAY | THURSDAY |  FRIDAY  | SATURDAY |\n");
			printf("                  |-------------+--------+---------+-----------+----------+----------+----------|\n");
			printf("                  | 8:00-8:45   | MAJOR7 |         |           |          |          |          |\n");
			printf("                  |             | LAB HE |         |           |          |          |          |\n");
			printf("                  |             |RM1ANNEX|         |           |          |          |          |\n");
			printf("                  |-------------+--------+---------+-----------+----------+----------+----------|\n");
			printf("                  | 8:45-9:00                      BREAK TIME                                   |\n");
			printf("                  |-------------+--------+---------+-----------+----------+----------+----------|\n");
			printf("                  | 9:00-10:00  | MAJOR7 |         |           |          |          |          |\n");
			printf("                  |             |  LAB   |         |           |          |          |          |\n");
			printf("                  |-------------+ HE RM1 +---------+-----------+----------+----------+----------|\n");
			printf("                  | 10:00-10:45 | ANNEX  |         |           |  MAJOR6  |          |          |\n");
			printf("                  |             |        |         |           | LAB HR-1 |          |          |\n");
			printf("                  |             |        |         |           |   ANNEX  |          |          |\n");
			printf("                  |-------------+--------+---------+-----------+----------+----------+----------|\n");
			printf("                  | 10:45-11:15                 LUNCH BREAK TIME                                |\n");
			printf("                  |-------------+--------+---------+-----------+----------+----------+----------|\n");
			printf("                  | 11:15-12:00 |        |         |           |          |          |          |\n");
			printf("                  |             | MAJOR5 |         |   MAJOR5  | MAJOR6   |          |          |\n");
			printf("                  |-------------+  LAB   +---------+   LEC     +   LAB    +----------+----------|\n");
			printf("                  | 12:00-1:00  | HE RM1 |         |   402     | HE RM1   |          |          |\n");
			printf("                  |             | ANNEX  |         |   MAIN    | ANNEX    |          |          |\n");
			printf("                  |-------------+--------+---------+-----------+----------+----------+----------|\n");
			printf("                  | 1:00-2:00   |        |         |           |          |          |          |\n");
			printf("                  |             |        | MAJOR8  |  TLE106   |          |          |          |\n");
			printf("                  |-------------+--------+   402   +    402    +----------+----------+----------|\n");
			printf("                  | 2:00-2:45   | MAJOR6 |  MAIN   |   MAIN    | TLE108   |          |          |\n");
			printf("                  |             | LEC108 |         |           |   403    |          |          |\n");
			printf("                  |             | ANNEX  |         |           |   MAIN   |          |          |\n");
			printf("                  |-------------+--------+---------+-----------+----------+----------+----------|\n");
			printf("                  | 2:45-3:00                    BREAK TIME                                     |\n");
			printf("                  |-------------+--------+---------+-----------+----------+----------+----------|\n");
			printf("                  | 3:00-4:00   | MAJOR6 | MAJOR8  |  TLE106   |          |          |          |\n");
			printf("                  |             | LEC108 |   402   |    402    |          |          |          |\n");
			printf("                  |             | ANNEX  |  MAIN   |   MAIN    | TLE108   |          |          |\n");
			printf("                  |-------------+--------+---------+-----------+   403    +----------+----------|\n");
			printf("                  | 4:00-4:45   |        |  GE107  |  TLE107   |  MAIN    |          |          |\n");
			printf("                  |             |        |404 MAIN |402  MAIN  |          |          |          |\n");
			printf("                  |-------------+--------+---------+-----------+----------+----------+----------|\n");
			printf("                  | 4:45-5:15                   BREAK TIME                                      |\n");
			printf("                  |-------------+--------+---------+-----------+----------+----------+----------|\n");
			printf("                  | 5:15-6:00   |        |  GE107  |  TLE107   | MAJOR7   |          |          |\n");
			printf("                  | 6:00-7:00   |        |   404   |    402    | LEC405   |          |          |\n");
			printf("                  |             |        |  MAIN   |   MAIN    |  MAIN    |          |          |\n");
			printf("                  |-------------+--------+---------+-----------+----------+----------+----------|\n\n\n");




            char ch = getch();
            if (ch == '9') { goToDashboard(); return; }
            return;
        }

        if (strcmp(sectionName, "3N1") == 0) {
            universalHeader();
            printf("                                    +----------------------------------+\n");
			printf("                                    |       BTVTED 3N1 Schedule        |\n");
			printf("                                    +----------------------------------+\n");
			printf("                                    |   Placeholder — Schedule TBD     |\n");
			printf("                                    +----------------------------------+\n\n");

            char ch = getch();
            if (ch == '9') { goToDashboard(); return; }
            return;
        }

        if (strcmp(sectionName, "4M1") == 0) {
            universalHeader();
            printf("                  |_____________________________________________________________________________|\n");
			printf("                  |                              COURSE: EDUC4M1/FOURTH YEAR                     |\n");
			printf("                  |                                  WEEKLY SCHEDULE                            |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+-----------|\n");
			printf("                  |       TIME   | MONDAY | TUESDAY | WEDNESDAY | THURSDAY | FRIDAY | SATURDAY  |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+-----------|\n");
			printf("                  |   7:00-8:00  |        |         |           |          |        |           |\n");
			printf("                  |              |        |         |           |          |        |           |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+-----------|\n");
			printf("                  |   8:00-8:45  |        |         |           |          |        |  ELC402   |\n");
			printf("                  |              |        |         |           |          |        | 402-MAIN  |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+-----------|\n");
			printf("                  |   8:45-9:00                   BREAK TIME                                    |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+-----------|\n");
			printf("                  |   9:00-10:00 |        |         |           |          |        |           |\n");
			printf("                  |              |        |         |           |          |        |  ELC402   |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+ 402-MAIN  |\n");
			printf("                  |  10:00-10:45 |        |         |           |          |        |           |\n");
			printf("                  |              |        |         |           |          |        |           |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+-----------|\n");
			printf("                  |  10:45-11:15                  LUNCH BREAK                                   |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+-----------|\n");
			printf("                  |  11:15-12:00 |        |         |           |          |        |           |\n");
			printf("                  |              |        |         |           |          |        |           |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+   AC303   |\n");
			printf("                  |  12:00-1:00  |        |         |           |          |        | 402- MAIN |\n");
			printf("                  |              |        |         |           |          |        |           |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+-----------|\n");
			printf("                  |  1:00-2:00   |        |         |           |          |        |           |\n");
			printf("                  |              |        |         |           |          |        |           |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+-----------|\n");
			printf("                  |   2:00-2:45  |        |         |           | AS102 402|        |   AC302   |\n");
			printf("                  |              |        |         |           |    MAIN  |        | 402- MAIN |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+-----------|\n");
			printf("                  |   2:45-3:00                   BREAK TIME                                    |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+-----------|\n");
			printf("                  |   3:00-4:00  |        |         |           |          |        |           |\n");
			printf("                  |              |        |         |           | AS102 402|        |  ELC402   |\n");
			printf("                  |--------------+--------+---------+-----------+    MAIN  +--------+  402-MAIN |\n");
			printf("                  |   4:00-4:45  |        |         |           |          |        |           |\n");
			printf("                  |              |        |         |           |          |        |           |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+-----------|\n");
			printf("                  |   4:45-5:15                   BREAK TIME                                   |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+-----------|\n");
			printf("                  |   5:15-6:00  |        |         |           |          |        |           |\n");
			printf("                  |   6:00-7:00  |        |         |           |          |        |           |\n");
			printf("                  |              |        |         |           |          |        |           |\n");
			printf("                  |--------------+--------+---------+-----------+----------+--------+-----------|\n\n\n");



            char ch = getch();
            if (ch == '9') { goToDashboard(); return; }
            return;
        }

        if (strcmp(sectionName, "4N1") == 0) {
            universalHeader();
            printf("                  |_____________________________________________________________________________|\n");
				printf("                  |                              COURSE: EDUC4N1/FOURTH YEAR                    |\n");
				printf("                  |                                  WEEKLY SCHEDULE                            |\n");
				printf("                  |--------------+--------+---------+-----------+----------+--------+-----------|\n");
				printf("                  |       TIME   | MONDAY | TUESDAY | WEDNESDAY | THURSDAY | FRIDAY | SATURDAY  |\n");
				printf("                  |--------------+--------+---------+-----------+----------+--------+-----------|\n");
				printf("                  |   7:00-8:00  |        |         |           |          |        |           |\n");
				printf("                  |              |        |         |           |          |        |           |\n");
				printf("                  |--------------+--------+---------+-----------+----------+--------+-----------|\n");
				printf("                  |   8:00-8:45  |        |         |           |          |        |  ELC402   |\n");
				printf("                  |              |        |         |           |          |        | 402-MAIN  |\n");
				printf("                  |--------------+--------+---------+-----------+----------+--------+-----------|\n");
				printf("                  |   8:45-9:00                   BREAK TIME                                    |\n");
				printf("                  |--------------+--------+---------+-----------+----------+--------+-----------|\n");
				printf("                  |   9:00-10:00 |        |         |           |          |        |           |\n");
				printf("                  |              |        |         |           |          |        |  ELC402   |\n");
				printf("                  |--------------+--------+---------+-----------+----------+--------+ 402-MAIN  |\n");
				printf("                  |  10:00-10:45 |        |         |           |          |        |           |\n");
				printf("                  |              |        |         |           |          |        |           |\n");
				printf("                  |--------------+--------+---------+-----------+----------+--------+-----------|\n");
				printf("                  |  10:45-11:15                  LUNCH BREAK                                   |\n");
				printf("                  |--------------+--------+---------+-----------+----------+--------+-----------|\n");
				printf("                  |  11:15-12:00 |        |         |           |          |        |           |\n");
				printf("                  |              |        |         |           |          |        |           |\n");
				printf("                  |--------------+--------+---------+-----------+----------+--------+-----------|\n");
				printf("                  |  12:00-1:00  |        |         |           |          |        |           |\n");
				printf("                  |              |        |         |           |          |        |           |\n");
				printf("                  |--------------+--------+---------+-----------+----------+--------+-----------|\n");
				printf("                  |  1:00-2:00   |        |         |           |          |        |           |\n");
				printf("                  |              |        |         |           |          |        |           |\n");
				printf("                  |--------------+--------+---------+-----------+----------+--------+-----------|\n");
				printf("                  |   2:00-2:45  |        |         |           |          |        |           |\n");
				printf("                  |              |        |         |           |          |        |           |\n");
				printf("                  |--------------+--------+---------+-----------+----------+--------+-----------|\n");
				printf("                  |   2:45-3:00                   BREAK TIME                                    |\n");
				printf("                  |--------------+--------+---------+-----------+----------+--------+-----------|\n");
				printf("                  |   3:00-4:00  |        |         |           |          |        |           |\n");
				printf("                  |              |        |         |           |          |        |  ELC403   |\n");
				printf("                  |--------------+--------+---------+-----------+----------+--------+  EDUC4N1  |\n");
				printf("                  |   4:00-4:45  |        |         |           |          |        |  LIBRARY  |\n");
				printf("                  |              |        |         |           |          |        |           |\n");
				printf("                  |--------------+--------+---------+-----------+----------+--------+-----------|\n");
				printf("                  |   4:45-5:15                   BREAK TIME                                    |\n");
				printf("                  |--------------+--------+---------+-----------+----------+--------+-----------|\n");
				printf("                  |   5:15-6:00  |        |         |           |          |        |           |\n");
				printf("                  |   6:00-7:00  |        |         |           |          |        |           |\n");
				printf("                  |              |        |         |           |          |        |           |\n");
				printf("                  |--------------+--------+---------+-----------+----------+--------+-----------|\n\n\n");



            char ch = getch();
            if (ch == '9') { goToDashboard(); return; }
            return;
        }
    }


    /* default fallback */
    universalHeader();
    printf("                                    +----------------------------------+\n");
	printf("                                    |       Schedule Placeholder       |\n");
	printf("                                    +----------------------------------+\n");
	printf("                                    |   Placeholder — Schedule TBD     |\n");
	printf("                                    +----------------------------------+\n\n");

    ch = getch();
    if (ch == '9') { goToDashboard(); return; }
}





void getBareSection(const char *full, char *bare) {
    int i = 0;

    /* Sunday sections look like: Sunday-BSOA1 */
    if (strncmp(full, "Sunday-", 7) == 0) {
        strcpy(bare, full);
        return;
    }

    /* Find first digit manually */
    while (full[i] != '\0') {
        if (full[i] >= '0' && full[i] <= '9')
            break;
        i++;
    }

    strcpy(bare, full + i);
}

/* ------------------------ Fixed-width print ------------------------ */
void printFixed(const char *s, int width) {
    int len = strlen(s);
    if(len >= width) for(int i=0;i<width;i++) putchar(s[i]);
    else { printf("%s", s); for(int i=len;i<width;i++) putchar(' '); }
}




/* ------------------------ Universal ASCII Header ------------------------ */
void drawAttendanceHeader(const char *title, const char *section) {
    system("cls");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |                         PHILTECH PORTAL                     |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             | %-59s |\n", title);
    printf("                             | SECTION: %-51s |\n", section);
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |  [9] Back                                                   |\n");
    printf("                             +-------------------------------------------------------------+\n\n");
}

/* ------------------------ Section Selection Screen ------------------------ */
void printSectionMenu() {
    system("cls");
    printf("\n                               +----------------------------------------------------------------------+\n");
      printf("                               |                         PHILTECH SECTION MENU                        |\n");
      printf("                               +----------------------------------------------------------------------+\n");
    printf("                               | Select mode: [*] Student                     [9] Back               |\n");
    printf("                               +----------------------------------------------------------------------+\n\n");

    // First Year
    printf("                               +----------------------------------------------------------------------+\n");
    printf("                               | FIRST YEAR SECTIONS                                                  |\n");
    printf("                               +----------------------------------------------------------------------+\n");
    printf("                               | [Q] BSCS1M1      [A] BSCS1N1      [Z] Sunday-BSCS1                   |\n");
    printf("                               | [W] BSOA1M1      [S] BSOA1N1      [X] Sunday-BSOA1                   |\n");
    printf("                               | [E] BTVTED1M1    [D] BTVTED1N1    [C] Sunday-BTVTED1                 |\n");
    printf("                               +----------------------------------------------------------------------+\n\n");

    // Second Year
    printf("                               +----------------------------------------------------------------------+\n");
    printf("                               | SECOND YEAR SECTIONS                                                 |\n");
    printf("                               +----------------------------------------------------------------------+\n");
    printf("                               | [R] BSCS2M1      [F] BSCS2N1      [V] Sunday-BSCS2                   |\n");
    printf("                               | [T] BSOA2M1      [G] BSOA2N1      [B] Sunday-BSOA2                   |\n");
    printf("                               | [Y] BTVTED2M1    [H] BTVTED2N1    [N] Sunday-BTVTED2                 |\n");
    printf("                               +----------------------------------------------------------------------+\n\n");

    // Third Year
    printf("                               +----------------------------------------------------------------------+\n");
    printf("                               | THIRD YEAR SECTIONS                                                  |\n");
    printf("                               +----------------------------------------------------------------------+\n");
    printf("                               | [U] BSCS3M1      [J] BSCS3N1      [M] Sunday-BSCS3                   |\n");
    printf("                               | [I] BSOA3M1      [K] BSOA3N1      [L] Sunday-BSOA3                   |\n");
    printf("                               | [O] BTVTED3M1    [P] BTVTED3N1    [0] Sunday-BTVTED3                 |\n");  
    printf("                               +----------------------------------------------------------------------+\n\n");

    // Fourth Year
    printf("                               +----------------------------------------------------------------------+\n");
    printf("                               | FOURTH YEAR SECTIONS                                                 |\n");
    printf("                               +----------------------------------------------------------------------+\n");
    printf("                               | [1] BSCS4M1      [4] BSCS4N1      [7] Sunday-BSCS4                   |\n");
    printf("                               | [2] BSOA4M1      [5] BSOA4N1      [8] Sunday-BSOA4                   |\n");
    printf("                               | [3] BTVTED4M1    [6] BTVTED4N1    [.] Sunday-BTVTED4                 |\n");
    printf("                               +----------------------------------------------------------------------+\n");

    // Handle input (JUST the 9-back function you requested)
    char ch = getch();
    if(ch == '9') {
        goToDashboard();
        return;
    }

    // (Your existing section selection code continues below…)
}



void getSectionChoice(char *selectedSection) {
    selectedSection[0] = '\0';
    while(1) {
        char ch = getch();
        if(ch == '9') return; // cancel/back

        // First Year
        if(ch=='Q'||ch=='q'){ strcpy(selectedSection,"BSCS1M1"); return; }
        else if(ch=='A'||ch=='a'){ strcpy(selectedSection,"BSCS1N1"); return; }
        else if(ch=='Z'||ch=='z'){ strcpy(selectedSection,"Sunday-BSCS1"); return; }
        else if(ch=='W'||ch=='w'){ strcpy(selectedSection,"BSOA1M1"); return; }
        else if(ch=='S'||ch=='s'){ strcpy(selectedSection,"BSOA1N1"); return; }
        else if(ch=='X'||ch=='x'){ strcpy(selectedSection,"Sunday-BSOA1"); return; }
        else if(ch=='E'||ch=='e'){ strcpy(selectedSection,"BTVTED1M1"); return; }
        else if(ch=='D'||ch=='d'){ strcpy(selectedSection,"BTVTED1N1"); return; }
        else if(ch=='C'||ch=='c'){ strcpy(selectedSection,"Sunday-BTVTED1"); return; }

        // Second Year
        else if(ch=='R'||ch=='r'){ strcpy(selectedSection,"BSCS2M1"); return; }
        else if(ch=='F'||ch=='f'){ strcpy(selectedSection,"BSCS2N1"); return; }
        else if(ch=='V'||ch=='v'){ strcpy(selectedSection,"Sunday-BSCS2"); return; }
        else if(ch=='T'||ch=='t'){ strcpy(selectedSection,"BSOA2M1"); return; }
        else if(ch=='G'||ch=='g'){ strcpy(selectedSection,"BSOA2N1"); return; }
        else if(ch=='B'||ch=='b'){ strcpy(selectedSection,"Sunday-BSOA2"); return; }
        else if(ch=='Y'||ch=='y'){ strcpy(selectedSection,"BTVTED2M1"); return; }
        else if(ch=='H'||ch=='h'){ strcpy(selectedSection,"BTVTED2N1"); return; }
        else if(ch=='N'||ch=='n'){ strcpy(selectedSection,"Sunday-BTVTED2"); return; }

        // Third Year
        else if(ch=='U'||ch=='u'){ strcpy(selectedSection,"BSCS3M1"); return; }
        else if(ch=='J'||ch=='j'){ strcpy(selectedSection,"BSCS3N1"); return; }
        else if(ch=='M'||ch=='m'){ strcpy(selectedSection,"Sunday-BSCS3"); return; }
        else if(ch=='I'||ch=='i'){ strcpy(selectedSection,"BSOA3M1"); return; }
        else if(ch=='K'||ch=='k'){ strcpy(selectedSection,"BSOA3N1"); return; }
        else if(ch=='L'||ch=='l'){ strcpy(selectedSection,"Sunday-BSOA3"); return; }
        else if(ch=='O'||ch=='o'){ strcpy(selectedSection,"BTVTED3M1"); return; }
        else if(ch=='P'||ch=='p'){ strcpy(selectedSection,"BTVTED3N1"); return; }
        else if(ch=='0'){ strcpy(selectedSection,"Sunday-BTVTED3"); return; }

        // Fourth Year
        else if(ch=='1'){ strcpy(selectedSection,"BSCS4M1"); return; }
        else if(ch=='2'){ strcpy(selectedSection,"BSOA4M1"); return; }
        else if(ch=='3'){ strcpy(selectedSection,"BTVTED4M1"); return; }
        else if(ch=='4'){ strcpy(selectedSection,"BSCS4N1"); return; }
        else if(ch=='5'){ strcpy(selectedSection,"BSOA4N1"); return; }
        else if(ch=='6'){ strcpy(selectedSection,"BTVTED4N1"); return; }
        else if(ch=='7'){ strcpy(selectedSection,"Sunday-BSCS4"); return; }
        else if(ch=='8'){ strcpy(selectedSection,"Sunday-BSOA4"); return; }
        else if(ch=='.'){ strcpy(selectedSection,"Sunday-BTVTED4"); return; }
        

        // invalid key, just loop again
    }
}


/* ------------------------ Reset Today's Attendance ------------------------ */
void resetTodayAttendance() {
    FILE *f=fopen("attendance_today.txt","w");
    if(f) fclose(f);
}

void takeAttendance(const char *sectionName) {
    FILE *f = fopen("schedules.txt","r");
if(!f){
    printf("Cannot open schedules.txt\n");
    printf("Press any key to return...\n");
    getch();
    goToDashboard();   // safe return
    return;
}


    char line[512], email[200], fullName[200], program[64], scheduleType[64], section[64];
    char bareSection[32];
    getBareSection(sectionName, bareSection);

    FILE *fout = fopen("attendance_today.txt","w");
if(!fout) { 
    fclose(f); 
    printf("Cannot open attendance_today.txt\n");
    printf("Press any key to return...\n");
    getch();
    goToDashboard();
    return; 
}

    FILE *fhist = fopen("attendance_history.txt","a"); // append to history
if(!fhist){ 
    fclose(f); 
    fclose(fout); 
    printf("Cannot open attendance_history.txt\n");
    printf("Press any key to return...\n");
    getch();
    goToDashboard();
    return; 
}

    // Get today's date
    char todayDate[12];
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(todayDate, sizeof(todayDate), "%Y-%m-%d", tm_info);

    int studentCount = 0;
    int pageSize = 10;  // local page size

    drawAttendanceHeader("ATTENDANCE RECORDING", sectionName);
    printf("                             [Press P=Present, A=Absent, E=Excused]\n\n");
    printf("                                   | %-30s | MARK |\n", "NAME");
    printf("                                   +--------------------------------+------+\n");

    while(fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = 0;
        if(sscanf(line,"%[^|]|%[^|]|%[^|]|%[^|]|%[^|]", email, fullName, program, scheduleType, section) != 5)
            continue;

        char studentSection[32];
        getBareSection(section, studentSection);
        if(strcmp(studentSection, bareSection) != 0) continue;

        // Print student and get mark
        printf("                                   | "); printFixed(fullName,30); printf(" | ");
        char mark = getch();
        if(mark=='p'||mark=='P') mark='P';
        else if(mark=='a'||mark=='A') mark='A';
        else if(mark=='e'||mark=='E') mark='E';
        else mark='A';
        printf("%c    |\n", mark);

        // Save to today's attendance and history
        fprintf(fout,"%s|%s|%s|%c\n", todayDate, sectionName, fullName, mark);
        fprintf(fhist,"%s|%s|%s|%c\n", todayDate, sectionName, fullName, mark);

        studentCount++;

        // ---- Pagination for long lists ----
        if(studentCount % pageSize == 0) {
            printf("\nPress any key to continue to next students...\n");
            getch();
            drawAttendanceHeader("ATTENDANCE RECORDING", sectionName);
            printf("                             [Press P=Present, A=Absent, E=Excused]\n\n");
            printf("                                   | %-30s | MARK |\n", "NAME");
            printf("                                   +--------------------------------+------+\n");
        }
    }

    if(studentCount==0){
        printf("                             | %-30s |      |\n","(No students enrolled)");
    }

    printf("                                   +--------------------------------+------+\n");
    fclose(f);
    fclose(fout);
    fclose(fhist);

    // ---- Save/discard prompt ----
    char choice = 0;
    do {
        printf("\nDo you want to save attendance and return to dashboard? [S] Save & Return / [C] Cancel: ");
        choice = getch();
        if(choice=='s'||choice=='S') {
            printf("\nAttendance saved. Returning to dashboard...\n");
            loading_screen();
			goToDashboard();
        }
        else if(choice=='c'||choice=='C') {
            printf("\nAttendance discarded. Returning to dashboard...\n");
            // remove("attendance_today.txt"); // optional discard
            loading_screen();
			goToDashboard();
        }
    } while(1);
}



/* ------------------------ Teacher Summary ------------------------ */
void generateTeacherSummary(const char *sectionName) {
    FILE *f = fopen("attendance_history.txt","r");
    if(!f){ printf("No history found.\n"); getch(); return; }

    char line[256];
    char section[64], fullName[200]; char mark;
    char names[100][200]; char dates[100][12];
    int nStudents=0, nDates=0;

    char bareSection[32];
    getBareSection(sectionName, bareSection);

    while(fgets(line,sizeof(line),f)){
        line[strcspn(line,"\r\n")] = 0;
        char ldate[12]; sscanf(line,"%[^|]|%[^|]|%[^|]|%c", ldate, section, fullName, &mark);

        char studentSection[32];
        getBareSection(section, studentSection);
        if(strcmp(studentSection, bareSection) != 0) continue;

        int found=0; for(int i=0;i<nDates;i++) if(strcmp(dates[i],ldate)==0) found=1;
        if(!found) strcpy(dates[nDates++],ldate);

        found=0; for(int i=0;i<nStudents;i++) if(strcmp(names[i],fullName)==0) found=1;
        if(!found) strcpy(names[nStudents++],fullName);
    }
    fclose(f);

    drawAttendanceHeader("TEACHER ATTENDANCE SUMMARY", sectionName);
    printf("                                   | %-30s ", "NAME");
    for(int d=0; d<nDates; d++) printf("| %s ", dates[d]+5);
    printf("|\n");
    printf("                                   +--------------------------------");
    for(int d=0; d<nDates; d++) printf("+-----");
    printf("+\n");

    for(int i=0;i<nStudents;i++){
        printf("                                   | "); printFixed(names[i],30);
        for(int d=0; d<nDates; d++){
            char todayMark=' ';
            FILE *f2=fopen("attendance_history.txt","r");
            while(fgets(line,sizeof(line),f2)){
                line[strcspn(line,"\r\n")] = 0;
                char ldate[12], lsection[64], lfull[200]; char lmark;
                sscanf(line,"%[^|]|%[^|]|%[^|]|%c",ldate,lsection,lfull,&lmark);

                char studentSection[32];
                getBareSection(lsection, studentSection);

                if(strcmp(ldate,dates[d])==0 && strcmp(studentSection, bareSection)==0 && strcmp(lfull,names[i])==0){
                    todayMark=lmark; break;
                }
            }
            fclose(f2);
            printf("|  %c  ", todayMark);
        }
        printf("|\n");
    }

    printf("                                   +--------------------------------");
    for(int d=0; d<nDates; d++) printf("+-----");
    printf("+\n");

    // ---- Wait for key, allow '9' to return to dashboard ----
    printf("\nPress any key to return to menu, or '9' to go to dashboard: ");
    char ch = getch();
    if(ch == '9') {
        loading_screen();
        goToDashboard();
    }
}



/* ------------------------ Registrar Summary ------------------------ */
void generateRegistrarSummary(const char *sectionName) {
    FILE *f = fopen("attendance_history.txt","r");
    if(!f){ printf("No history found. Press any key to return to dashboard\n"); 
	getch();
        goToDashboard();
    } 

    char line[256];
    char section[64], fullName[200]; 
    char mark;
    char names[100][200];           // store unique student names
    int p[100]={0}, a[100]={0}, e[100]={0};
    int n = 0;                      // number of students found

    char bareSection[32];
    getBareSection(sectionName, bareSection);

    while(fgets(line,sizeof(line),f)){
        line[strcspn(line,"\r\n")] = 0;
        char ldate[12];
        if(sscanf(line,"%[^|]|%[^|]|%[^|]|%c", ldate, section, fullName, &mark) != 4) continue;

        char studentSection[32];
        getBareSection(section, studentSection);
        if(strcmp(studentSection, bareSection) != 0) continue;

        int idx=-1;
        for(int j=0;j<n;j++){ if(strcmp(names[j], fullName)==0){ idx=j; break; } }
        if(idx==-1){ idx=n; strcpy(names[n], fullName); n++; }

        if(mark=='P') p[idx]++;
        else if(mark=='A') a[idx]++;
        else e[idx]++;
    }
    fclose(f);

    drawAttendanceHeader("REGISTRAR ATTENDANCE SUMMARY", sectionName);
    printf("                             | %-30s | Present | Absent | Excused |\n", "NAME");
    printf("                             +--------------------------------+---------+--------+--------+\n");

    for(int i=0;i<n;i++){
        printf("                               | "); printFixed(names[i],30);
        printf("| %6d | %6d | %6d |\n", p[i], a[i], e[i]);
    }

    if(n==0){
        printf("                        | %-30s | %6s | %6s | %6s |\n","(No students enrolled)","-","-","-");
    }

    printf("                             +--------------------------------+---------+--------+--------+\n");
    // ---- Wait for key, allow '9' to return to dashboard ----
    char ch = getch();
    if(ch == '9') {
        loading_screen();
        goToDashboard();
}
}


/* ------------------------ Wrapper Menus ------------------------ */
void teacherAttendanceMenu() {
    char section[64]; printSectionMenu(); getSectionChoice(section);
    if(strlen(section)==0) return;
    resetTodayAttendance();
    takeAttendance(section);
}

void teacherSummaryMenu() {
    char section[64]; printSectionMenu(); getSectionChoice(section);
    if(strlen(section)==0) return;
    generateTeacherSummary(section);
}

void registrarSummaryMenu() {
    char section[64]; printSectionMenu(); getSectionChoice(section);
    if(strlen(section)==0) return;
    generateRegistrarSummary(section);
}






void addStudentInteractive(char *section) {
    char lastN[50], firstN[50], middleN[50] = "";
    char fullName[200];
    char scheduleTypeLocal[64];
    char shiftLocal[4] = "M";
    int yearInt = 1;
    char choice;

    // ---------- STEP 1: NAME ----------
STEP_NAME:
    system("cls");
    printf("\n\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |                        ADD STUDENT                           |\n");
    printf("                             +-------------------------------------------------------------+\n\n");

    printf("  Last Name                 First Name                         Middle Name\n");
    printf("  +---------------------+   +--------------------------------+   +---------------------+\n");
    printf("  |                     |   |                                |   |                     |\n");
    printf("  +---------------------+   +--------------------------------+   +---------------------+\n\n");

    printf("  (Press ENTER to move between fields)\n");

    showCursor(1);
    clearBox(4, 8, 21);   // Last Name
    clearBox(30, 8, 32);  // First Name
    clearBox(67, 8, 21);  // Middle Name

    gotoxy(4, 8); inputBox(4, 8, lastN, sizeof(lastN));
    gotoxy(30, 8); inputBox(30, 8, firstN, sizeof(firstN));
    gotoxy(67, 8); inputBox(67, 8, middleN, sizeof(middleN));

    if(strlen(lastN)<2 || strlen(firstN)<2) {
        gotoxy(1, 20); printf("(System): Last and First Name must be at least 2 characters.");
        getch();
        goto STEP_NAME;
    }

    if(strlen(middleN)>0)
        snprintf(fullName,sizeof(fullName),"%s, %s %s", lastN, firstN, middleN);
    else
        snprintf(fullName,sizeof(fullName),"%s, %s", lastN, firstN);

    // Next / Back
    printf("\n\n\n                                  +----------------------+      +----------------------+\n");
    printf("                                  |        [N] Next      |      |       [B] Back       |\n");
    printf("                                  +----------------------+      +----------------------+\n");

    choice = getch();
    if(choice=='B' || choice=='b') goto STEP_NAME;

    // ---------- STEP 2: PROGRAM ----------
STEP_PROGRAM:
    system("cls");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |                        SELECT PROGRAM                        |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             | Name: %-57s |\n", fullName);
    printf("                             +-------------------------------------------------------------+\n\n");

    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |  [ C ]  BSCS  (Computer Science)                            |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |  [ O ]  BSOA  (Office Administration)                       |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |  [ T ]  BTVTED (Tech-Voc Teacher Ed)                        |\n");
    printf("                             +-------------------------------------------------------------+\n\n");

    printf("                                  +----------------------+      +----------------------+\n");
    printf("                                  |        [N] Next      |      |       [B] Back       |\n");
    printf("                                  +----------------------+      +----------------------+\n");

    choice = getch();
    if(choice=='B' || choice=='b') goto STEP_NAME;

    if(choice=='C'||choice=='c') strcpy(profMajor,"BS Computer Science");
    else if(choice=='O'||choice=='o') strcpy(profMajor,"BS Office Administration");
    else if(choice=='T'||choice=='t') strcpy(profMajor,"BTVTEd");
    else goto STEP_PROGRAM;

    // ---------- STEP 3: YEAR ----------
STEP_YEAR:
    system("cls");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |                        SELECT YEAR                           |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             | Name: %-57s |\n", fullName);
    printf("                             | Program: %-53s |\n", profMajor);
    printf("                             +-------------------------------------------------------------+\n\n");

    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |  [1]  1st Year                                               |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |  [2]  2nd Year                                               |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |  [3]  3rd Year                                               |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |  [4]  4th Year                                               |\n");
    printf("                             +-------------------------------------------------------------+\n\n");

    printf("                                  +----------------------+      +----------------------+\n");
    printf("                                  |        [N] Next      |      |       [B] Back       |\n");
    printf("                                  +----------------------+      +----------------------+\n");

    choice = getch();
    if(choice=='B' || choice=='b') goto STEP_PROGRAM;

    if(choice>='1' && choice<='4') yearInt = choice-'0';
    else goto STEP_YEAR;
    sprintf(yearLevel,"%d",yearInt);

    // ---------- STEP 4: SCHEDULE ----------
STEP_SCHEDULE:
    system("cls");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |                        SELECT SCHEDULE                       |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             | Name: %-57s |\n", fullName);
    printf("                             | Program: %-53s |\n", profMajor);
    printf("                             | Year: %-55d |\n", yearInt);
    printf("                             +-------------------------------------------------------------+\n\n");

    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |  [1] Regular Class (Mon-Sat)                                  |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |  [2] Sunday Class                                             |\n");
    printf("                             +-------------------------------------------------------------+\n\n");

    printf("                                  +----------------------+      +----------------------+\n");
    printf("                                  |        [N] Next      |      |       [B] Back       |\n");
    printf("                                  +----------------------+      +----------------------+\n");

    choice = getch();
    if(choice=='B' || choice=='b') goto STEP_YEAR;

    if(choice=='1') strcpy(scheduleTypeLocal,"Regular Class");
    else if(choice=='2') strcpy(scheduleTypeLocal,"Sunday Class");
    else goto STEP_SCHEDULE;
    strcpy(scheduleType,scheduleTypeLocal);

    // ---------- STEP 5: SHIFT (if Regular) ----------
    if(strcmp(scheduleTypeLocal,"Regular Class")==0) {
STEP_SHIFT:
        system("cls");
        printf("                             +-------------------------------------------------------------+\n");
        printf("                             |                        SELECT SHIFT                          |\n");
        printf("                             +-------------------------------------------------------------+\n");
        printf("                             | Name: %-57s |\n", fullName);
        printf("                             | Program: %-53s |\n", profMajor);
        printf("                             | Year: %-55d |\n", yearInt);
        printf("                             | Schedule: %-50s |\n", scheduleTypeLocal);
        printf("                             +-------------------------------------------------------------+\n\n");

        printf("                             +-------------------------------------------------------------+\n");
        printf("                             |  [1] Morning                                                   |\n");
        printf("                             +-------------------------------------------------------------+\n");
        printf("                             |  [2] Afternoon                                                 |\n");
        printf("                             +-------------------------------------------------------------+\n\n");

        printf("                                  +----------------------+      +----------------------+\n");
        printf("                                  |        [N] Next      |      |       [B] Back       |\n");
        printf("                                  +----------------------+      +----------------------+\n");

        choice = getch();
        if(choice=='B' || choice=='b') goto STEP_SCHEDULE;
        if(choice=='2') strcpy(shiftLocal,"N");
    }

    // ---------- SAVE ----------
    generateSection(section, yearInt, scheduleTypeLocal, shiftLocal);

    FILE *fout = fopen("schedules.txt","a");
    if(fout) {
        fprintf(fout,"n/a|%s|%s|%s|%s\n", fullName, profMajor, scheduleTypeLocal, section);
        fclose(fout);
    }

    // ---------- CONFIRMATION ----------
    system("cls");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |                       STUDENT ADDED                          |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             | Name: %-57s |\n", fullName);
    printf("                             | Program: %-53s |\n", profMajor);
    printf("                             | Year: %-55d |\n", yearInt);
    printf("                             | Schedule: %-50s |\n", scheduleTypeLocal);
    printf("                             | Section: %-50s |\n", section);
    printf("                             +-------------------------------------------------------------+\n\n");

    printf("                             Press any key to return to the dashboard...");
    getch();
    goToDashboard();
}

void manageSectionStudents() {
    char section[64];
    printSectionMenu();
    getSectionChoice(section);
    if(strlen(section) == 0) return;

    while(1) {
        system("cls");
        printf("                             +-------------------------------------------------------------+\n");
        printf("                             |                        PHILTECH PORTAL                      |\n");
        printf("                             |                         STUDENT LIST                        |\n");
        printf("                             +-------------------------------------------------------------+\n");
        printf("                             | SECTION: %-57s |\n", section);
        printf("                             +-------------------------------------------------------------+\n\n");

        // Table header
        printf("                             +--------------------------------+-------------------------+\n");
        printf("                             | %-30s | %-23s |\n", "STUDENT NAME", "PROGRAM");
        printf("                             +--------------------------------+-------------------------+\n");

        FILE *f = fopen("schedules.txt","r");
        int count = 0;

        if(f) {
            char line[512], fullName[200], program[128], schedType[64], sec[64];

            while(fgets(line, sizeof(line), f)) {
                line[strcspn(line, "\r\n")] = 0;
                if(sscanf(line,"%*[^|]|%[^|]|%[^|]|%[^|]|%[^|]",
                          fullName, program, schedType, sec) != 4) continue;

                // Convert program to short code
                char progShort[16];
                if(strstr(program,"Computer")) strcpy(progShort,"BSCS");
                else if(strstr(program,"Office")) strcpy(progShort,"BSOA");
                else if(strstr(program,"TVT") || strstr(program,"Teacher")) strcpy(progShort,"BTVTED");
                else strcpy(progShort, program);

                // Combine program short code + section
                char fullSection[32];
                snprintf(fullSection,sizeof(fullSection),"%s%s", progShort, sec);

                // Compare with menu selection
                if(strcmp(fullSection, section) == 0) {
                    printf("                             | "); printFixed(fullName,30);
                    printf(" | "); printFixed(progShort,23); printf(" |\n");
                    count++;
                }
            }
            fclose(f);
        }

        if(count == 0)
            printf("                             | %-30s | %-23s |\n","(No students enrolled)","-");

        printf("                             +--------------------------------+-------------------------+\n\n");

        // Options box
        printf("                                  +----------------------+      +----------------------+\n");
        printf("                                  |     [A] Add Student  |      |       [9] Back       |\n");
        printf("                                  +----------------------+      +----------------------+\n");

        char choice = getch();
        if(choice=='9') {
            loading_screen();
            goToDashboard();
            return;
        }
        if(choice=='a' || choice=='A') {
            addStudentInteractive(section);  // Call the add student function
        }
    }
}
void displayAllStudentSchedules() {
    char selectedSection[50] = {0};
    char program[100] = {0};
    int year = 0;
    char placeholderSection[50] = {0};

    // 1. Show the static section menu
    printSectionMenu();

    // 2. Wait for the user to select a section
    getSectionChoice(selectedSection); // e.g., "BSCS1M1", "Sunday-BSCS1"

    // 3. Determine program
    if (strstr(selectedSection, "BSCS")) {
        strcpy(program, "BSCS");
    } else if (strstr(selectedSection, "BSOA")) {
        strcpy(program, "BSOA");
    } else if (strstr(selectedSection, "BTVTED")) {
        strcpy(program, "BTVTED");
    } else {
        printf("\n(System): Unknown program in section '%s'\n", selectedSection);
        return;
    }

    // 4. Translate selectedSection into placeholder format
    if (strstr(selectedSection, "Sunday")) {
        // Sunday classes: keep full string
        strcpy(placeholderSection, selectedSection);
        year = selectedSection[strlen(selectedSection) - 1] - '0';
    } else {
        // Regular classes: remove program prefix
        if (strncmp(selectedSection, "BSCS", 4) == 0)
            strcpy(placeholderSection, selectedSection + 4);
        else if (strncmp(selectedSection, "BSOA", 4) == 0)
            strcpy(placeholderSection, selectedSection + 4);
        else if (strncmp(selectedSection, "BTVTED", 6) == 0)
            strcpy(placeholderSection, selectedSection + 6);

        // Determine year from first character of section
        year = placeholderSection[0] - '0';
    }

    // 5. Call the placeholder function (header + back button inside)
    printSchedulePlaceholder(program, placeholderSection, year);
}






void changePassword(const char *emailInput) {

    char newPass[200];
    char confirmPass[200];

START_CHANGE:

    system("cls");
    printf("\n\n");
    printf("                             +------------------------------------------------------------------+\n");
    printf("                             |                        CHANGE PASSWORD                            |\n");
    printf("                             +------------------------------------------------------------------+\n\n");

    printf("                             Enter New Password\n");
    printf("                             +------------------------------------------------------------------+\n");
    printf("                             |                                                                  |\n");
    printf("                             +------------------------------------------------------------------+\n\n");

    printf("                             Confirm New Password\n");
    printf("                             +------------------------------------------------------------------+\n");
    printf("                             |                                                                  |\n");
    printf("                             +------------------------------------------------------------------+\n\n");

    /* ? REMOVE button print here — THEY APPEAR LATER */

    /* ---------------- NEW PASSWORD INPUT ---------------- */
    showCursor(1);
    memset(newPass, 0, sizeof(newPass));
    gotoxy(30, 8);

    int ch, idx = 0;

    while ((ch = getch()) != 13) {       // ENTER finishes typing
        if (ch == 8 && idx > 0) {        // BACKSPACE
            idx--;
            newPass[idx] = '\0';
            printf("\b \b");
        }
        else if (ch >= 32 && ch <= 126 && idx < sizeof(newPass)-1) {
            newPass[idx++] = ch;
            putchar('*');
        }
        else if (ch == 'b' || ch == 'B') {
            return;
        }
    }
    newPass[idx] = '\0';

    /* ---------------- CONFIRM PASSWORD INPUT ---------------- */
    memset(confirmPass, 0, sizeof(confirmPass));
    gotoxy(30, 13);
    idx = 0;

    while ((ch = getch()) != 13) {
        if (ch == 8 && idx > 0) {
            idx--;
            confirmPass[idx] = '\0';
            printf("\b \b");
        }
        else if (ch >= 32 && ch <= 126 && idx < sizeof(confirmPass)-1) {
            confirmPass[idx++] = ch;
            putchar('*');
        }
        else if (ch == 'b' || ch == 'B') {
            return;
        }
    }
    confirmPass[idx] = '\0';

    /* ---------------- NOW SHOW BUTTONS (AFTER ENTER) ---------------- */
    showCursor(0);
    printf("\n\n");
    printf("                                  +------------------+      +------------------+\n");
    printf("                                  |     [N] Next     |      |     [B] Back     |\n");
    printf("                                  +------------------+      +------------------+\n");
    printf("\n                                  Press N to save or B to go back...");

BUTTONS:
    char key = getch();
    if (key >= 'A' && key <= 'Z') key += 32;

    if (key == 'b') return;
    if (key != 'n') goto BUTTONS;

    /* ---------------- VALIDATION ---------------- */
    if (strcmp(newPass, confirmPass) != 0) {
        printf("\n\n                                  Passwords do not match! Press any key...");
        getch();
        goto START_CHANGE;
    }

    /* ---------------- SAFE UPDATE FOR 15 FIELDS ---------------- */
    FILE *fin = fopen("users.txt", "r");
    FILE *fout = fopen("users.tmp", "w");

    if (!fin || !fout) {
        printf("\n\n                                  Error updating password!");
        getch();
        return;
    }

    char line[4096];

    while (fgets(line, sizeof(line), fin)) {

        char fields[15][600];
        int count = 0;

        char *token = strtok(line, "|");
        while (token && count < 15) {
            strcpy(fields[count++], token);
            token = strtok(NULL, "|");
        }

        if (count != 15) {   // malformed line
            fputs(line, fout);
            continue;
        }

        /* If same email ? update ONLY password field (#1) */
        if (strcmp(fields[0], emailInput) == 0) {
            strcpy(fields[1], newPass);
        }

        fprintf(fout,
            "%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s\n",
            fields[0], fields[1], fields[2], fields[3], fields[4],
            fields[5], fields[6], fields[7], fields[8], fields[9],
            fields[10], fields[11], fields[12], fields[13], fields[14]
        );
    }

    fclose(fin);
    fclose(fout);
    remove("users.txt");
    rename("users.tmp", "users.txt");

    printf("\n\n                                  Password updated successfully!");
    printf("\n                                  Press any key...");
    getch();
    loading_screen();
}



/* ============================
   Faculty Schedule System
   ============================ */

/* Unified Add Schedule Input for Registrar */
void addFacultySchedule(const char *facultyName){
    char subject[128], section[64], day[32], time[64], building[32], room[32];
    const char *days[7] = {"Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday"};

    /* Subject */
    while(1){
        system("cls");
        printf("\n\n");
        printf("                             +------------------------------------------------------------------+\n");
        printf("                             |                       ENTER SUBJECT CODE                         |\n");
        printf("                             +------------------------------------------------------------------+\n");
        printf("                             |                                                                  |\n");
        printf("                             +------------------------------------------------------------------+\n\n");
        showCursor(1); clearBox(30,5,60); gotoxy(30,5); inputBox(30,5,subject,sizeof(subject)); showCursor(0);
        if(strlen(subject)==0){ printf("\n\n                             Subject cannot be empty. Press any key..."); getch(); continue; }
        break;
    }

    /* Section */
    while(1){
        system("cls");
        printf("\n\n");
        printf("                             +------------------------------------------------------------------+\n");
        printf("                             |                       ENTER SECTION CODE                         |\n");
        printf("                             +------------------------------------------------------------------+\n");
        printf("                             |                                                                  |\n");
        printf("                             +------------------------------------------------------------------+\n\n");
        showCursor(1); clearBox(30,5,30); gotoxy(30,5); inputBox(30,5,section,sizeof(section)); showCursor(0);
        if(strlen(section)==0){ printf("\n\n                             Section cannot be empty. Press any key..."); getch(); continue; }
        break;
    }

    /* Day */
    while(1){
        system("cls");
        printf("\n\n");
        printf("                             +------------------------------------------------------------------+\n");
        printf("                             |                             SELECT DAY                           |\n");
        printf("                             +------------------------------------------------------------------+\n");
        printf("                             |  [1] Monday                                                      |\n");
        printf("                             +------------------------------------------------------------------+\n");
        printf("                             |  [2] Tuesday                                                     |\n");
        printf("                             +------------------------------------------------------------------+\n");
        printf("                             |  [3] Wednesday                                                   |\n");
        printf("                             +------------------------------------------------------------------+\n");
        printf("                             |  [4] Thursday                                                    |\n");
        printf("                             +------------------------------------------------------------------+\n");
        printf("                             |  [5] Friday                                                      |\n");
        printf("                             +------------------------------------------------------------------+\n");
        printf("                             |  [6] Saturday                                                    |\n");
        printf("                             +------------------------------------------------------------------+\n");
        printf("                             |  [7] Sunday                                                      |\n");
        printf("                             +------------------------------------------------------------------+\n");
        printf("                             |  [9] Cancel                                                      |\n");
        printf("                             +------------------------------------------------------------------+\n");

        char ch=getch();
        if(ch=='9') return;
        if(ch>='1' && ch<='7'){ strcpy(day,days[ch-'1']); break; }
    }

    /* Time */
    /* Time selection using QWERTY keys */
while(1){
    system("cls");
    printf("\n\n");
    printf("                             +------------------------------------------------------------------+\n");
    printf("                             |                             SELECT TIME                           |\n");
    printf("                             +------------------------------------------------------------------+\n");
    printf("                             |  [Q] 07:00 AM - 08:00 AM                                         |\n");
    printf("                             +------------------------------------------------------------------+\n");
    printf("                             |  [W] 08:00 AM - 08:45 AM                                         |\n");
    printf("                             +------------------------------------------------------------------+\n");
    printf("                             |  [E] 09:00 AM - 10:00 AM                                         |\n");
    printf("                             +------------------------------------------------------------------+\n");
    printf("                             |  [R] 10:00 AM - 10:45 AM                                         |\n");
    printf("                             +------------------------------------------------------------------+\n");
    printf("                             |  [T] 11:15 AM - 12:00 PM                                         |\n");
    printf("                             +------------------------------------------------------------------+\n");
    printf("                             |  [Y] 12:00 PM - 01:00 PM                                         |\n");
    printf("                             +------------------------------------------------------------------+\n");
    printf("                             |  [U] 02:00 PM - 02:45 PM                                         |\n");
    printf("                             +------------------------------------------------------------------+\n");
    printf("                             |  [I] 03:00 PM - 04:00 PM                                         |\n");
    printf("                             +------------------------------------------------------------------+\n");
    printf("                             |  [O] 04:00 PM - 04:45 PM                                         |\n");
    printf("                             +------------------------------------------------------------------+\n");
    printf("                             |  [P] 05:15 PM - 06:00 PM                                         |\n");
    printf("                             +------------------------------------------------------------------+\n");
    printf("                             |  [A] 06:00 PM - 07:00 PM                                         |\n");
    printf("                             +------------------------------------------------------------------+\n");
    printf("                             |  [9] Cancel                                                      |\n");
    printf("                             +------------------------------------------------------------------+\n");

    char ch = getch();
    if(ch=='9') return; // cancel
    switch(ch){
        case 'Q': case 'q': strcpy(time,"07:00 AM - 08:00 AM"); break;
        case 'W': case 'w': strcpy(time,"08:00 AM - 08:45 AM"); break;
        case 'E': case 'e': strcpy(time,"09:00 AM - 10:00 AM"); break;
        case 'R': case 'r': strcpy(time,"10:00 AM - 10:45 AM"); break;
        case 'T': case 't': strcpy(time,"11:15 AM - 12:00 PM"); break;
        case 'Y': case 'y': strcpy(time,"12:00 PM - 01:00 PM"); break;
        case 'U': case 'u': strcpy(time,"02:00 PM - 02:45 PM"); break;
        case 'I': case 'i': strcpy(time,"03:00 PM - 04:00 PM"); break;
        case 'O': case 'o': strcpy(time,"04:00 PM - 04:45 PM"); break;
        case 'P': case 'p': strcpy(time,"05:15 PM - 06:00 PM"); break;
        case 'A': case 'a': strcpy(time,"06:00 PM - 07:00 PM"); break;
        default: continue; // invalid key, stay in loop
    }
    break; // exit loop after valid selection
}


    /* Building */
    while(1){
        system("cls");
        printf("\n\n");
        printf("                             +------------------------------------------------------------------+\n");
        printf("                             |                             SELECT BUILDING                      |\n");
        printf("                             +------------------------------------------------------------------+\n");
        printf("                             |  [1] Annex                                                       |\n");
        printf("                             +------------------------------------------------------------------+\n");
        printf("                             |  [2] Main                                                        |\n");
        printf("                             +------------------------------------------------------------------+\n");
        printf("                             |  [9] Cancel                                                      |\n");
        printf("                             +------------------------------------------------------------------+\n");

        char ch=getch();
        if(ch=='1'){ strcpy(building,"Annex"); break; }
        if(ch=='2'){ strcpy(building,"Main"); break; }
        if(ch=='9') return;
    }

    /* Room */
    while(1){
        system("cls");
        printf("\n\n");
        printf("                             +------------------------------------------------------------------+\n");
        printf("                             |                             ENTER ROOM                           |\n");
        printf("                             +------------------------------------------------------------------+\n");
        printf("                             |                                                                  |\n");
        printf("                             +------------------------------------------------------------------+\n\n");
        showCursor(1); clearBox(30,5,20); gotoxy(30,5); inputBox(30,5,room,sizeof(room)); showCursor(0);
        if(strlen(room)==0){ printf("\n\n                             Room cannot be empty. Press any key..."); getch(); continue; }
        break;
    }

    /* Confirmation summary before saving */
    system("cls");
    printf("                             +------------------------------------------------------------------+\n");
    printf("                             |                         SCHEDULE SUMMARY                         |\n");
    printf("                             +------------------------------------------------------------------+\n");
    printf("                             | Faculty : %-53s |\n",facultyName);
    printf("                             | Subject : %-53s |\n",subject);
    printf("                             | Section : %-53s |\n",section);
    printf("                             | Day     : %-53s |\n",day);
    printf("                             | Time    : %-53s |\n",time);
    printf("                             | Building: %-53s |\n",building);
    printf("                             | Room    : %-53s |\n",room);
    printf("                             +------------------------------------------------------------------+\n");

    /* Button-like boxes for Save and Cancel */
    printf("                             +----------------+  +----------------+\n");
    printf("                             |   [S] Save     |  |   [9] Cancel   |\n");
    printf("                             +----------------+  +----------------+\n");

    char k = getch();
    if(k=='9') return; // cancel without saving
    if(k=='S' || k=='s'){
         // ---------- SAVE SCHEDULE ----------
    // Convert facultyName to clean Lastname, Firstname
    char cleanName[128];
    {
        char first[64], middle[64], last[64];
        first[0] = middle[0] = last[0] = 0;

        // Parse the incoming fullName assuming "Lastname, First Middle"
        sscanf(facultyName, "%63[^,], %63s %63s", last, first, middle);
        snprintf(cleanName, sizeof(cleanName), "%s, %s", last, first); // ignore middle
    }

    FILE *fs=fopen("faculty_schedules.txt","a");
    if(fs){
        fprintf(fs,"%s|%s|%s|%s|%s|%s|%s\n",
                cleanName, subject, section, day, time, building, room);
        fclose(fs);
    }

    // ---------- DISPLAY CONFIRMATION ----------
    system("cls");
    printf("                             +------------------------------------------------------------------+\n");
    printf("                             |                         SCHEDULE ADDED                           |\n");
    printf("                             +------------------------------------------------------------------+\n");
    printf("                             | Faculty : %-53s |\n", cleanName);
    printf("                             | Subject : %-53s |\n",subject);
    printf("                             | Section : %-53s |\n",section);
    printf("                             | Day     : %-53s |\n",day);
    printf("                             | Time    : %-53s |\n",time);
    printf("                             | Building: %-53s |\n",building);
    printf("                             | Room    : %-53s |\n",room);
    printf("                             +------------------------------------------------------------------+\n");
    printf("Press any key to return...");
    getch();
}
}
 
 
 
 

void viewFacultySchedule(const char *facultyName, const char *role) {

    struct FacultySched {
        char subject[64];
        char section[32];
        char day[32];
        char time[32];
        char building[32];
        char room[32];
    } entries[300];

    int n = 0;

    /* Load schedules */
    FILE *f = fopen("faculty_schedules.txt","r");
    if(f){
        char line[512];
        while(fgets(line,sizeof(line),f) && n < 300){
            line[strcspn(line,"\r\n")] = 0;

            char *fields[7];
            char *token = strtok(line,"|");
            int i = 0;

            while(token && i < 7){
                fields[i++] = token;
                token = strtok(NULL,"|");
            }
            if(i < 7) continue;

            if(strcmp(fields[0], facultyName) != 0) continue;

            strcpy(entries[n].subject, fields[1]);
            strcpy(entries[n].section, fields[2]);
            strcpy(entries[n].day, fields[3]);
            strcpy(entries[n].time, fields[4]);
            strcpy(entries[n].building, fields[5]);
            strcpy(entries[n].room, fields[6]);
            n++;
        }
        fclose(f);
    }

    const char *DAYS[7] = {
        "Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday"
    };

    system("cls");
    printf("\n");
    printf("                             +------------------------------------------------------------------+\n");
    printf("                             |                 FACULTY SCHEDULE — %s\n", facultyName);
    printf("                             +------------------------------------------------------------------+\n\n");

    /* If no schedules at all */
    if(n == 0){
        printf("                             +------------------------------------------------------------------+\n");
        printf("                             |                     NO SCHEDULES FOUND                           |\n");
        printf("                             +------------------------------------------------------------------+\n\n");

        if(strcmp(role,"Registrar")==0){
            printf("                             [A] Add Schedule   [9] Back\n");
            char c = getch();
            if(c=='A'||c=='a') addFacultySchedule(facultyName);
        }
        return;
    }

    /* Display all 7 days with tables */
    for(int d=0; d<7; d++){
        printf("                             +------------------------------------------------------------------+\n");
        printf("                             | %-64s |\n", DAYS[d]);
        printf("                             +------------------------------------------------------------------+\n");

        int found = 0;

        /* Check if there are schedules for this day */
        for(int i=0; i<n; i++){
            if(strcmp(entries[i].day, DAYS[d]) == 0){
                found = 1;
                break;
            }
        }

        if(!found){
            printf("                             |                 No schedules for today                           |\n");
            printf("                             +------------------------------------------------------------------+\n\n");
            continue;
        }

        /* Print table header */
        printf("                             | %-18s | %-12s | %-10s | %-10s |\n",
            "Time", "Subject", "Room", "Building");
        printf("                             +------------------------------------------------------------------+\n");

        /* Print all schedules for this day */
        for(int i=0; i<n; i++){
            if(strcmp(entries[i].day, DAYS[d]) == 0){
                printf("                             | %-18s | %-12s | %-10s | %-10s |\n",
                    entries[i].time,
                    entries[i].subject,
                    entries[i].room,
                    entries[i].building
                );
            }
        }

        printf("                             +------------------------------------------------------------------+\n\n");
    }

    /* Footer */
    if(strcmp(role,"Registrar")==0){
    printf("                             +----------------+  +----------------+\n");
    printf("                             | [A] Add        |  | [9] Back       |\n");
    printf("                             +----------------+  +----------------+\n");

    char c = getch();
    if(c=='A'||c=='a') addFacultySchedule(facultyName);
    if(c=='9') goToDashboard();
    return;
}
    else {
    printf("                             +----------------+\n");
    printf("                             | [9] Back       |\n");
    printf("                             +----------------+\n");

    char c = getch();
    if(c=='9') goToDashboard();
    return;
}
}





/* ------------------------
   Print Faculty List
   ------------------------ */
void printFacultyList(const char *role){
    const int perPage = 8;
    int page = 0;

    int showAll = (strcmp(role, "Faculty") != 0);

    char facultyNames[200][128];
    char departments[200][128];
    char employments[200][64];
    int total = 0;

    FILE *f = fopen("users.txt","r");
    if(f){
        char line[1024];
        while(fgets(line,sizeof(line),f) && total < 200){
            line[strcspn(line, "\r\n")] = 0;

            char *fields[20];
            char *token = strtok(line,"|");
            int i = 0;
            while(token && i < 20){
                fields[i++] = token;
                token = strtok(NULL,"|");
            }
            if(i < 16) continue;

            if(strcmp(fields[2], "Faculty") != 0) continue;

            /* Inline clean name: Lastname, Firstname (ignore middle) */
            char formatted[128];
            snprintf(formatted, sizeof(formatted), "%s, %s", fields[5], fields[3]);
            strcpy(facultyNames[total], formatted);

            strcpy(departments[total], fields[14]);
            strcpy(employments[total], fields[15]);
            total++;
        }
        fclose(f);
    }

    /* Faculty role: show only their own name */
    if(!showAll){
    	while(1){
    	
        system("cls");
        printf("\n");
        printf("                             +------------------------------------------------------------------+\n");
        printf("                             |                           FACULTY LIST                           |\n");
        printf("                             +------------------------------------------------------------------+\n\n");

        printf("                             [1] %s\n\n", Fullname);

        printf("                             +----------------+\n");
        printf("                             | Press any key  |\n");
        printf("                             +----------------+\n");

        char ch = getch();
        viewFacultySchedule(Fullname, role);
        break;
    }
}

    /* Registrar/Admin: show all with paging */
    while(1){
        system("cls");
        int start = page * perPage;
        int end = start + perPage;
        if(end > total) end = total;

        printf("\n");
        printf("                             +------------------------------------------------------------------+\n");
        printf("                             |                           FACULTY LIST                           |\n");
        printf("                             +------------------------------------------------------------------+\n\n");

        printf("                        --------------------------------------------------------------------------------\n");
        printf("                        | %-28s | %-28s | %-16s |\n","NAME","DEPARTMENT","EMPLOYMENT");
        printf("                        --------------------------------------------------------------------------------\n");

        for(int i = start; i < end; i++){
            printf("                        | [%d] %-24s | %-28s | %-16s |\n",
                (i-start)+1, facultyNames[i], departments[i], employments[i]);
        }

        printf("                        --------------------------------------------------------------------------------\n\n");
        printf("                             +-------------------------------------------------------------+\n");
        printf("                             |  [N] Next Page   [P] Prev Page   [9] Back                    |\n");
        printf("                             +-------------------------------------------------------------+\n");

        char ch = getch();

        if(ch=='9'){ goToDashboard(); return; }
        if((ch=='n'||ch=='N') && end < total){ page++; continue; }
        if((ch=='p'||ch=='P') && page > 0){ page--; continue; }

        if(ch >= '1' && ch <= '8'){
            int sel = ch - '0';
            int index = start + (sel-1);
            if(index < total){
                viewFacultySchedule(facultyNames[index], role);
            }
        }
    }
}






void forgotPasswordPage() {
    char emailInput[300];
    char newPass[200];
    char confirmPass[200];
    char line[4096];

    while (1) {
        system("cls");

        /* HEADER */
        printf("\n\n");
        printf("                             +------------------------------------------------------------------+\n");
        printf("                             |    Philippine Technological Institute of Science Arts and Trade  |\n");
        printf("                             |                         (PHILTECH) PORTAL                        |\n");
        printf("                             +------------------------------------------------------------------+\n");

        /* TITLE */
        printf("                                                     +-------------------------+\n");
        printf("                                                     |     FORGOT PASSWORD     |\n");
        printf("                                                     +-------------------------+\n\n");

        /* EMAIL PROMPT BOX */
        printf("                             Enter your registered Email Address\n\n");
        printf("                             +------------------------------------------------------------------+\n");
        printf("                             |                                                                  |\n");
        printf("                             +------------------------------------------------------------------+\n\n");

        printf("                             Press [ESC] to return to Login.\n");
        printf("                             Press [ENTER] to begin typing.\n\n");

        /* wait for ENTER or ESC */
        showCursor(0);
        int k;
        while (1) {
            k = getch();
            if (k == 13) break;
            if (k == 27) { loginPage(); return; }   // ESC -> back to login
            if (k == 'r' || k == 'R') break; // allow R as alternative
        }
        showCursor(1);

        /* clear the prompt area (optional) */
        gotoxy(0, 20); printf("                                                                                 ");
        gotoxy(0, 21); printf("                                                                                 ");

        /* EMAIL INPUT (same style as login) */
        memset(emailInput, 0, sizeof(emailInput));
        clearBox(30, 13, 50);
        gotoxy(30, 13);

        int ch, idx = 0;
        while (1) {
            ch = getch();
            if (ch == 13) break;           // ENTER -> proceed
            if (ch == 27) { loginPage(); return; } // ESC -> cancel
            if (ch == 8 && idx > 0) {      // BACKSPACE
                idx--;
                emailInput[idx] = 0;
                printf("\b \b");
                continue;
            }
            if (ch == 0 || ch == 224) { getch(); continue; }
            if (ch >= 32 && ch <= 126 && idx < (int)sizeof(emailInput)-1) {
                emailInput[idx++] = ch;
                putchar(ch);
            }
        }
        emailInput[idx] = 0;

        /* SEARCH users.txt FOR EMAIL */
        FILE *in = fopen("users.txt", "r");
        if (in == NULL) {
            system("cls");
            printf("\n\n\t\tUnable to open users database (users.txt).\n");
            printf("\t\tPress any key to return to login...\n");
            getch();
            loginPage();
            return;
        }

        int found = 0;
        /* We'll also prepare to rewrite file only if we find it */
        while (fgets(line, sizeof(line), in)) {
            /* remove trailing newline for safe comparison */
            char copy[4096];
            strncpy(copy, line, sizeof(copy)-1); copy[sizeof(copy)-1]=0;
            copy[strcspn(copy, "\r\n")] = 0;

            /* extract email (field before first '|') */
            char storedEmail[512] = {0};
            char *p = strchr(copy, '|');
            if (p) {
                size_t len = p - copy;
                if (len >= sizeof(storedEmail)) len = sizeof(storedEmail)-1;
                strncpy(storedEmail, copy, len);
                storedEmail[len] = '\0';
            } else {
                /* malformed line — skip */
                continue;
            }

            if (strcmp(storedEmail, emailInput) == 0) {
                found = 1;
                break;
            }
        }
        fclose(in);

        if (!found) {
            /* not found -> message & retry or return */
            system("cls");
            printf("\n\n\t\tEmail address not found.\n");
            printf("\t\tPress [R] to retry or [ESC] to return to Login.\n");
            while (1) {
                k = getch();
                if (k == 'r' || k == 'R') break;     // outer while loop will repeat
                if (k == 27) { loginPage(); return; } // ESC
            }
            continue; // retry
        }

       

/* INSERT NEW CODE HERE */
changePassword(emailInput);
loginPage();
return;

/* DELETE EVERYTHING BELOW THIS IN YOUR FILE:
   (the giant while(1) password-change loop)
*/

    } /* end outer while */
}

/* -------------------------------------------------------------------
   STEP 1 — Draw static form (unchanged)
   ------------------------------------------------------------------- */
void drawForm() {
    system("cls");
    printf("\n\n");
    printf("                             +--------------------------------------------------+\n");
    printf("                             |              STEP 1 - CREATE ACCOUNT             |\n");
    printf("                             +--------------------------------------------------+\n\n");

    printf("  Last Name                 First Name                         Middle Name\n");
    printf("  +---------------------+   +--------------------------------+   +---------------------+\n");
    printf("  |                     |   |                                |   |                     |\n");
    printf("  +---------------------+   +--------------------------------+   +---------------------+\n\n");

    printf("  Birth Date (MM / DD / YYYY)\n");
    printf("  +------+   +------+   +--------+\n");
    printf("  |      |   |      |   |        |\n");
    printf("  +------+   +------+   +--------+\n\n");

    printf("  Sex (Male/Female)\n");
    printf("  +---------------------------+\n");
    printf("  |                           |\n");
    printf("  +---------------------------+\n\n");

    printf("  Address Information\n");
    printf("  House & Street              Barangay               City/Municipality           Province\n");
    printf("  +----------------------+   +-------------------+   +----------------------+   +-------------------+\n");
    printf("  |                      |   |                   |   |                      |   |                   |\n");
    printf("  +----------------------+   +-------------------+   +----------------------+   +-------------------+\n");

    printf("\n  (Press ENTER to move between fields)\n");
}

/* -------------------------------------------------------------------
   Forward declarations
   ------------------------------------------------------------------- */
void dashboard();
void enrollMent();
void createAccount();
void accountInfo();
void positionChoices();
void loginPage();

void saveUserToFile();
int checkUserExists(const char *checkEmail);
int verifyLogin(const char *loginEmail, const char *password, char *roleOut, char *majorOut);
int verifyPassword(const char *loginEmail, const char *password);
void saveScheduleToFile();
int checkSection(char section[]);

/* -------------------------------------------------------------------
   Dashboard
   ------------------------------------------------------------------- */

/* -------------------------------------------------------------------
   STEP 1 — createAccount() (secondary)
   ------------------------------------------------------------------- */
void createAccount() {

START_STEP1:

    drawForm();
    showCursor(1);

    /* ------------ NAME ------------ */
NAME_RETRY:

    clearBox(4, 8, 20);
    clearBox(30, 8, 31);
    clearBox(67, 8, 20);

    gotoxy(4, 8);  inputBox(4, 8, lastN, sizeof(lastN));
    gotoxy(30, 8); inputBox(30, 8, firstN, sizeof(firstN));
    gotoxy(67, 8); inputBox(67, 8, middleN, sizeof(middleN));

   // ---------- INLINE VALIDATION ----------
int valid = 1; // assume valid

char *names[] = {lastN, firstN, middleN};
for (int i = 0; i < 3; i++) {
    int len = strlen(names[i]);
    if (len < 2) { valid = 0; break; }

    for (int j = 0; j < len; j++) {
        if (names[i][j] >= '0' && names[i][j] <= '9') {
            valid = 0;
            break;
        }
    }
    if (!valid) break;
}

if (!valid) {
    gotoxy(1, 27);
    printf("(System): Each name must be at least 2 characters and cannot contain numbers.");
    getch();
    clearBox(1, 27, 90);
    goto NAME_RETRY;
}

    /* ------------ DATE ------------ */
DATE_RETRY: ;
    char mS[5], dS[5], yS[6];
    int m, d, y;

    clearBox(4, 13, 5);
    clearBox(15, 13, 5);
    clearBox(26, 13, 7);

    gotoxy(4, 13);  inputBox(4, 13, mS, sizeof(mS));
    gotoxy(15, 13); inputBox(15, 13, dS, sizeof(dS));
    gotoxy(26, 13); inputBox(26, 13, yS, sizeof(yS));

    m = atoi(mS); d = atoi(dS); y = atoi(yS);

    if (!validateDate(m, d, y)) {
        gotoxy(1, 27); printf("(System): Invalid date. Try again.");
        getch();
        clearBox(1, 27, 90);
        goto DATE_RETRY;
    }

    sprintf(birthD, "%02d/%02d/%04d", m, d, y);

    /* ------------ SEX ------------ */
SEX_RETRY:

    clearBox(4, 18, 26);
    gotoxy(4, 18); inputBox(4, 18, sex, sizeof(sex));

    capitalize(sex);

    if (!validateSex(sex)) {
        gotoxy(1, 27); printf("(System): Enter 'Male' or 'Female'.");
        getch();
        clearBox(1, 27, 90);
        goto SEX_RETRY;
    }

    /* ------------ ADDRESS ------------ */
ADDRESS_RETRY:

    clearBox(4, 24, 21);
    clearBox(30, 24, 18);
    clearBox(54, 24, 22);
    clearBox(82, 24, 18);

    gotoxy(4, 24);  inputBox(4, 24, house, sizeof(house));
    gotoxy(30, 24); inputBox(30, 24, barangay, sizeof(barangay));
    gotoxy(54, 24); inputBox(54, 24, city, sizeof(city));
    gotoxy(82, 24); inputBox(82, 24, province, sizeof(province));

    /* ----------- ADDRESS VALIDATION (NEW FIX) ----------- */
    if (strlen(house) < 1 || strlen(barangay) < 1 ||
        strlen(city) < 1 || strlen(province) < 1) {

        gotoxy(1, 27);
        printf("(System): Address fields cannot be empty. Type 'N/A' if not applicable.");
        getch();
        clearBox(1, 27, 90);

        goto ADDRESS_RETRY;
    }

    /* ------------ NAVIGATION ------------ */
NAV1:
    gotoxy(1, 33);
    printf(" \n\n All inputs captured successfully!\n");

    printf("\n\n\t\t\t+-----------------+      +-----------------+\n");
    printf("\t\t\t|     [N] Next    |      |     [B] Back    |\n");
    printf("\t\t\t+-----------------+      +-----------------+\n");

    char c = getch();
    if (c >= 'A' && c <= 'Z') c += 32;

    if (c == 'n') { loading_screen(); accountInfo(); return; }
    if (c == 'b') goto START_STEP1;

    gotoxy(1, 40);
    printf("Invalid key. Press N or B.");
    Sleep(500);
    clearBox(1, 40, 90);
    goto NAV1;
}


/* -------------------------------------------------------------------
   STEP 2 – accountInfo()
   ------------------------------------------------------------------- */
void accountInfo() {

    char ch;
    int i;

RELOAD_UI:
    system("cls");
    printf("\n\n");
    printf("                             +--------------------------------------------------+\n");
    printf("                             |           STEP 2 - ACCOUNT INFORMATION          |\n");
    printf("                             +--------------------------------------------------+\n\n");

    printf("  Email Address (will be your login email)\n");
    printf("  +--------------------------------------------------+\n");
    printf("  |                                                  |\n");
    printf("  +--------------------------------------------------+\n\n");

    printf("  Contact Number (11 digits, starts with 09)\n");
    printf("  +---------------------+\n");
    printf("  |                     |\n");
    printf("  +---------------------+\n\n");

    printf("  Password (min 8 characters)\n");
    printf("  +---------------------+\n");
    printf("  |                     |\n");
    printf("  +---------------------+\n\n");

    printf("  Confirm Password\n");
    printf("  +---------------------+\n");
    printf("  |                     |\n");
    printf("  +---------------------+\n\n");

    /* ------------ EMAIL ------------ */
EMAIL:
    clearBox(3, 8, 49);
    gotoxy(3, 8);
    inputBox(3, 8, email, sizeof(email));

    if (!strchr(email, '@') || !strchr(email, '.')) {
        gotoxy(1, 25); printf("(System): Invalid email!");
        getch();
        clearBox(1, 25, 80);
        goto EMAIL;
    }

    /* duplicate check against users.txt */
    if (checkUserExists(email)) {
        gotoxy(1, 25); printf("(System): Email already exists.");
        getch();
        clearBox(1, 25, 80);
        goto EMAIL;
    }

    /* ------------ CONTACT ------------ */
CONTACT:
    clearBox(3, 13,20);
    gotoxy(3,13);
    inputBox(3, 13, contactN, sizeof(contactN));

    if (strlen(contactN) != 11 || contactN[0] != '0' || contactN[1] != '9') {
        gotoxy(1, 25);
        printf("(System): Invalid phone number.");
        getch();
        clearBox(1, 25, 80);
        goto CONTACT;
    }

    /* ------------ PASSWORD ------------ */
PASSWORD:
    clearBox(3, 18, 20);
    clearBox(3, 24, 20);

    memset(passW, 0, sizeof(passW));
    memset(passConfirm, 0, sizeof(passConfirm));

    /* Password typing (masked) */
    gotoxy(3, 18);
    i = 0;
    int ch_int;
    while ((ch_int = getch()) != '\r') {
        if (ch_int == 8 && i > 0) {
            i--;
            passW[i] = 0;
            printf("\b \b");
        }
        else if (ch_int >= 32 && ch_int <= 126 && i < (int)sizeof(passW)-1) {
            passW[i++] = ch_int;
            printf("*");
        }
    }

    if (strlen(passW) < 8) {
        gotoxy(1, 30); printf("(System): Password too short.");
        getch();
        clearBox(1, 30, 80);
        goto PASSWORD;
    }

    /* ------------ CONFIRM PASSWORD ------------ */
    clearBox(3, 23, 20);
    gotoxy(3, 23);

    i = 0;

    while ((ch_int = getch()) != '\r') {
        if (ch_int == 8 && i > 0) {
            i--;
            passConfirm[i] = 0;
            printf("\b \b");
        }
        else if (ch_int >= 32 && ch_int <= 126 && i < (int)sizeof(passConfirm)-1) {
            passConfirm[i++] = ch_int;
            printf("*");
        }
    }

    if (strcmp(passW, passConfirm) != 0) {
        gotoxy(1, 30); printf("(System): Passwords do not match.");
        getch();
        clearBox(1, 30, 80);
        goto PASSWORD;
    }

    /* ------------ NAV ------------ */
NAV2:
    clearBox(1, 28, 120);
    clearBox(1, 29, 120);
    clearBox(1, 30, 120);
    clearBox(1, 31, 120);

    gotoxy(20, 29);
    printf("+-----------------+      +-----------------+\n");
    gotoxy(20, 30);
    printf("|     [N] Next    |      |     [B] Back    |\n");
    gotoxy(20, 31);
    printf("+-----------------+      +-----------------+\n\n");

    ch = getch();
    if (ch >= 'A' && ch <= 'Z') ch += 32;

    if (ch == 'n') {
        loading_screen();
        positionChoices();
        return;
    }
    if (ch == 'b') {
        accountInfo();
        return;
    }

    gotoxy(1, 33);
    printf("Invalid key!");
    Sleep(500);
    clearBox(1, 33, 80);
    goto NAV2;
}

/* -------------------------------------------------------------------
   STEP 3 – positionChoices (saves user)
   ------------------------------------------------------------------- */
void positionChoices() {
    char key;
    strcpy(roleChoice, "");
    strcpy(programChoice, "");
    strcpy(facultyType, ""); // reset faculty type

    /* ======================
       STEP 3 — ROLE SELECT
       ====================== */
    while (1) {
        system("cls");
        printf("\n\n\n");
        printf("                                  +--------------------------------------------------+\n");
        printf("                                  |                STEP 3 - POSITION                 |\n");
        printf("                                  +--------------------------------------------------+\n\n");
        printf("                                  Select Your Position (Press Key)\n\n");
        printf("                             +-------------------------------------------------------------+\n");
        printf("                             |  [ S ]  Student                                             |\n");
        printf("                             +-------------------------------------------------------------+\n");
        printf("                             |  [ F ]  Faculty                                             |\n");
        printf("                             +-------------------------------------------------------------+\n");
        printf("                             |  [ A ]  Admin                                               |\n");
        printf("                             +-------------------------------------------------------------+\n");
        printf("                             |  [ R ]  Registrar                                           |\n");
        printf("                             +-------------------------------------------------------------+\n\n");
        printf("                                  Press key: ");

        key = getch();
        if (key >= 'A' && key <= 'Z') key += 32;

        if (key == 's') { strcpy(roleChoice, "Student"); break; }
        else if (key == 'f') { strcpy(roleChoice, "Faculty"); break; }
        else if (key == 'a') { strcpy(roleChoice, "Admin"); break; }
        else if (key == 'r') { strcpy(roleChoice, "Registrar"); break; }
        else {
            printf("\n\n                                  Invalid selection. Try again.");
            Sleep(600);
        }
    }

    /* ======================
       PROGRAM / DEPARTMENT SELECT
       ====================== */
    if (strcmp(roleChoice, "Student") == 0) {
        while (1) {
            system("cls");
            printf("\n\n\n                                  Select Program (Press Key)\n\n");
            printf("                             +-------------------------------------------------------------+\n");
            printf("                             |  [ C ]  BSCS  (Computer Science)                           |\n");
            printf("                             +-------------------------------------------------------------+\n");
            printf("                             |  [ O ]  BSOA  (Office Administration)                      |\n");
            printf("                             +-------------------------------------------------------------+\n");
            printf("                             |  [ T ]  BTVTED (Tech-Voc Teacher Ed)                       |\n");
            printf("                             +-------------------------------------------------------------+\n\n");
            printf("                                  Press key: ");

            key = getch();
            if (key >= 'A' && key <= 'Z') key += 32;

            if (key == 'c') { strcpy(programChoice, "BSCS (Bachelor of Science in Computer Science)"); break; }
            else if (key == 'o') { strcpy(programChoice, "BSOA (Bachelor of Science in Office Administration)"); break; }
            else if (key == 't') { strcpy(programChoice, "BTVTED (Bachelor of Technical-Vocational Teacher Education)"); break; }
            else {
                printf("\n\n                                  Invalid selection."); 
                Sleep(800);
            }
        }
    }
    else if (strcmp(roleChoice, "Faculty") == 0) {
        // Department selection
        while (1) {
            system("cls");
            printf("\n\n\n                                  Select Department (Press Key)\n\n");
            printf("                             +-------------------------------------------------------------+\n");
            printf("                             |  [ Z ]  Computer Science Department                        |\n");
            printf("                             +-------------------------------------------------------------+\n");
            printf("                             |  [ X ]  Office Administration Department                   |\n");
            printf("                             +-------------------------------------------------------------+\n");
            printf("                             |  [ D ]  Teacher Education Department                       |\n");
            printf("                             +-------------------------------------------------------------+\n");
            printf("                             |  [ V ]  General Education Department                       |\n");
            printf("                             +-------------------------------------------------------------+\n\n");
            printf("                                  Press key: ");
            key = getch();
            if (key >= 'A' && key <= 'Z') key += 32;

            if (key == 'z') { strcpy(programChoice, "Computer Science Department"); break; }
            else if (key == 'x') { strcpy(programChoice, "Office Administration Department"); break; }
            else if (key == 'd') { strcpy(programChoice, "Teacher Education Department"); break; }
            else if (key == 'v') { strcpy(programChoice, "General Education Department"); break; }
            else {
                printf("\n\n                                  Invalid selection.");
                Sleep(800);
            }
        }

        // Employment type selection
        while (1) {
            system("cls");
            printf("\n\n\n                                  Select Employment Type\n\n");
            printf("                             +-------------------------------------------------------------+\n");
            printf("                             |  [ F ]  Full-Time                                          |\n");
            printf("                             +-------------------------------------------------------------+\n");
            printf("                             |  [ P ]  Part-Time                                          |\n");
            printf("                             +-------------------------------------------------------------+\n\n");
            printf("                                  Press key: ");
            key = getch();
            if (key >= 'A' && key <= 'Z') key += 32;

            if (key == 'f') { strcpy(facultyType, "Full-Time"); break; }
            else if (key == 'p') { strcpy(facultyType, "Part-Time"); break; }
            else {
                printf("\n\n                                  Invalid selection.");
                Sleep(800);
            }
        }
    }
    else if (strcmp(roleChoice, "Admin") == 0) {
        while (1) {
            system("cls");
            printf("\n\n\n                                  Select Department (Press Key)\n\n");
            printf("                             +-------------------------------------------------------------+\n");
            printf("                             |  [ M ]  Administration Office                              |\n");
            printf("                             +-------------------------------------------------------------+\n");
            printf("                             |  [ F ]  Finance / Accounting                               |\n");
            printf("                             +-------------------------------------------------------------+\n");
            printf("                             |  [ H ]  Human Resources (HR)                               |\n");
            printf("                             +-------------------------------------------------------------+\n\n");
            printf("                                  Press key: ");
            key = getch();
            if (key >= 'A' && key <= 'Z') key += 32;

            if (key == 'm') { strcpy(programChoice, "Administration Office"); break; }
            else if (key == 'f') { strcpy(programChoice, "Finance Department"); break; }
            else if (key == 'h') { strcpy(programChoice, "Human Resources"); break; }
            else { printf("\n\n                                  Invalid selection."); Sleep(800); }
        }
    }
    else if (strcmp(roleChoice, "Registrar") == 0) {
        strcpy(programChoice, "Registrar's Office");
    }

    // --- Show final selection ---
    system("cls");
    printf("\n\n\n");
    printf("                                  +--------------------------------------------------+\n");
    printf("                                  |                STEP 3 - POSITION                 |\n");
    printf("                                  +--------------------------------------------------+\n\n");
    printf("                                  Position: %s\n", roleChoice);
    printf("                                  Department/Program: %s\n\n", programChoice);
    Sleep(1200);

    // --- Final navigation ---
    while (1) {
        system("cls");
        printf("\n\n\n");
        printf("                                  +--------------------------------------------------+\n");
        printf("                                  |                STEP 3 - POSITION                 |\n");
        printf("                                  +--------------------------------------------------+\n\n");
        printf("                                  Position: %s\n", roleChoice);
        printf("                                  Program/Dept: %s\n\n\n", programChoice);
        printf("                                  +----------------------+      +----------------------+\n");
        printf("                                  |        [N] Next      |      |       [B] Back       |\n");
        printf("                                  +----------------------+      +----------------------+\n");

        key = getch();
        if (key >= 'A' && key <= 'Z') key += 32;

        if (key == 'b') { positionChoices(); return; }
        if (key == 'n') break;
    }

    // --- Prepare final strings for saving ---
    snprintf(addressFull, sizeof(addressFull), "%s, %s, %s, %s", house, barangay, city, province);
    snprintf(Fullname, sizeof(Fullname), "%s, %s", lastN, firstN);

    saveUserToFile();


    /* SUCCESS SCREEN */
    system("cls");
    printf("\n\n\n");
    printf("                                  +--------------------------------------------------+\n");
    printf("                                  |            ACCOUNT CREATED SUCCESSFULLY          |\n");
    printf("                                  +--------------------------------------------------+\n\n");
    printf("                                  Redirecting to Login Page...\n");
    Sleep(1500);

    loading_screen();
    loginPage();
}



void aboutScreen() {
    int page = 1;
    char key;

    while (1) {
        system("cls");
        printf("\n\n");

        // TITLE BOX
        printf("                             +------------------------------------------------------------------+\n");
        printf("                             |                                ABOUT                             |\n");
        printf("                             +------------------------------------------------------------------+\n");

        // CONTENT BOX
        	printf("                             +------------------------------------------------------------------+\n");
	if (page == 1) {
        	printf("                             |                         PHILTECH PORTAL                         |\n");
            printf("                             |    Philippine Technological Institute of Science,               |\n");
            printf("                             |    Arts and Trade, Inc.  designed for students,                 |\n");
            printf("                             |   faculty, and registrar staff. It aims                         |\n");
            printf("                             |   to streamline enrollment, schedules, and academic records.    |\n");
            printf("                             |                                                                 |\n");

        }
        else if (page == 2) {
            printf("                             |                              MISSION                            |\n");
            printf("                             |   Philippine Technological Institute of Science, Arts and Trade,|\n");
            printf("                             |     Inc. (PHILTECH) is dedicated to give quality education      |\n");
            printf("                             |   to the development of every Filipino. It aims to hone students|\n");
            printf("                             |   who are values oriented and physically, academically, socially|\n");
            printf("                             |    and spiritually  committed to the achievement of life-long   |\n");
            printf("                             |             learning and service to the nation.                 |\n");
            printf("                             |                                                                 |\n");
            printf("                             |                               VISION                            |\n");
            printf("                             |    Philippine Technological Institute of Science, Arts          |\n");
            printf("                             |    and Trade, Inc. (PHILTECH) is dedicated to give              |\n");
            printf("                             |    quality education to the development of every Filipino.      |\n");

        }
        else if (page == 3) {
            printf("                             |                           SYSTEM VERSION                        |\n");
            printf("                             |               PhilTech Information System v1.0                  |\n");
            printf("                             |                                                                 |\n");
            printf("                             |                             DEVELOPERS                          |\n");
            printf("                             |   - Trisha Mae Labagala                                         |\n");
            printf("                             |   - Karlsen Falcon                                              |\n");
            printf("                             |   - Marry Joy Cortes                                            |\n");
            printf("                             |   - Bench Josh Cadille                                          |\n");

        }

        	printf("                             +------------------------------------------------------------------+\n");

        // FOOTER BOX WITH NAVIGATION
        	printf("                             +------------------------------------------------------------------+\n");
        	printf("                             |   Page %d of 3     [A] Previous   [D] Next   [M] Main Menu       |\n", page);
        	printf("                             +------------------------------------------------------------------+\n");

        key = getch();

        if (key == 'a' || key == 'A') {
            if (page > 1) page--;
        }
        else if (key == 'd' || key == 'D') {
            if (page < 3) page++;
        }
        else if (key == 'm' || key == 'M') { 
		loading_screen(); 
        goToDashboard();
        }
    }
}

void profileCard() {
    system("cls");

    // Build full name: Last, First Middle
    char fullName[300];
    snprintf(fullName, sizeof(fullName), "%s, %s %s", lastN, firstN, middleN);

    // Mask password
    char maskedPass[300];
    int len = strlen(passW);
    for(int i = 0; i < len; i++) maskedPass[i] = '*';
    maskedPass[len] = '\0';

    // Build full address
    char fullAddress[600];
    snprintf(fullAddress, sizeof(fullAddress), "%s, %s, %s, %s", house, barangay, city, province);

    char choice;

    while(1) {
        system("cls");

        // Header
        printf("      +------------------------------------------------------------------------------------+\n");
        printf("      |                               PHILTECH PORTAL                                      |\n");
        printf("      |                               PROFILE CARD                                         |\n");
        printf("      +------------------------------------------------------------------------------------+\n");
        printf("      |                                                                          [9] Back  |\n");
        printf("      +------------------------------------------------------------------------------------+\n\n");

        // Left ASCII Account Art + Right Info Box
        printf("      +-------------------------+   +------------------------------------------------------+\n");
        printf("      |     PHILTECH ACCOUNT    |   |                   Personal Information               |\n");
        printf("      +=========================+   +======================================================+\n");
        printf("      |       ________          |   | Name:        %-40s|\n", fullName);                 
        printf("      |      /        \\         |   | Gender:      %-40s|\n", sex);                     
        printf("      |      |________|         |   | Birthdate:   %-40s|\n", birthD);
        printf("      |      |        |         |   | Address:     %-40s|\n", fullAddress);               
        printf("      |      |        |         |   | Department:  %-40s|\n", programChoice);              
        printf("      |      \\      /           |   | Employment:  %-40s|\n", strcmp(roleChoice,"Faculty")==0 ? facultyType : " ");
        printf("      |       \\____/            |   |                                                      |\n");
        printf("      |   ___/      \\___        |   |                                                      |\n");
        printf("      |  /            \\         |   +------------------------------------------------------+\n"); 
        printf("      +=========================+   +------------------------------------------------------+\n");
        printf("      | [*] Personal Info       |   | Email:       %-40s|\n", email);
        printf("      | [0] Sign Out            |   | Contact #:   %-40s|\n", contactN);                   
        printf("      |                         |   | Password:    %-40s|\n", maskedPass);                   
        printf("      +-------------------------+   +------------------------------------------------------+\n");

        choice = getch();

        if(choice=='9') { 
            loading_screen(); 
            goToDashboard(); 
            return; 
        }
        else if(choice=='0') { 
            loading_screen(); 
            loginPage(); 
            return; 
        }
    }
}







 
void viewSchedule() {
    showCursor(0);
    system("cls");

    FILE *f = fopen("schedules.txt", "r");
    if (!f) {
        printf("                             +-------------------------------------------------------------+\n");
        printf("                             |     ERROR: Cannot open schedules.txt                       |\n");
        printf("                             +-------------------------------------------------------------+\n\n");
        printf("Press any key to return...\n");
    getch();                // Let user read the error
    goToDashboard();        // Return safely
    return;
    }

    char line[512];
    char storedEmail[300], storedName[256], storedProgramFull[128], storedSchedType[64];
    char storedSection[64];

    int found = 0;

    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = 0;

        int parsed = sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]",
                            storedEmail, storedName, storedProgramFull,
                            storedSchedType, storedSection);

        if (parsed == 5 && strcmp(storedEmail, email) == 0) {
            found = 1;
            break;
        }
    }

    fclose(f);

    if (!found) {
        printf("                             +-------------------------------------------------------------+\n");
        printf("                             |       NO ENROLLMENT RECORD FOUND FOR THIS ACCOUNT          |\n");
        printf("                             +-------------------------------------------------------------+\n\n");
        printf("                             Press any key to return...\n");
        getch();
        goToDashboard();
        return;
    }

    // Convert full program name to short code
    char programShort[16];
    if (strstr(storedProgramFull, "Computer")) strcpy(programShort, "BSCS");
    else if (strstr(storedProgramFull, "Office")) strcpy(programShort, "BSOA");
    else strcpy(programShort, "BTVTED");

    // Determine year from section (first character), only used for Sunday schedules
    int year = 0;
    if (storedSection[0] >= '1' && storedSection[0] <= '4')
        year = storedSection[0] - '0';

    system("cls");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |                        PHILTECH PORTAL                      |\n");
    printf("                             |                         MY SCHEDULE                         |\n");
    printf("                             +-------------------------------------------------------------+\n");

    printf("                             | Name: %-25s Program: %-12s |\n", storedName, programShort);
    printf("                             | Section: %-25s Type: %-18s |\n", storedSection, storedSchedType);
    printf("                             +-------------------------------------------------------------+\n\n");

    // ---------- Call the placeholder function ----------
    // If it's a Sunday schedule, pass the year; otherwise, pass 0
    if (strstr(storedSchedType, "Sunday")) {
        printSchedulePlaceholder(programShort, storedSection, year);
    } else {
        printSchedulePlaceholder(programShort, storedSection, 0);
    }

    printf("\n\n                             Press any key to return...");
    getch();
    showCursor(1);
    goToDashboard();
}





int isAlreadyEnrolled() {
    FILE *f = fopen("schedules.txt","r");
    if(!f) return 0;

    char line[256], storedEmail[100];

    while(fgets(line,sizeof(line),f)){
        line[strcspn(line,"\r\n")] = 0;
        sscanf(line,"%[^|]", storedEmail);
        if(strcmp(storedEmail,email) == 0){
            fclose(f);
            return 1;
        }
    }

    fclose(f);
    return 0;
}




void enrollMent() {
    showCursor(0);   // Hide cursor always

    char choice;
    char sectionName[64];
    char scheduleTypeLocal[64];
    char shiftLocal[4] = "M";
    int yearInt = 0;
    int showOptions = 0;
if (isAlreadyEnrolled()) {
        system("cls");
        printf("                             +-------------------------------------------------------------+\n");
        printf("                             |                       ENROLLMENT STATUS                     |\n");
        printf("                             +-------------------------------------------------------------+\n");
        printf("                             |        You are already enrolled in the system.              |\n");
        printf("                             +-------------------------------------------------------------+\n\n");
        printf("                             Press any key to return to dashboard...");
        getch();
        goToDashboard();
        return;
    }
    /* ================================================================
       PROGRAM SELECTION
       ================================================================ */
ProgramMenu:
    system("cls");

    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |                        PHILTECH PORTAL                      |\n");
    printf("                             |                           ENROLLMENT                        |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             | Name: %-20s                          [9] Back |\n", Fullname);
    printf("                             +-------------------------------------------------------------+\n\n");

    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |  [ C ]  BSCS  (Computer Science)                            |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |  [ O ]  BSOA  (Office Administration)                       |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |  [ T ]  BTVTED (Tech-Voc Teacher Ed)                        |\n");
    printf("                             +-------------------------------------------------------------+\n\n");

    choice = getch();
    system("cls");

    if (choice == '9') { goToDashboard(); return; }

    if (choice=='C'||choice=='c') strcpy(profMajor, "BS Computer Science");
    else if (choice=='O'||choice=='o') strcpy(profMajor, "BS Office Administration");
    else if (choice=='T'||choice=='t') strcpy(profMajor, "BTVTEd");
    else {
        printf("                             +-------------------------------------------------------------+\n");
        printf("                             |                  INVALID INPUT — TRY AGAIN                  |\n");
        printf("                             +-------------------------------------------------------------+\n\n");
        Sleep(1000);
        goto ProgramMenu;
    }

    /* ================================================================
       YEAR SELECTION
       ================================================================ */
YearMenu:
    system("cls");
	printf("                             +-------------------------------------------------------------+\n");
    printf("                             |                        PHILTECH PORTAL                      |\n");
    printf("                             |                           ENROLLMENT                        |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             | Name: %-20s                          [9] Back |\n", Fullname);
    printf("                             +-------------------------------------------------------------+\n\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |  Select Year Level                                          |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |  [ 1 ]  1st Year                                            |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |  [ 2 ]  2nd Year                                            |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |  [ 3 ]  3rd Year                                            |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |  [ 4 ]  4th Year                                            |\n");
    printf("                             +-------------------------------------------------------------+\n\n");


    choice = getch();
    system("cls");

    if (choice == '9') goto ProgramMenu;

    if (choice=='1') yearInt = 1;
    else if (choice=='2') yearInt = 2;
    else if (choice=='3') yearInt = 3;
    else if (choice=='4') yearInt = 4;
    else {
        printf("                             +-------------------------------------------------------------+\n");
        printf("                             |                  INVALID INPUT — TRY AGAIN                  |\n");
        printf("                             +-------------------------------------------------------------+\n\n");
        Sleep(1000);
        goto YearMenu;
    }
    sprintf(yearLevel, "%d", yearInt);

    /* ================================================================
       SCHEDULE TYPE
       ================================================================ */
SchedMenu:
    system("cls");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |                        PHILTECH PORTAL                      |\n");
    printf("                             |                           ENROLLMENT                        |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             | Name: %-20s                          [9] Back |\n", Fullname);
    printf("                             +-------------------------------------------------------------+\n\n");

    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |  Choose Preferred Schedule                                  |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |  [ 1 ]  Regular Class (Mon - Sat)                           |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |  [ 2 ]  Sunday Class                                        |\n");
    printf("                             +-------------------------------------------------------------+\n\n");
    printf("                             [9] Back\n");

    choice = getch();
    system("cls");

    if (choice == '9') goto YearMenu;

    if (choice=='1') strcpy(scheduleTypeLocal, "Regular Class");
    else if (choice=='2') strcpy(scheduleTypeLocal, "Sunday Class");
    else {
        printf("                             +-------------------------------------------------------------+\n");
        printf("                             |                  INVALID INPUT — TRY AGAIN                  |\n");
        printf("                             +-------------------------------------------------------------+\n\n");
        Sleep(1000);
        goto SchedMenu;
    }

    /* ================================================================
       SHIFT — ONLY FOR REGULAR
       ================================================================ */
    if (strcmp(scheduleTypeLocal, "Regular Class") == 0) {

ShiftMenu:
        system("cls");
	printf("                             +-------------------------------------------------------------+\n");
    printf("                             |                        PHILTECH PORTAL                      |\n");
    printf("                             |                           ENROLLMENT                        |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             | Name: %-20s                          [9] Back |\n", Fullname);
    printf("                             +-------------------------------------------------------------+\n\n");
        printf("                             +-------------------------------------------------------------+\n");
        printf("                             |  Select Shift                                               |\n");
        printf("                             +-------------------------------------------------------------+\n");
        printf("                             |  [ 1 ]  Morning                                             |\n");
        printf("                             +-------------------------------------------------------------+\n");
        printf("                             |  [ 2 ]  Afternoon                                           |\n");
        printf("                             +-------------------------------------------------------------+\n\n");
        

        choice = getch();
        system("cls");

        if (choice=='9') goto SchedMenu;

        if (choice=='1') strcpy(shiftLocal, "M");
        else if (choice=='2') strcpy(shiftLocal, "N");
        else {
            printf("                             +-------------------------------------------------------------+\n");
            printf("                             |                  INVALID INPUT — TRY AGAIN                  |\n");
            printf("                             +-------------------------------------------------------------+\n\n");
            Sleep(1000);
            goto ShiftMenu;
        }
    }

    /* ================================================================
       GENERATE + SAVE
       ================================================================ */
    strcpy(scheduleType, scheduleTypeLocal);
	generateSection(sectionName, yearInt, scheduleTypeLocal, shiftLocal);
    saveScheduleToFile(sectionName);

    /* ================================================================
   CONFIRMATION
   ================================================================ */
ConfirmMenu:
    system("cls");

    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |                        PHILTECH PORTAL                      |\n");
    printf("                             |                           ENROLLMENT                        |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             | Name: %-20s                          [9] Back |\n", Fullname);
    printf("                             +-------------------------------------------------------------+\n\n");

    printf("                                Dear: %s\n\n", Fullname);
    printf("                                Thank you for enrolling with us! We are delighted to have you.\n\n");
    printf("                                Enrollment details saved successfully.\n\n");

    printf("                                To complete your enrollment, visit our main office:\n");
    printf("                                  1. Payment: P1,500.00 Reservation Fee\n");
    printf("                                  2. Submit required documents\n\n");

    printf("                                Required Documents:\n\n");

    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |  QTY | DETAILS                                              |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |   1  | Form 138 (Original)                                 |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |   4  | Form 138 (Photocopy)                                |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |   1  | Good Moral (Original)                               |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |   4  | Good Moral (Photocopy)                              |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |   1  | TOR/137 (Original)                                  |\n");
    printf("                             +-------------------------------------------------------------+\n");
    printf("                             |   4  | TOR/137 (Photocopy)                                 |\n");
    printf("                             +-------------------------------------------------------------+\n\n");

    printf("                                Press any key to return to dashboard...\n");

    choice = getch();
    system("cls");

    if (choice == '9') {
    	loading_screen();
        goToDashboard();
        return;
    }
	
	loading_screen();
    goToDashboard();
    return;
}



//STUDENT DASHBOARD
void studentDashboard() {
    char key;
	system("cls");

    	printf("                             +------------------------------------------------------------------+\n");
        printf("                             |    Philippine Technological Institute of Science Arts and Trade  |\n");
        printf("                             |                         (PHILTECH) PORTAL                        |\n");
        printf("                             +------------------------------------------------------------------+\n");

    printf("\n\n");

    printf("                                             +------------------------------------+\n");
    printf("                                             |         [1] Account Profile        |\n");
    printf("                                             +------------------------------------+\n");

    printf("\n");

    printf("                                             +------------------------------------+\n");
    printf("                                             |         [2] View Schedule          |\n");
    printf("                                             +------------------------------------+\n");

    printf("\n");

    printf("                                             +------------------------------------+\n");
    printf("                                             |           [3] Enrollment           |\n");
    printf("                                             +------------------------------------+\n");

    printf("\n");


    printf("                                             +------------------------------------+\n");
    printf("                                             |             [4] About Us           |\n");
    printf("                                             +------------------------------------+\n");

    printf("\n");

    printf("                                             +------------------------------------+\n");
    printf("                                             |              [5] Logout            |\n");
    printf("                                             +------------------------------------+\n");

    

    key = getch();

        if (key == '1') {
            profileCard();
			return;
        }
        else if (key == '2') {
             viewSchedule();
			 return;
        }
        else if (key == '3') {
            enrollMent();
            return;
         }
          else if (key == '4') {
             aboutScreen();
			return;
         }
         else if (key == '5') {
           loading_screen();
            loginPage();
         }
         
	
}
// FACULTY DASHBOARD

void facultyDashboard() {
    system("cls");
    char key;

    printf("                             +------------------------------------------------------------------+\n");
    printf("                             |    Philippine Technological Institute of Science Arts and Trade  |\n");
    printf("                             |                         (PHILTECH) PORTAL                        |\n");
    printf("                             +------------------------------------------------------------------+\n\n");

    printf("                                             +------------------------------------+\n");
    printf("                                             |         [1] Account Profile        |\n");
    printf("                                             +------------------------------------+\n\n");

    printf("                                             +------------------------------------+\n");
    printf("                                             |          [2] Time IN / OUT         |\n");
    printf("                                             +------------------------------------+\n\n");

    printf("                                             +------------------------------------+\n");
    printf("                                             |           [3] My Schedule          |\n");
    printf("                                             +------------------------------------+\n\n");

    printf("                                             +------------------------------------+\n");
    printf("                                             |     [4] Class Attendance Record    |\n");
    printf("                                             +------------------------------------+\n\n");

    printf("                                             +------------------------------------+\n");
    printf("                                             |          [5] Student Record        |\n");
    printf("                                             +------------------------------------+\n\n");

    printf("                                             +------------------------------------+\n");
    printf("                                             |             [6] About Us           |\n");
    printf("                                             +------------------------------------+\n\n");

    printf("                                             +------------------------------------+\n");
    printf("                                             |              [7] Logout            |\n");
    printf("                                             +------------------------------------+\n\n");

    key = getch();

if (key == '1') {
    profileCard();
    return;
}
else if (key == '2') {
    attendancePage("Faculty");
    return;
}
else if (key == '3') {
    printFacultyList("Faculty");
    return;
}
else if (key == '4') {
teacherAttendanceMenu(); 
    return;
}
else if (key == '5') {
teacherSummaryMenu(); 

    return;
}
else if (key == '6') {
    aboutScreen();
    return;
}
else if (key == '7') {
    loading_screen();
    loginPage();
    return;
}

}

//REGISTRAR DASHBOARD

void registrarDashboard() {
    system("cls");
    char key;

    printf("                             +------------------------------------------------------------------+\n");
    printf("                             |    Philippine Technological Institute of Science Arts and Trade  |\n");
    printf("                             |                         (PHILTECH) PORTAL                        |\n");
    printf("                             +------------------------------------------------------------------+\n\n");

    printf("                                             +------------------------------------+\n");
    printf("                                             |         [1] Account Profile        |\n");
    printf("                                             +------------------------------------+\n\n");

    printf("                                             +------------------------------------+\n");
    printf("                                             |          [2] Time IN / OUT         |\n");
    printf("                                             +------------------------------------+\n\n");

    printf("                                             +------------------------------------+\n");
    printf("                                             |     [3] Class Attendance Record    |\n");
    printf("                                             +------------------------------------+\n\n");
    
    printf("                                             +------------------------------------+\n");
    printf("                                             |        [4] Faculty Schedules       |\n");
    printf("                                             +------------------------------------+\n\n");
    
     printf("                                             +------------------------------------+\n");
    printf("                                             |        [5]   Class  Schedules       |\n");
    printf("                                             +-------------------------------------+\n\n");

    printf("                                             +------------------------------------+\n");
    printf("                                             |          [6] Student Record        |\n");
    printf("                                             +------------------------------------+\n\n");

    printf("                                             +------------------------------------+\n");
    printf("                                             |             [7] About Us           |\n");
    printf("                                             +------------------------------------+\n\n");

    printf("                                             +------------------------------------+\n");
    printf("                                             |              [8] Logout            |\n");
    printf("                                             +------------------------------------+\n\n");

    key = getch();

if (key == '1') {
    profileCard();
    return;
}
else if (key == '2') {
    attendancePage("Registrar");
    return;
}
else if (key == '3') {
registrarSummaryMenu();
    return;
}
else if (key == '4') {
  printFacultyList("Registrar");
    return;
}
else if (key == '5') {
    displayAllStudentSchedules();
    return;
}
else if (key == '6') {
   manageSectionStudents();
    return;
}
else if (key == '7') {
    aboutScreen();
    return;
}
else if (key == '8') {
    loading_screen();
    loginPage();
    return;
}

}
// ADMIN DASHBOARD

void adminDashboard() {
    system("cls");
    char key;

    printf("                             +------------------------------------------------------------------+\n");
    printf("                             |    Philippine Technological Institute of Science Arts and Trade  |\n");
    printf("                             |                         (PHILTECH) PORTAL                        |\n");
    printf("                             +------------------------------------------------------------------+\n\n");

    printf("                                             +------------------------------------+\n");
    printf("                                             |         [1] Account Profile        |\n");
    printf("                                             +------------------------------------+\n\n");

    printf("                                             +------------------------------------+\n");
    printf("                                             |          [2] Time IN / OUT         |\n");
    printf("                                             +------------------------------------+\n\n");

    printf("                                             +------------------------------------+\n");
    printf("                                             |     [3] Class Attendance Record    |\n");
    printf("                                             +------------------------------------+\n\n");

    printf("                                             +------------------------------------+\n");
    printf("                                             |          [4] Student Record        |\n");
    printf("                                             +------------------------------------+\n\n");

    printf("                                             +------------------------------------+\n");
    printf("                                             |          [5] Class Schedules       |\n");
    printf("                                             +------------------------------------+\n\n");

    printf("                                             +------------------------------------+\n");
    printf("                                             |           [6] Faculty List         |\n");
    printf("                                             +------------------------------------+\n\n");

    printf("                                             +------------------------------------+\n");
    printf("                                             |             [7] About Us           |\n");
    printf("                                             +------------------------------------+\n\n");

    printf("                                             +------------------------------------+\n");
    printf("                                             |              [8] Logout            |\n");
    printf("                                             +------------------------------------+\n\n");

    key = getch();

if (key == '1') {
    profileCard();
    return;
}
else if (key == '2') {
   attendancePage("Admin");
    return;
}
else if (key == '3') {
    registrarSummaryMenu();
    return;
}
else if (key == '4') {
   manageSectionStudents();
    return;
}
else if (key == '5') {
  displayAllStudentSchedules();
    return;
}
else if (key == '6') {
    printFacultyList("Admin");
    return;
}
else if (key == '7') {
    aboutScreen();
    return;
}
else if (key == '8') {
    loading_screen();
    loginPage();
    return;
}

}



void goToDashboard() {
    if (strcmp(currentRole, "Student") == 0) {
        studentDashboard();
    }
    else if (strcmp(currentRole, "Faculty") == 0) {
        facultyDashboard();
    }
    else if (strcmp(currentRole, "Admin") == 0) {
        adminDashboard();
    }
    else if (strcmp(currentRole, "Registrar") == 0) {
        registrarDashboard();
    }
    else {
        printf("\n\nUnknown role detected.\n");
        getch();
    }
}

/* -------------------------------------------------------------------
   LOGIN PAGE - fully integrated with confirmed layout & coords
   (renamed to camelCase loginPage)
   ------------------------------------------------------------------- */
/* -------------------------------------------------------------------
   LOGIN PAGE - improved Tab/Space handling
   ------------------------------------------------------------------- */
void loginPage() {

    system("cls");

    char loginEmail[200];
    char loginPass[200];

    /* ================= HEADER ================= */
    printf("\n\n");
    printf("                             +------------------------------------------------------------------+\n");
    printf("                             |    Philippine Technological Institute of Science Arts and Trade  |\n");
    printf("                             |                         (PHILTECH) PORTAL                        |\n");
    printf("                             +------------------------------------------------------------------+\n");

    /* LOGIN TITLE BOX — widened + centered correctly */
    printf("                                                       +------------+\n");
    printf("                                                       |    LOGIN   |\n");
    printf("                                                       +------------+\n");

    /* EMAIL LABEL */
    printf("                             Email Address\n");

    /* EMAIL BOX — fixed + matched width */
    printf("                             +------------------------------------------------------------------+\n");
    printf("                             |                                                                  |\n");
    printf("                             +------------------------------------------------------------------+\n\n");

    /* PASSWORD LABEL */
    printf("                             Password\n");

    /* PASSWORD BOX — same width */
    printf("                             +------------------------------------------------------------------+\n");
    printf("                             |                                                                  |\n");
    printf("                             +------------------------------------------------------------------+\n");

    /* Instructions (will be replaced later on row 19–23) */
    printf("\n\n                                 Press [ENTER] to begin typing. Press [C] to create an account.\n\n");

    /* ============================================================
       BEFORE TYPING: WAIT FOR ENTER or C
       ============================================================ */

    showCursor(0);

    int key;

    while (1) {
        key = getch();

        if (key == 'c' || key == 'C') {
            loading_screen();
            createAccount();
            return;
        }

        if (key == 13) {   // ENTER pressed
            break;
        }
    }

    showCursor(1);

    /* CLEAR THE INSTRUCTION TEXT (rows 19–23) */
    for (int r = 19; r <= 23; r++) {
        gotoxy(0, r);
        for (int c = 0; c < 150; c++) printf(" "); // erase entire line
    }

    /* ================= EMAIL INPUT ================= */
    memset(loginEmail, 0, sizeof(loginEmail));
    clearBox(30, 11, 50);
    gotoxy(30, 11);

    int ch, idx = 0;

    while (1) {
        ch = getch();

        if (ch == 13) break;     // ENTER ? next field

        if (ch == 8 && idx > 0) {  // BACKSPACE
            idx--;
            loginEmail[idx] = 0;
            printf("\b \b");     // correct backspace handling
            continue;
        }

        if (ch == 0 || ch == 224) { getch(); continue; } // ignore arrows

        if (ch >= 32 && ch <= 126 && idx < sizeof(loginEmail)-1) {
            loginEmail[idx++] = ch;
            putchar(ch);
        }
    }

    loginEmail[idx] = 0;

    /* ================= PASSWORD INPUT (masked) ================= */
    memset(loginPass, 0, sizeof(loginPass));
    clearBox(30, 16, 50);
    gotoxy(30, 16);

    idx = 0;

    while (1) {
        ch = getch();

        if (ch == 13) break; // ENTER ? verify login

        if (ch == 8 && idx > 0) {  // BACKSPACE
            idx--;
            loginPass[idx] = 0;
            printf("\b \b");
            continue;
        }

        if (ch == 0 || ch == 224) { getch(); continue; }

        if (ch >= 32 && ch <= 126 && idx < sizeof(loginPass)-1) {
            loginPass[idx++] = ch;
            putchar('*');
        }
    }

    loginPass[idx] = 0;

    /* ================= VERIFY LOGIN ================= */
    char role[20], major[50];

    if (verifyLogin(loginEmail, loginPass, role, major)) {

        system("cls");
        printf("\n\n\n\t\tLogin successful! Welcome, %s.\n\n", loginEmail);
        strcpy(currentRole, role);
        loading_screen();
        goToDashboard();


    } else {

        /* WRONG CREDENTIALS — SHOW MESSAGE IN THE SAME CLEARED AREA (19–23) */

        for (int r = 19; r <= 23; r++) {
            gotoxy(0, r);
            for (int c = 0; c < 150; c++) printf(" ");
        }

        gotoxy(0, 19);
        printf("                                 Invalid Email or Password!");
        gotoxy(0, 20);
        printf("                                 Press [F] to reset your password.");
        gotoxy(0, 21);
        printf("                                 Press any other key to try again...");

        int k = getch();

        if (k == 'f' || k == 'F') {
            forgotPasswordPage();
            return;
        }

        loginPage();
    }
}



/* -------------------------------------------------------------------
   File helpers: save, check, verify
   Format:
   email|password|role|first|middle|last|birthdate|sex|contact|house|barangay|city|province|fullAddress|programChoice
   ------------------------------------------------------------------- */

void saveUserToFile() {
    FILE *file = fopen("users.txt", "a");
    if (!file) {
        printf("(System): Cannot open users.txt!\n");
        getch(); // pause so you can see error
        return;
    }

    fprintf(file,
        "%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s\n",
        email, passW, roleChoice, firstN, middleN, lastN,
        birthD, sex, contactN, house, barangay, city,
        province, addressFull, programChoice,
        (strcmp(roleChoice, "Faculty") == 0 ? facultyType : "N/A")
    );

    fclose(file);
    printf("(System): User saved successfully!\n");
    getch();
}


/* Return 1 if email exists in users.txt, else 0 */
int checkUserExists(const char *checkEmail) {
    FILE *file = fopen("users.txt", "r");
    if (!file) return 0;

    char line[2048];
    char storedEmail[300];

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%299[^|]", storedEmail);
        if (strcmp(storedEmail, checkEmail) == 0) {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

/* Verify login by email + password; on success load user globals */
int verifyLogin(const char *loginEmail, const char *password,
                char *roleOut, char *programOut) {

    FILE *file = fopen("users.txt", "r");
    if (!file) return 0;

    char line[4096];

    /* Temporary storage for ALL 16 fields */
    char storedEmail[300], storedPassword[200], storedRole[100];
    char storedFirst[100], storedMiddle[100], storedLast[100];
    char storedBirth[20], storedSex[20], storedContact[30];
    char storedHouse[80], storedBarangay[50], storedCity[50];
    char storedProvince[50], storedAddress[300], storedProgram[200];
    char storedEmploymentType[20];

    while (fgets(line, sizeof(line), file)) {

        /* Parse EXACTLY 16 fields */
        int items = sscanf(line,
            "%299[^|]|%199[^|]|%99[^|]|"
            "%99[^|]|%99[^|]|%99[^|]|"
            "%19[^|]|%19[^|]|%29[^|]|"
            "%79[^|]|%49[^|]|%49[^|]|"
            "%49[^|]|%299[^|]|%199[^|]|%49[^\n]",
            storedEmail, storedPassword, storedRole,
            storedFirst, storedMiddle, storedLast,
            storedBirth, storedSex, storedContact,
            storedHouse, storedBarangay, storedCity,
            storedProvince, storedAddress, storedProgram,
            storedEmploymentType
        );

        /* If line is malformed, skip it */
        if (items != 16)
            continue;

        /* Match login credentials */
        if (strcmp(storedEmail, loginEmail) == 0 &&
            strcmp(storedPassword, password) == 0) {

            /* Transfer all data into global variables */
            strcpy(email, storedEmail);
            strcpy(passW, storedPassword);
            strcpy(roleChoice, storedRole);
            strcpy(firstN, storedFirst);
            strcpy(middleN, storedMiddle);
            strcpy(lastN, storedLast);
            strcpy(birthD, storedBirth);
            strcpy(sex, storedSex);
            strcpy(contactN, storedContact);
            strcpy(house, storedHouse);
            strcpy(barangay, storedBarangay);
            strcpy(city, storedCity);
            strcpy(province, storedProvince);
            strcpy(addressFull, storedAddress);
            strcpy(programChoice, storedProgram);

            /* Store faculty type if applicable */
            if (strcmp(roleChoice, "Faculty") == 0)
                strcpy(facultyType, storedEmploymentType);
            else
                strcpy(facultyType, "N/A");

            snprintf(Fullname, sizeof(Fullname), "%s, %s", lastN, firstN);

            if (roleOut)     strcpy(roleOut, storedRole);
            if (programOut)  strcpy(programOut, storedProgram);

            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

/* simple wrapper used by login page */
int verifyPassword(const char *loginEmail, const char *password) {
    char dummyRole[200], dummyMajor[200];
    return verifyLogin(loginEmail, password, dummyRole, dummyMajor);
}

/* keep the sample section checker (unchanged) */
int checkSection(char section[]) {
    FILE *file;
    char line[100];
    int found = 0;  // 0 means not found, 1 means found
    
    // Open the file
    file = fopen("sections.txt", "r");
    
    // Check if file opened successfully
    if (file == NULL) {
        printf("\n                                Cannot open sections file!\n");
        return 0;
    }
    
    // Read the file line by line
    while(fgets(line, 100, file)) {
        // Remove the newline character at the end of line
        line[strcspn(line, "\n")] = 0;
        
        // Compare the section with current line
        if(strcmp(section, line) == 0) {
            found = 1;  // Section found!
            break;
        }
    }
    
    // Close the file
    fclose(file);
    
    return found;
}





/* -------------------------------------------------------------------
   MAIN
   ------------------------------------------------------------------- */
int main(void) {
    showCursor(1);
    loginPage();
    return 0;
}
