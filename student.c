#include "common.h"

#define TIMETABLE_FILE "timetable.dat"

static const char *day_names[] = {
    "Monday", "Tuesday", "Wednesday", "Thursday",
    "Friday", "Saturday", "Sunday"
};

/* Read a line safely */
static int read_line(const char *prompt, char *buffer, size_t size)
{
    int ch;
    size_t len;

    printf("%s", prompt);

    if (fgets(buffer, (int)size, stdin) == NULL)
        return 0;

    len = strlen(buffer);

    if (len > 0 && buffer[len - 1] == '\n')
    {
        buffer[len - 1] = '\0';
    }
    else
    {
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;
    }

    return 1;
}

/* Validate HH:MM time */
static int valid_time(const char *text)
{
    int hour, minute;
    char extra;

    if (strlen(text) != 5 ||
        sscanf(text, "%2d:%2d%c", &hour, &minute, &extra) != 2)
        return 0;

    return text[2] == ':' &&
           hour >= 0 && hour <= 23 &&
           minute >= 0 && minute <= 59 &&
           text[0] >= '0' && text[0] <= '9' &&
           text[1] >= '0' && text[1] <= '9' &&
           text[3] >= '0' && text[3] <= '9' &&
           text[4] >= '0' && text[4] <= '9';
}

/* Generate the next class ID */
static int next_class_id(void)
{
    FILE *fp = fopen(TIMETABLE_FILE, "rb");
    ClassEntry entry;
    int max_id = 0;

    if (fp == NULL)
        return 1;

    while (fread(&entry, sizeof(entry), 1, fp) == 1)
    {
        if (entry.id > max_id)
            max_id = entry.id;
    }

    fclose(fp);
    return max_id + 1;
}

/* Add a class */
void add_class(void)
{
    ClassEntry entry = {0};
    FILE *fp;
    int day_choice;
    int ch;

    printf("\n========== ADD A CLASS ==========\n");

    for (int i = 0; i < 7; i++)
        printf("%d. %s\n", i + 1, day_names[i]);

    printf("Choose day (1-7): ");

    if (scanf("%d", &day_choice) != 1)
    {
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;

        printf("JARVIS: Invalid day selection.\n");
        return;
    }

    /* Clear remaining input */
    while ((ch = getchar()) != '\n' && ch != EOF)
        ;

    /* Validate the selected day */
    if (day_choice < 1 || day_choice > 7)
    {
        printf("JARVIS: Choose a day from 1 to 7.\n");
        return;
    }

    /* IMPORTANT: Store Monday as 0, Tuesday as 1, etc. */
    entry.day = day_choice - 1;
    entry.id = next_class_id();

    if (!read_line("Subject name: ", entry.subject,
                   sizeof(entry.subject)) ||
        entry.subject[0] == '\0')
    {
        printf("JARVIS: Subject name is required.\n");
        return;
    }

    if (!read_line("Start time (HH:MM): ", entry.start_time,
                   sizeof(entry.start_time)) ||
        !valid_time(entry.start_time))
    {
        printf("JARVIS: Invalid start time. Use HH:MM.\n");
        return;
    }

    if (!read_line("End time (HH:MM): ", entry.end_time,
                   sizeof(entry.end_time)) ||
        !valid_time(entry.end_time))
    {
        printf("JARVIS: Invalid end time. Use HH:MM.\n");
        return;
    }

    if (strcmp(entry.end_time, entry.start_time) <= 0)
    {
        printf("JARVIS: End time must be after start time.\n");
        return;
    }

    if (!read_line("Room or location: ", entry.room,
                   sizeof(entry.room)))
    {
        printf("JARVIS: Could not read room.\n");
        return;
    }

    fp = fopen(TIMETABLE_FILE, "ab");

    if (fp == NULL)
    {
        printf("JARVIS: Could not open timetable file.\n");
        return;
    }

    if (fwrite(&entry, sizeof(entry), 1, fp) != 1)
    {
        printf("JARVIS: Could not save the class.\n");
        fclose(fp);
        return;
    }

    fclose(fp);

    printf("JARVIS: Class saved successfully! ID: %d\n", entry.id);
}

/* Display classes for a particular day */
static void show_day(int day)
{
    FILE *fp = fopen(TIMETABLE_FILE, "rb");
    ClassEntry entry;
    int found = 0;

    if (fp == NULL)
    {
        printf("JARVIS: No timetable found. Add a class first.\n");
        return;
    }

    printf("\n========== %s ==========\n", day_names[day]);

    while (fread(&entry, sizeof(entry), 1, fp) == 1)
    {
        if (entry.day == day)
        {
            printf("%s - %s | %s | Room: %s | ID: %d\n",
                   entry.start_time, entry.end_time,
                   entry.subject, entry.room, entry.id);
            found = 1;
        }
    }

    if (!found)
        printf("No classes scheduled.\n");

    fclose(fp);
}

/* Display the complete weekly timetable */
void show_timetable(void)
{
    FILE *fp = fopen(TIMETABLE_FILE, "rb");
    ClassEntry entry;
    int day;
    int found;

    if (fp == NULL)
    {
        printf("JARVIS: No timetable found. Use 'addclass' first.\n");
        return;
    }

    printf("\n========== WEEKLY TIMETABLE ==========\n");

    for (day = 0; day < 7; day++)
    {
        rewind(fp);
        found = 0;

        printf("\n--- %s ---\n", day_names[day]);

        while (fread(&entry, sizeof(entry), 1, fp) == 1)
        {
            if (entry.day == day)
            {
                printf("%s - %s | %s | Room: %s | ID: %d\n",
                       entry.start_time, entry.end_time,
                       entry.subject, entry.room, entry.id);
                found = 1;
            }
        }

        if (!found)
            printf("No classes scheduled.\n");
    }

    fclose(fp);
}

/* Display today's classes */
void show_today(void)
{
    time_t now = time(NULL);
    struct tm *current = localtime(&now);
    int day;

    if (current == NULL)
    {
        printf("JARVIS: Could not determine today's date.\n");
        return;
    }

    /* Convert Sunday=0 to Monday=0 */
    day = (current->tm_wday + 6) % 7;

    show_day(day);
}

/* Delete a class by ID */
void delete_class(void)
{
    FILE *fp;
    FILE *temp;
    ClassEntry entry;
    int target_id;
    int found = 0;
    int ch;
    char confirm;

    printf("\n========== DELETE CLASS ==========\n");
    printf("Enter the class ID to delete: ");

    if (scanf("%d", &target_id) != 1)
    {
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;
        printf("JARVIS: Invalid class ID.\n");
        return;
    }

    while ((ch = getchar()) != '\n' && ch != EOF)
        ;

    fp = fopen(TIMETABLE_FILE, "rb");

    if (fp == NULL)
    {
        printf("JARVIS: No timetable file found.\n");
        return;
    }

    temp = fopen("timetable.tmp", "wb");

    if (temp == NULL)
    {
        fclose(fp);
        printf("JARVIS: Could not create temporary file.\n");
        return;
    }

    /* Check whether the class exists */
    while (fread(&entry, sizeof(entry), 1, fp) == 1)
    {
        if (entry.id == target_id)
        {
            found = 1;
            printf("Class: %s | %s - %s | Room: %s\n",
                   entry.subject, entry.start_time,
                   entry.end_time, entry.room);
        }
    }

    if (!found)
    {
        fclose(fp);
        fclose(temp);
        remove("timetable.tmp");
        printf("JARVIS: Class ID %d was not found.\n", target_id);
        return;
    }

    printf("Are you sure you want to delete this class? (y/n): ");

    if (scanf(" %c", &confirm) != 1)
    {
        fclose(fp);
        fclose(temp);
        remove("timetable.tmp");
        printf("JARVIS: Could not read confirmation.\n");
        return;
    }

    while ((ch = getchar()) != '\n' && ch != EOF)
        ;

    if (confirm != 'y' && confirm != 'Y')
    {
        fclose(fp);
        fclose(temp);
        remove("timetable.tmp");
        printf("JARVIS: Deletion cancelled.\n");
        return;
    }

    /* Rewind and copy all other classes */
    rewind(fp);
    found = 0;

    while (fread(&entry, sizeof(entry), 1, fp) == 1)
    {
        if (entry.id == target_id)
        {
            found = 1;
            continue;
        }

        if (fwrite(&entry, sizeof(entry), 1, temp) != 1)
        {
            fclose(fp);
            fclose(temp);
            remove("timetable.tmp");
            printf("JARVIS: Error writing temporary file.\n");
            return;
        }
    }

    if (ferror(fp))
    {
        fclose(fp);
        fclose(temp);
        remove("timetable.tmp");
        printf("JARVIS: Error reading timetable file.\n");
        return;
    }

    if (fclose(fp) != 0)
    {
        fclose(temp);
        remove("timetable.tmp");
        printf("JARVIS: Error closing timetable file.\n");
        return;
    }

    if (fclose(temp) != 0)
    {
        remove("timetable.tmp");
        printf("JARVIS: Error closing temporary file.\n");
        return;
    }

    if (!found)
    {
        remove("timetable.tmp");
        printf("JARVIS: Class could not be deleted.\n");
        return;
    }

    /* Replace the original file */
    if (remove(TIMETABLE_FILE) != 0)
    {
        printf("JARVIS: Could not replace the timetable file.\n");
        printf("Your original timetable file has been preserved.\n");
        remove("timetable.tmp");
        return;
    }

    if (rename("timetable.tmp", TIMETABLE_FILE) != 0)
    {
        printf("JARVIS: Could not rename the temporary file.\n");
        printf("Recovery file: timetable.tmp\n");
        return;
    }

    printf("JARVIS: Class deleted successfully!\n");
}