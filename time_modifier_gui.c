#define _DEFAULT_SOURCE

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <string.h>

#define IOCONTROL_CMD _IOW(0xCD, 0x19, char*)

struct path_data 
{
	char* path;
	size_t size;
	struct timespec time;
	char* command;
}; 

int validate_date(const char *dateInput, struct path_data *data);
void refresh_all_display_content(char *path);
void commit_all_changes(char *file_path);
int interact_with_kernel_module(struct path_data *data);

GtkBuilder      *builder; 
GtkWidget       *window;
GtkWidget       *file_chooser;
GtkWidget       *curr_date_changed_display;
GtkWidget       *new_date_changed_entry;
GtkWidget       *curr_date_modified_display;
GtkWidget       *new_date_modified_entry;
GtkWidget       *curr_date_accessed_display;
GtkWidget       *new_date_accessed_entry;
GtkWidget       *ok_button;
GtkWidget       *apply_button;

// method called when the user selects a file via the file chooser
void on_file_chooser_file_set() {
    // load the current date changed/mod/accessed for the selected file
    GFile *chosen_file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(file_chooser));
    refresh_all_display_content(g_file_get_path (chosen_file));

    // turn on the OK and APPLY buttons, if they aren't already on
    gtk_widget_set_sensitive (ok_button, 1);
    gtk_widget_set_sensitive (apply_button, 1);
}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);
 
    builder = gtk_builder_new();
    gtk_builder_add_from_file (builder, "gui_layout.xml", NULL);
 
    window = GTK_WIDGET(gtk_builder_get_object(builder, "application_window"));
    gtk_builder_connect_signals(builder, NULL);
    
    file_chooser = GTK_WIDGET(gtk_builder_get_object(builder, "file_chooser"));
    curr_date_changed_display = GTK_WIDGET(gtk_builder_get_object(builder, "curr_date_changed_display"));
    new_date_changed_entry = GTK_WIDGET(gtk_builder_get_object(builder, "new_date_changed_entry"));
    curr_date_modified_display = GTK_WIDGET(gtk_builder_get_object(builder, "curr_date_modified_display"));
    new_date_modified_entry = GTK_WIDGET(gtk_builder_get_object(builder, "new_date_modified_entry"));
    curr_date_accessed_display = GTK_WIDGET(gtk_builder_get_object(builder, "curr_date_accessed_display"));
    new_date_accessed_entry = GTK_WIDGET(gtk_builder_get_object(builder, "new_date_accessed_entry"));
    ok_button = GTK_WIDGET(gtk_builder_get_object(builder, "ok_button"));
    apply_button = GTK_WIDGET(gtk_builder_get_object(builder, "apply_button"));

    g_object_unref(builder);
 
    gtk_widget_show(window);                
    gtk_main();
 
    return 0;
}
 
// called when window is closed
void on_window_main_destroy()
{
    gtk_main_quit();
}

void on_ok_button_press()
{
    GFile *chosen_file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(file_chooser));
    commit_all_changes(g_file_get_path (chosen_file));
    gtk_main_quit();
    printf("OK pressed");
}

void on_apply_button_clicked() 
{
    GFile *chosen_file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(file_chooser));
    commit_all_changes(g_file_get_path (chosen_file)); 
    refresh_all_display_content(g_file_get_path (chosen_file));
    printf("apply clicked");
}

void refresh_all_display_content(char *path)
{
    struct stat attr;
    stat(path, &attr);

    gtk_entry_set_text (GTK_ENTRY(curr_date_changed_display), ctime(&attr.st_ctime));
    gtk_entry_set_text (GTK_ENTRY(curr_date_modified_display), ctime(&attr.st_mtime));
    gtk_entry_set_text (GTK_ENTRY(curr_date_accessed_display), ctime(&attr.st_atime));
}

int validate_date(const char *dateInput, struct path_data *data)
{
    struct tm tinfo;
    struct timespec ts;  
 
    // Get the third parameter and transfer its data to a time struct
    sscanf(dateInput, "%u.%u.%u-%u:%u:%u:%ld", &tinfo.tm_mday, &tinfo.tm_mon,
	   &tinfo.tm_year, &tinfo.tm_hour, &tinfo.tm_min, &tinfo.tm_sec,
           &ts.tv_nsec);
    tinfo.tm_year -= 1900; //relative time
    tinfo.tm_mon -= 1;
    data->time.tv_sec = mktime(&tinfo);
    data->time.tv_nsec = ts.tv_nsec;

    if(data->time.tv_sec < 0 || data->time.tv_nsec < 0 || data->time.tv_nsec >= 100000000L){
        fprintf(stderr, "HIT ERROR WITH tv_sec: %ld\n", data->time.tv_sec);
	fprintf(stderr, "AND tv_nsec: %ld\n", data->time.tv_sec);
	return 0;
    }

    return 1;
}

void commit_all_changes(char *file_path)
{
    int shouldPerformOperation; //bool that tells us whether we should attempt to modify a certain property
    struct path_data data; // stuct that will hold data to pass to kernel module
    char inputBoxCommand[3];

    
    data.path = file_path;
    data.size = strlen(data.path) + 1;
    
    const char *DC_new_time_string = gtk_entry_get_text (GTK_ENTRY(new_date_changed_entry));
    shouldPerformOperation = validate_date(DC_new_time_string, &data);
    if (shouldPerformOperation)
    {
        strncpy(inputBoxCommand,"DC",sizeof inputBoxCommand);
        data.command = inputBoxCommand;
        // perform last accessed date change
        interact_with_kernel_module (&data);
    } 
    else
    {
        fprintf(stderr, "Invalid time: '%s'\n", DC_new_time_string);
    }

    const char *LM_new_time_string = gtk_entry_get_text (GTK_ENTRY(new_date_modified_entry));
    shouldPerformOperation = validate_date(LM_new_time_string, &data);
    if (shouldPerformOperation)
    {
	strncpy(inputBoxCommand,"LM",sizeof inputBoxCommand);
        data.command = inputBoxCommand;
        // perform last modified date change
        interact_with_kernel_module (&data);
    } 
    else
    {
        fprintf(stderr, "Invalid time: '%s'\n", LM_new_time_string);
    }

    const char *DA_new_time_string = gtk_entry_get_text (GTK_ENTRY(new_date_accessed_entry));
    shouldPerformOperation = validate_date(DA_new_time_string, &data);
    if (shouldPerformOperation)
    {
	strncpy(inputBoxCommand,"DA",sizeof inputBoxCommand);
        data.command = inputBoxCommand;
        // perform last accessed date change
        interact_with_kernel_module (&data);
    } 
    else
    {
        fprintf(stderr, "Invalid time: '%s'\n", DA_new_time_string);
    }
}

int interact_with_kernel_module (struct path_data *data) 
{
    int file = open("/dev/timem", O_RDWR);	// Open driver file
	
    // Check for error from opening file
    if(file < 0)
    {
        fprintf(stderr, "Could not open driver's device\n");
        return 0;
    }

    if(ioctl(file, IOCONTROL_CMD, data) < 0)
    {
        fprintf(stderr, "Failed to change time of '%s': %s\n", data->path, strerror(errno));
        close(file);
        return 0;
    }

    close(file);
    return 1;
}
