#include "common.h"

/* Find the next unique note ID */
static int get_next_note_id(void)
{
    FILE *fp;
    Note note;
    int max_id = 0;

    fp = fopen("notes.dat", "rb");

    if (fp == NULL)
    {
        return 1;
    }
    while (fread(&note, sizeof(Note), 1,fp) == 1)
    {
        if(note.id > max_id)
        {
            max_id = note.id;
        }
    }

    fclose(fp);
    return max_id + 1;
}

/* Add a new note to JARVIS memory */
void add_note(void)
{
    FILE *fp;
    Note note;

    note.id = get_next_note_id();

    printf("\nEnter your note: ");

    if(fgets(note.text, sizeof(note.text),stdin) == NULL)
    {
        printf("JARVIS: Could not read your note.\n");
        return;
    }

    note.text[strcspn(note.text, "\n")] = '\0';

    if(note.text[0] == '\0')
    {
        printf("JARVIS: Empty notes cannot be saved.\n");
        return;
    }

    note.created_at =  time(NULL);

    fp = fopen("notes.dat", "ab");

    if(fp == NULL)
    {
        printf("JARVIS: Could not open memory file.\n");
        return;
    }

    if(fwrite(&note, sizeof(Note), 1,fp) != 1)
    {
        printf("JARVIS: Failed to save your note.\n");
        fclose(fp);
        return;
    }

    fclose(fp);

    printf("JARVIS: Note saved successfully!\n");
    printf("Note ID: %d\n", note.id);
}

/* Display all saved notes */
void view_notes(void)
{
    FILE *fp;
    Note note;
    int found = 0;

    fp = fopen("notes.dat", "rb");

    if(fp == NULL)
    {
        printf("JARVIS: No saved notes found yet.\n");
        return;
    }

    printf("\n=============== JARVIS MEMORY ===============\n");
    
    while(fread(&note, sizeof(Note), 1, fp) == 1)
    {
        printf("\nNote ID: %d\n", note.id);
        printf("Content: %s\n", note.text);

        found = 1;
    }
    if(!found)
    {
        printf("Your memory is currently empty.\n");
    }

    fclose(fp);
    printf("==============================================\n");
}

/* Search saved notes by keyword */
void search_notes(void)
{
    FILE *fp;
    Note note;
    char keyword[100];
    int found = 0;

    printf("Enter keyword to search: ");

    if(fgets(keyword, sizeof(keyword), stdin) == NULL)
    {
        printf("JARVIS: Could not read your keyword.\n");
        return;
    }

    keyword[strcspn(keyword, "\n")] = '\0';

    if(keyword[0] == '\0')
    {
        printf("JARVIS: Please enter a keyword.\n");
        return;
    }

    fp = fopen("notes.dat", "rb");

    if(fp == NULL)
    {
        printf("JARVIS: No saved notes found.\n");
        return;
    }

    printf("\n=============== SEARCH RESULTS ===============\n");

    while (fread(&note, sizeof(Note), 1,fp) == 1)
    {
        if(strstr(note.text, keyword) != NULL)
        {
            printf("Note ID: %d\n", note.id);
            printf("Consent: %s\n\n", note.text);
            found = 1;
        }
    }
    
    if(!found)
    {
        printf("JARVIS: No matching notes found.\n");
    }

    fclose(fp);
}

/* Delete a note by ID */
void delete_note(void)
{
    FILE *fp;
    FILE *temp;
    Note note;
    int target_id;
    int found = 0;
    int ch;
    char confirm;

    printf("\n========== DELETE NOTE ==========\n");
    printf("Enter note ID to delete: ");

    if (scanf("%d", &target_id) != 1)
    {
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;
        printf("JARVIS: Invalid note ID.\n");
        return;
    }

    while ((ch = getchar()) != '\n' && ch != EOF)
        ;

    fp = fopen("notes.dat", "rb");

    if (fp == NULL)
    {
        printf("JARVIS: No notes file found.\n");
        return;
    }

    /* Find the requested note */
    while (fread(&note, sizeof(Note), 1, fp) == 1)
    {
        if (note.id == target_id)
        {
            found = 1;
            printf("Note ID: %d\nNote: %s\n",
                   note.id, note.text);
            break;
        }
    }

    if (ferror(fp))
    {
        fclose(fp);
        printf("JARVIS: Error reading notes.\n");
        return;
    }

    fclose(fp);

    if (!found)
    {
        printf("JARVIS: Note ID %d not found.\n", target_id);
        return;
    }

    printf("Delete this note? (y/n): ");

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

    fp = fopen("notes.dat", "rb");
    if (fp == NULL)
    {
        printf("JARVIS: Could not reopen notes file.\n");
        return;
    }

    temp = fopen("notes.tmp", "wb");
    if (temp == NULL)
    {
        fclose(fp);
        printf("JARVIS: Could not create temporary file.\n");
        return;
    }

    found = 0;

    while (fread(&note, sizeof(Note), 1, fp) == 1)
    {
        if (note.id == target_id)
        {
            found = 1;
            continue;
        }

        if (fwrite(&note, sizeof(Note), 1, temp) != 1)
        {
            fclose(fp);
            fclose(temp);
            remove("notes.tmp");
            printf("JARVIS: Error writing temporary file.\n");
            return;
        }
    }

    if (ferror(fp) || !found)
    {
        fclose(fp);
        fclose(temp);
        remove("notes.tmp");
        printf("JARVIS: Could not safely delete the note.\n");
        return;
    }

    if (fclose(fp) != 0 || fclose(temp) != 0)
    {
        remove("notes.tmp");
        printf("JARVIS: Error closing files.\n");
        return;
    }

    /* Replace the original file */
    if (remove("notes.dat") != 0)
    {
        remove("notes.tmp");
        printf("JARVIS: Could not replace notes file.\n");
        return;
    }

    if (rename("notes.tmp", "notes.dat") != 0)
    {
        printf("JARVIS: Rename failed. Your notes are in notes.tmp.\n");
        return;
    }

    printf("JARVIS: Note deleted successfully!\n");
}