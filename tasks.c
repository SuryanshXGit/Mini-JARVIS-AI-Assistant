#include "common.h"

#define TASK_FILE "tasks.dat"

/* Convert YYYY-MM-DD into a deadline at the end of that day */
static int parse_deadline(const char input[], time_t *deadline)
{
    int year, month, day;
    char extra;
    struct tm date = {0};
    struct tm *check;
    time_t value;

    if(sscanf(input, "%d-%d-%d%c", &year,&month,&day, &extra) != 3)
    {
        return 0;
    }

    date.tm_year = year - 1900;
    date.tm_mon = month - 1;
    date.tm_mday = day;
    date.tm_hour = 23;
    date.tm_min = 59;
    date.tm_sec = 59;
    date.tm_isdst = -1;

    value = mktime(&date);

    if(value == (time_t)-1)
    return 0;

    check = localtime(&value);

    if(check == NULL ||
    check->tm_year != year - 1900 ||
    check->tm_mon != month - 1||
    check->tm_mday != day)
    {
        return 0;
    }

    *deadline = value;
    return 1;
}

/* Find the next available task ID */
static int next_task_id(void)
{
    FILE *fp = fopen(TASK_FILE, "rb");
    Task task;
    int max_id = 0;

    if(fp == NULL)
    return 1;

    while (fread(&task, sizeof(Task), 1, fp) == 1)
    {
        if(task.id > max_id)
        max_id = task.id;
    }
    
    fclose(fp);
    return max_id + 1;
}

/* Add a task */
void add_task(void)
{
    FILE *fp;
    Task task;
    char date_input[100];

    task.id = next_task_id();
    task.completed = 0;

    printf("Enter task title: ");

    if(fgets(task.title, sizeof(task.title), stdin) == NULL)
    {
        printf("JARVIS: Could not read task.\n");
        return;
    }

    task.title[strcspn(task.title, "\n")] = '\0';

    if(task.title[0] == '\0')
    {
        printf("JARVIS: Task title cannot be empty.\n");
        return;
    }

    printf("Enter deadline (YYYY-MM-DD), or 0 for a no deadline: ");

    if(fgets(date_input, sizeof(date_input), stdin) == NULL)
    {
        printf("JARVIS: Could not read deadline.\n");
        return;
    }

    date_input[strcspn(date_input, "\n")] = '\0';

    if(strcmp(date_input, "0") == 0)
    {
        task.deadline = 0;
    }
    else if(!parse_deadline(date_input, &task.deadline))
    {
        printf("JARVIS: Invalid date. Use YYYY-MM-DD.\n");
        return;
    }

    fp = fopen(TASK_FILE, "ab");

    if(fp == NULL)
    {
        printf("JARVIS: Cannot open task file.\n");
        return;
    }

    if(fwrite(&task, sizeof(Task), 1, fp) != 1)
    {
        printf("JARVIS: Failed to save task.\n");
        fclose(fp);
        return;
    }

    fclose(fp);

    printf("JARVIS: Task added successfully! ID: %d\n", task.id);

    if(task.deadline != 0)
    printf("Deadline saved: %s\n", date_input);
}

/* Display all tasks */
void view_tasks(void)
{
    FILE *fp = fopen(TASK_FILE, "rb");
    Task task;
    int found = 0;

    if(fp == NULL)
    {
        printf("JARVIS: No tasks found.\n");
        return;
    }

    printf("\n=============== JARVIS TASKS ===============\n");

    while(fread(&task, sizeof(Task), 1,fp) == 1) 
    {
        printf("\nID: %d\n", task.id);
        printf("Task: %s\n", task.title);
        printf("Status: %s\n", task.completed? "Completed": "Pending");
        found = 1;
    }

    if(!found)
    printf("No tasks saved yet.\n");

    fclose(fp);
}

/* Mark a task complete */
void complete_task(void)
{
    FILE *fp;
    Task task;
    int id;
    int found = 0;

    printf("Enter task ID to complete: ");

    if(scanf("%d", &id) != 1)
    {
        printf("JARVIS: Invalid task ID.\n");

        /* Clear invalid input */
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF)
        ;
        return;
    }

    /* Consume the rest of the input line */
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
    ;
    
    fp = fopen(TASK_FILE, "r+b");

    if(fp == NULL)
    {
        printf("JARVIS: No task file found.\n");
        return;
    }

    while (fread(&task, sizeof(Task), 1, fp) == 1)
    {
        if(task.id == id)
        {
            if(task.completed)
            {
                printf("JARVIS: Task is already completed.\n");
                found = 1;
                break;
            }

            task.completed = 1;

            if(fseek(fp, -(long)sizeof(Task), SEEK_CUR) != 0)
            {
                printf("JARVIS: Could not update task.\n");
                fclose(fp);
                return;
            }

            if(fwrite(&task, sizeof(Task), 1, fp) != 1)
            {
                printf("JARVIS: Could not save task.\n");
                fclose(fp);
                return;
            }

            printf("JARVIS: Task completed successfully!\n");
            found = 1;
            break;
        }
    }

    if(!found)
        printf("JARVIS: Task ID not found.\n");

    fclose(fp);
}

/* Display task statistics */
void task_stats(void)
{
    FILE *fp = fopen(TASK_FILE, "rb");
    Task task;
    int total = 0;
    int completed = 0;

    if(fp == NULL)
    {
        printf("JARVIS: No tasks recorded yet.\n");
        return;
    }

    while(fread(&task, sizeof(Task), 1, fp) == 1)
    {
        total++;

        if(task.completed)
        completed++;
    }

    fclose(fp);

    printf("\n=============== TASK STATISTICS ===============\n");
    printf("Total tasks: %d\n", total);
    printf("Completed:   %d\n", completed);
    printf("Pending:     %d\n", total - completed);
    printf("================================================\n");
}

/* Display tasks that have deadlines */
void show_deadlines(void)
{
    FILE *fp = fopen(TASK_FILE, "rb");
    Task task;
    struct tm *time_info;
    char date_buffer[30];
    int found = 0;

    if(fp == NULL)
    {
        printf("JARVIS: No tasks found.\n");
        return;
    }

    printf("\n=============== TASK DEADLINES ===============\n");

    while(fread(&task, sizeof(Task), 1 , fp) == 1)
    {
        if(task.deadline == 0)
        continue;

        time_info = localtime(&task.deadline);

        if(time_info == NULL)
        continue;

        if(strftime(date_buffer, sizeof(date_buffer), "%Y-%m-%d", time_info) == 0)
        continue;

        printf("\nID: %d\n", task.id);
        printf("Task: %s\n", task.title);
        printf("Deadline: %s\n", date_buffer);
        printf("Status: %s\n", task.completed ? "Completed" : "Pending");

        found = 1;
    }

    if(!found)
    printf("No tasks with deadlines found.\n");

    fclose(fp);
}

/* Display unfinished tasks past their deadlines */
void show_overdue_tasks(void)
{
    FILE *fp = fopen(TASK_FILE, "rb");
    Task task;
    time_t now = time(NULL);
    int found = 0;

    if(fp == NULL)
    {
        printf("JARVIS: No tasks found.\n");
        return;
    }

    printf("\n=============== OVERDUE TASKS ===============\n");

    while (fread(&task, sizeof(Task), 1, fp) == 1)
    {
        if(!task.completed &&
            task.deadline != 0 &&
            now > task.deadline)
            {
                printf("\nID: %d\n", task.id);
                printf("Task: %s\n", task.title);
                printf("Status: OVERDUE\n");
                found = 1;
            }
    }

    if(!found)
    printf("No overdue tasks. Great work!\n");

    fclose(fp);
}

/* Delete a task by ID */
void delete_task(void)
{
    FILE *fp;
    FILE *temp;
    Task task;
    int target_id;
    int found = 0;
    int ch;
    char confirm;

    printf("\n========== DELETE TASK ==========\n");
    printf("Enter task ID to delete: ");

    if (scanf("%d", &target_id) != 1)
    {
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;
        printf("JARVIS: Invalid task ID.\n");
        return;
    }

    while ((ch = getchar()) != '\n' && ch != EOF)
        ;

    fp = fopen("tasks.dat", "rb");

    if (fp == NULL)
    {
        printf("JARVIS: No tasks file found.\n");
        return;
    }

    /* Find the requested task */
    while (fread(&task, sizeof(Task), 1, fp) == 1)
    {
        if (task.id == target_id)
        {
            found = 1;
            printf("Task ID: %d\nTask: %s\nStatus: %s\n",
                   task.id, task.title,
                   task.completed ? "Completed" : "Pending");
            break;
        }
    }

    if (ferror(fp))
    {
        fclose(fp);
        printf("JARVIS: Error reading tasks.\n");
        return;
    }

    fclose(fp);

    if (!found)
    {
        printf("JARVIS: Task ID %d not found.\n", target_id);
        return;
    }

    printf("Delete this task? (y/n): ");

    if (scanf(" %c", &confirm) != 1)
    {
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;
        printf("JARVIS: Could not read confirmation.\n");
        return;
    }

    while ((ch = getchar()) != '\n' && ch != EOF)
        ;

    if (confirm != 'y' && confirm != 'Y')
    {
        printf("JARVIS: Deletion cancelled.\n");
        return;
    }

    fp = fopen("tasks.dat", "rb");
    if (fp == NULL)
    {
        printf("JARVIS: Could not reopen tasks file.\n");
        return;
    }

    temp = fopen("tasks.tmp", "wb");
    if (temp == NULL)
    {
        fclose(fp);
        printf("JARVIS: Could not create temporary file.\n");
        return;
    }

    found = 0;

    while (fread(&task, sizeof(Task), 1, fp) == 1)
    {
        if (task.id == target_id)
        {
            found = 1;
            continue;
        }

        if (fwrite(&task, sizeof(Task), 1, temp) != 1)
        {
            fclose(fp);
            fclose(temp);
            remove("tasks.tmp");
            printf("JARVIS: Error writing temporary file.\n");
            return;
        }
    }

    if (ferror(fp) || !found)
    {
        fclose(fp);
        fclose(temp);
        remove("tasks.tmp");
        printf("JARVIS: Could not safely delete the task.\n");
        return;
    }

    if (fclose(fp) != 0 || fclose(temp) != 0)
    {
        remove("tasks.tmp");
        printf("JARVIS: Error closing files.\n");
        return;
    }

    /* Replace the original file */
    if (remove("tasks.dat") != 0)
    {
        remove("tasks.tmp");
        printf("JARVIS: Could not replace tasks file.\n");
        return;
    }

    if (rename("tasks.tmp", "tasks.dat") != 0)
    {
        printf("JARVIS: Rename failed. Your tasks are in tasks.tmp.\n");
        return;
    }

    printf("JARVIS: Task deleted successfully!\n");
}