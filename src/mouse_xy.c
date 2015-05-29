#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* get_mouse_xy() {
	FILE* file = popen("xdotool getmouselocation", "r");

	const int max_buffer = 512;
	char buffer[max_buffer];

	if (file) {
		fgets(buffer, max_buffer, file);
		pclose(file);
	}
	char* res = (char *)malloc(max_buffer);
	strcpy(res, buffer);
	return res;
}

void main1() {
	char* r1 = get_mouse_xy();
	char* r2 = get_mouse_xy();
	int res = strcmp(r1, r2);
	printf("%i \n", res);
}
