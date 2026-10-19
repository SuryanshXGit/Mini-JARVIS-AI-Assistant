#include "common.h"
/* Display the welcome message when JARVIS starts */
void show_welcome(void)
{
    printf("\n");
    printf("====================================\n");
    printf("       WELCOME TO MINI JARVIS       \n");
    printf("====================================\n");
    printf("Your personal C programming assistant\n");
    printf("Type 'help' to see available commands.\n");
    printf("Type 'exit' to close JARVIS.\n");
}


/* Display all available commands */
void show_help(void)
{
    printf("\n========== JARVIS COMMANDS ==========\n");

    printf("\n--- General Commands ---\n");
    printf("hello          - Display a greeting\n");
    printf("time           - Display current date and time\n");
    printf("calc           - Open the calculator\n");
    printf("system         - Display system information\n");
    printf("help           - Display this help menu\n");
    printf("exit           - Close JARVIS\n");

    printf("\n--- Notes ---\n");
    printf("note           - Add a note\n");
    printf("notes          - View all notes\n");
    printf("search         - Search notes\n");
    printf("deletenote     - Delete a note\n");

    printf("\n--- Tasks ---\n");
    printf("addtask        - Add a task\n");
    printf("tasks          - View all tasks\n");
    printf("complete       - Mark a task as completed\n");
    printf("deletetask     - Delete a task\n");
    printf("taskstats      - Show task statistics\n");
    printf("deadlines      - Show upcoming deadlines\n");
    printf("overdue        - Show overdue tasks\n");

    printf("\n--- Student Timetable ---\n");
    printf("addclass       - Add a timetable entry\n");
    printf("timetable      - View the weekly timetable\n");
    printf("today          - View today's classes\n");
    printf("deleteclass    - Delete a timetable entry\n");

    printf("\n====================================\n");
}


/* Display the current date and time */
void show_time(void)
{
    time_t current_time;
    struct tm *local_time;

    /* Get the current system time */
    current_time = time(NULL);

    /* Convert it to local date and time */
    local_time = localtime(&current_time);

    if (local_time == NULL)
    {
        printf("Unable to get the current time.\n");
        return;
    }

    printf("\nCurrent date and time: ");

    /* Format the date and time for display */
    printf("%02d-%02d-%04d ",
           local_time->tm_mday,
           local_time->tm_mon + 1,
           local_time->tm_year + 1900);

    printf("%02d:%02d:%02d\n",
           local_time->tm_hour,
           local_time->tm_min,
           local_time->tm_sec);
}


/* A simple calculator supporting +, -, *, and / */
void calculate(void)
{
    double number1;
    double number2;
    double result;
    char operator;

    printf("\n========== CALCULATOR ==========\n");

    /* Ask for the first number */
    printf("Enter first number: ");

    if (scanf("%lf", &number1) != 1)
    {
        printf("Invalid number.\n");

        /* Clear invalid input from the input buffer */
        while (getchar() != '\n')
        {
        }

        return;
    }

    /* Ask for the operator */
    printf("Enter operator (+, -, *, /): ");

    if (scanf(" %c", &operator) != 1)
    {
        printf("Invalid operator input.\n");
        while (getchar() != '\n')
        {
        }
        return;
    }

    /* Ask for the second number */
    printf("Enter second number: ");

    if (scanf("%lf", &number2) != 1)
    {
        printf("Invalid number.\n");
        while (getchar() != '\n')
        {
        }
        return;
    }

    /* Clear the remaining newline so fgets works correctly */
    while (getchar() != '\n')
    {
    }

    /* Perform the selected calculation */
    switch (operator)
    {
        case '+':
            result = number1 + number2;
            break;

        case '-':
            result = number1 - number2;
            break;

        case '*':
            result = number1 * number2;
            break;

        case '/':
            /* Prevent division by zero */
            if (number2 == 0)
            {
                printf("Error: Division by zero is not allowed.\n");
                return;
            }

            result = number1 / number2;
            break;

        default:
            printf("Invalid operator.\n");
            return;
    }

    printf("Result: %.2f\n", result);
}


/* Remove leading and trailing spaces from a command */
static void clean_command(char command[])
{
    size_t length;
    size_t start = 0;

    /* Find the command's current length */
    length = strlen(command);

    /* Remove trailing whitespace */
    while (length > 0 &&
           isspace((unsigned char)command[length - 1]))
    {
        command[length - 1] = '\0';
        length--;
    }

    /* Find the first non-space character */
    while (command[start] != '\0' &&
           isspace((unsigned char)command[start]))
    {
        start++;
    }

    /* Move the cleaned command to the beginning */
    if (start > 0)
    {
        memmove(command, command + start,
                strlen(command + start) + 1);
    }

    /* Convert the command to lowercase */
    for (size_t i = 0; command[i] != '\0'; i++)
    {
        command[i] = (char)tolower((unsigned char)command[i]);
    }
}


/* Process a command entered by the user */
void process_command(char command[])
{
    /* Clean spaces and standardize capitalization */
    clean_command(command);

    /* Ignore an empty command */
    if (command[0] == '\0')
    {
        return;
    }

    /* General commands */
    if (strcmp(command, "help") == 0)
    {
        show_help();
    }
    else if (strcmp(command, "hello") == 0)
    {
        printf("Hello! I am Mini JARVIS. How can I help you?\n");
    }
    else if (strcmp(command, "time") == 0)
    {
        show_time();
    }
    else if (strcmp(command, "calc") == 0 ||
             strcmp(command, "calculator") == 0)
    {
        calculate();
    }
    else if (strcmp(command, "system") == 0)
    {
        printf("\nMini JARVIS System Information\n");
        printf("Platform: Windows-compatible C application\n");
        printf("Language: C\n");
        printf("Storage: Local data files\n");
    }

    /* Notes module */
    else if (strcmp(command, "note") == 0 ||
             strcmp(command, "addnote") == 0)
    {
        add_note();
    }
    else if (strcmp(command, "notes") == 0 ||
             strcmp(command, "viewnotes") == 0)
    {
        view_notes();
    }
    else if (strcmp(command, "search") == 0 ||
             strcmp(command, "searchnote") == 0)
    {
        search_notes();
    }
    else if (strcmp(command, "deletenote") == 0)
    {
        delete_note();
    }

    /* Tasks module */
    else if (strcmp(command, "addtask") == 0)
    {
        add_task();
    }
    else if (strcmp(command, "tasks") == 0 ||
             strcmp(command, "viewtasks") == 0)
    {
        view_tasks();
    }
    else if (strcmp(command, "complete") == 0 ||
             strcmp(command, "completetask") == 0)
    {
        complete_task();
    }
    else if (strcmp(command, "deletetask") == 0)
    {
        delete_task();
    }
    else if (strcmp(command, "taskstats") == 0)
    {
        task_stats();
    }
    else if (strcmp(command, "deadlines") == 0)
    {
        show_deadlines();
    }
    else if (strcmp(command, "overdue") == 0)
    {
        show_overdue_tasks();
    }

    /* Student timetable module */
    else if (strcmp(command, "addclass") == 0)
    {
        add_class();
    }
    else if (strcmp(command, "timetable") == 0)
    {
        show_timetable();
    }
    else if (strcmp(command, "today") == 0)
    {
        show_today();
    }
    else if (strcmp(command, "deleteclass") == 0)
    {
        delete_class();
    }

    /* Handle commands that are not recognized */
    else
    {
        printf("Sorry, I don't recognize that command.\n");
        printf("Type 'help' to see the available commands.\n");
    }
}


/* Main function: the program starts here */
int main(void)
{
    char command[200];

    /* Display the welcome screen */
    show_welcome();

    /* Keep accepting commands until the user exits */
    while (1)
    {
        printf("\nJARVIS > ");

        /* Read a complete line of user input */
        if (fgets(command, sizeof(command), stdin) == NULL)
        {
            printf("\nInput ended. Closing JARVIS.\n");
            break;
        }

        /* Remove the newline added by fgets */
        command[strcspn(command, "\r\n")] = '\0';

        /* Exit the program when requested */
        clean_command(command);

        if (strcmp(command, "exit") == 0 ||
            strcmp(command, "quit") == 0)
        {
            printf("JARVIS: Goodbye! Have a great day.\n");
            break;
        }

        /* Process all other commands */
        process_command(command);
    }

    return 0;
}