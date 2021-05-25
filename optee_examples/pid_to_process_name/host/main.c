#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
	char* from_pn = malloc(sizeof(char) * 50);
	char* to_pn = malloc(sizeof(char) * 50);
	memset(from_pn, '\0', sizeof(char) * 50);
	memset(to_pn, '\0', sizeof(char) * 50);
	
	FILE *fr;
	char path1[50] = {};
	sprintf(path1, "/proc/%s/cmdline", argv[1]);
	fr = fopen(path1, "r");
	fscanf(fr, "%s", from_pn);
	fclose(fr);
	char path2[50] = {};
	sprintf(path2, "/proc/%s/cmdline", argv[2]);
	fr = fopen(path2, "r");
	fscanf(fr, "%s", to_pn);
	fclose(fr);
	
	FILE *fw;
	fw = fopen("/pid_to_process_name.txt", "w");
	fprintf(fw, "%s, %s", from_pn, to_pn);
	fclose(fw);
	
	free(from_pn);
	free(to_pn);
	return 0;
}
