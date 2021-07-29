#include <err.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
/* OP-TEE TEE client API (built by optee_client) */
#include <tee_client_api.h>

/* For the UUID (found in the TA's h-file(s)) */
#include <get_policy_ta.h>

void inc_value(TEEC_Result *res, TEEC_Session *sess,
		  TEEC_Operation *op, uint32_t *err_origin)
{
	op->paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT, TEEC_NONE,
					 TEEC_NONE, TEEC_NONE);
	op->params[0].value.a = 42;

	printf("Invoking TA to increment %d\n", op->params[0].value.a);
	*res = TEEC_InvokeCommand(sess, TA_HELLO_WORLD_CMD_INC_VALUE, op,
				 err_origin);
	if (*res != TEEC_SUCCESS)
		errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
			*res, *err_origin);
	printf("TA incremented value to %d\n", op->params[0].value.a);
}

void dec_value(TEEC_Result *res, TEEC_Session *sess,
		  TEEC_Operation *op, uint32_t *err_origin)
{
	op->paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT, TEEC_NONE,
					 TEEC_NONE, TEEC_NONE);
	op->params[0].value.a = 42;

	printf("Invoking TA to decrement %d\n", op->params[0].value.a);
	*res = TEEC_InvokeCommand(sess, TA_HELLO_WORLD_CMD_DEC_VALUE, op,
				 err_origin);
	if (*res != TEEC_SUCCESS)
		errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
			*res, *err_origin);
	printf("TA decremented value to %d\n", op->params[0].value.a);
}

Receive_Information *get_request_info(TEEC_Result *result,
					      TEEC_Session *session,
					      TEEC_Operation *operation,
					      uint32_t *err_origin)
{
	/*
	 * Prepare the argument
	 */
	Receive_Information *RI;
	RI = (Receive_Information *) malloc(sizeof(Receive_Information));
	if (!RI) {
		return NULL;
	}
	memset(RI, 0, sizeof(Receive_Information));

	operation->paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_OUTPUT,
						 TEEC_NONE,
						 TEEC_NONE,
						 TEEC_NONE);

	operation->params[0].tmpref.buffer = RI;
	operation->params[0].tmpref.size = sizeof(Receive_Information);
	/*
	 * TA_CMD_GET_POLICY
	 */
	//printf("Invoke TA to get the Policy from server\n");
	*result = TEEC_InvokeCommand(session, TA_GET_POLICY_CMD,
	 			     operation, err_origin);
	if (*result != TEEC_SUCCESS) {
		errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
			*result, *err_origin);
	}
	
	printf("Get the Receive information successfully.\n");

	printf("Servername : %s, policy: %s, digest: %s\n, list_cnt: %d, Receive time: %d\n",
			RI->servername, RI->policy[0], RI->digest, RI->list_cnt, RI->recv_time);
	
	return RI;
}

int main(int argc, char *argv[])
{
	int choice;
	if (argc < 2) {
		printf("Usage: %s 0|1|2 (0 -> inc value | 1 -> dec value | 2 -> get Policy) \n", argv[0]);
		return 0;
	}
 	choice = atoi(argv[1]); 
	if (choice != 0 && choice != 1 && choice != 2) {
		printf("Choice must be 0、1 or 2 only!!!\n");
		return 0;
	}
	
	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op, t_op;
	TEEC_UUID uuid = TA_GET_POLICY_UUID;
	uint32_t err_origin;
	Receive_Information *RI = NULL;

	/* Initialize a context connecting us to the TEE */
	res = TEEC_InitializeContext(NULL, &ctx);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

	static bool entried = false;

	res = TEEC_OpenSession(&ctx, &sess, &uuid,
			       TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
			res, err_origin);

	/* Clear the TEEC_Operation struct */
	memset(&op, 0, sizeof(op));


	switch(choice){
		case 2 :
			RI = get_request_info(&res, &sess, &op, &err_origin);
			break;
		case 1 :
			dec_value(&res, &sess, &op, &err_origin);
			break;
		case 0 :
			inc_value(&res, &sess, &op, &err_origin);
			break;
		default :
			break;
	}

	TEEC_CloseSession(&sess);
	TEEC_FinalizeContext(&ctx);

	return 0;
}
