#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/* Note structure */
#define MAX_NOTE 300

typedef struct
{
    int id;
    char text[MAX_NOTE];
    time_t created_at;
} Note;

/* Task structure */
#define MAX_TITLE 100

typedef struct
{
    int id;
    char title[MAX_TITLE];
    int completed;
    time_t deadline;
} Task;

/* Student timetable structure */
#define MAX_SUBJECT 80
#define MAX_ROOM 40

typedef struct
{
    int id;
    int day;  /* 0 = Monday, 6 = Sunday */
    char subject[MAX_SUBJECT];
    char start_time[6];
    char end_time[6];
    char room[MAX_ROOM];
} ClassEntry;

/* Main application */
void show_welcome(void);
void show_help(void);
void show_time(void);
void calculate(void);
void process_command(char command[]);

/* Notes */
void add_note(void);
void view_notes(void);
void search_notes(void);
void delete_note(void);

/* Tasks */
void add_task(void);
void view_tasks(void);
void complete_task(void);
void delete_task(void);
void task_stats(void);
void show_deadlines(void);
void show_overdue_tasks(void);

/* Timetable */
void add_class(void);
void show_timetable(void);
void show_today(void);
void delete_class(void);

#endif