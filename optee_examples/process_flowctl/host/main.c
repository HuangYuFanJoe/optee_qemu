#include <err.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* OP-TEE TEE client API (built by optee_client) */
#include <tee_client_api.h>

/* For the UUID (found in the TA's h-file(s)) */
#include <process_flowctl_ta.h>

int main(int argc, char *argv[])
{
	if(argc < 3 || argc > 3){
		printf("Usage: %s source destination", argv[0]);
		return -1;
	}
	
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
	
	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_UUID uuid = TA_PROCESS_FLOWCTL_UUID;
	uint32_t err_origin;

	res = TEEC_InitializeContext(NULL, &ctx);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

	res = TEEC_OpenSession(&ctx, &sess, &uuid,
			       TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
			res, err_origin);

	Check_Information *CI;
	CI = (Check_Information *) malloc(sizeof(Check_Information));
	if(!CI){
		printf("malloc CI failed!!\n");
		return -1;
	}
	memset(CI, 0, sizeof(Check_Information));
	
	strcpy(CI->source, from_pn);
	strcpy(CI->destination, to_pn);

	/* Clear the TEEC_Operation struct */
	memset(&op, 0, sizeof(op));

	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INOUT, TEEC_NONE,
					 TEEC_NONE, TEEC_NONE);
					 
	op.params[0].tmpref.buffer = CI;
	op.params[0].tmpref.size = sizeof(Check_Information);

	res = TEEC_InvokeCommand(&sess, TA_PROCESS_FLOWCTL_CMD, &op,
				 &err_origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
			res, err_origin);
	printf("Result: %d\n", CI->result);

	TEEC_CloseSession(&sess);

	TEEC_FinalizeContext(&ctx);
	
	free(from_pn);
	free(to_pn);
	if(!CI->result)
		return -1;
	return 0;
}
