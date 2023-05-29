#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define b_read_size 4

union argument_value_struct {
	char number_value;
	char* string_value;
	bool no_value; // we set this to bool because void is just funky and if we can set it to false then we can just check if its false and know that there is no argument
};
struct setting_struct {
	char option;
	union argument_value_struct value;
}; 

struct setting_list_struct {
	int nsettings;
	struct setting_struct** settings;
};

typedef struct setting_struct setting;
typedef union argument_value_struct argument_value;
typedef struct setting_list_struct setting_list;
setting_list* get_settings(int nargs, char* args[]) {
	setting_list* list = malloc(sizeof(setting_list)); 
	list->settings = malloc(nargs*sizeof(int)); // not point in doing nargs*sizeof(int), this is also memory inefficient but who cares
	for(int i = 1; i<nargs; i++) {
		char* argument = args[i];
		char* next_argument = args[i+1];
		if(argument[0] == '-') {
			setting* setting = malloc(sizeof(setting));
			list->settings[list->nsettings++] = setting;
			setting->option = argument[1];
			if(i+1 >= nargs || next_argument[0] == '-') {  
				 setting->value.no_value = true; // apparently i have to do setting->value.NO_VALUE because some reason unions are funky, probably just compiler needs to know what type, i dont program in c
				 continue; 			
			}
			int success = sscanf(next_argument, "%d",&setting->value);
			if(success != 1) // if scanning for a number didnt succeed 
				success = sscanf(next_argument, "%s", &setting->value);
			if(success != 1) // if scanning for a string didnt succeed
				printf("error lol!");
		}
	};
	return list;
}


char get_max_brightness() {
	FILE *f_max_brightness;
	f_max_brightness = fopen("/sys/class/backlight/nvidia_wmi_ec_backlight/max_brightness","r");
	
	char* c_max_brightness = malloc(b_read_size);
	fgets(c_max_brightness,b_read_size,f_max_brightness);
	
	char max_brightness;
	sscanf(c_max_brightness,"%d",&max_brightness);
	free(c_max_brightness);
	//fclose(f_max_brightness);
	return max_brightness;
}

char get_brightness() {
	FILE *f_brightness;
	f_brightness = fopen("/sys/class/backlight/nvidia_wmi_ec_backlight/brightness","r");

	if(!f_brightness) {
		printf("broken as FUCK");
		return -1;
	}
	char* c_brightness = malloc(b_read_size);
	fgets(c_brightness,b_read_size,f_brightness);

	char brightness;
	sscanf(c_brightness,"%d",&brightness);
	return brightness;
}
int set_brightness(char new_brightness) {
	FILE *f_brightness;
	f_brightness = fopen("/sys/class/backlight/nvidia_wmi_ec_backlight/brightness","w+");

	if(!f_brightness) {
		printf("broken as FUCK");
		return -1;
	}	

	char* c_new_brightness = malloc(4);
	sprintf(c_new_brightness,"%d",new_brightness);
	fputs(c_new_brightness,f_brightness);
	//fclose(f_brightness); -- causes a FUNKY ass segmentation fault
	return 1;	
}

int change_brightness(char change) {
	char max_brightness=get_max_brightness();
	char old_brightness=get_brightness();
	char new_brightness = old_brightness+change;
	if(new_brightness > max_brightness || new_brightness < 0) {
		printf("broken ass shit bro stop trying to make the brightness higher than you know you can");
		return -1;
	};
	set_brightness(old_brightness+change);
	return 1;
}

int main(int nargs, char** args) {
	setting_list* list = get_settings(nargs,args);
	for(int i = 0; i<list->nsettings; i++) {
		setting* setting = list->settings[i];
		switch(setting->option) {
			case 'i':
				change_brightness(setting->value.number_value);	
			
			break;
			case 'd':
				change_brightness(-setting->value.number_value);
			
			break;
			case 's':
				set_brightness(setting->value.number_value);
			break;
			default:
			printf("Unknown option: -%c, please try something else\n", setting->option);
		}
	}
}

