#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include <process_flowctl_ta.h>
#include <get_policy_ta.h>
#include <string.h>

static TEE_TASessionHandle get_policy_ta_session = TEE_HANDLE_NULL;
static const TEE_UUID get_policy_ta_uuid = TA_GET_POLICY_UUID;
static uint32_t ta_param_types;
static TEE_Param ta_params[TEE_NUM_PARAMS];
char **whitelist;
uint32_t list_cnt;
Receive_Information *RI;
/*
 * Called when the instance of the TA is created. This is the first call in
 * the TA.
 */
TEE_Result TA_CreateEntryPoint(void)
{
	DMSG("has been called");

	return TEE_SUCCESS;
}

/*
 * Called when the instance of the TA is destroyed if the TA has not
 * crashed or panicked. This is the last call in the TA.
 */
void TA_DestroyEntryPoint(void)
{
	DMSG("has been called");
}

/*
 * Called when a new session is opened to the TA. *sess_ctx can be updated
 * with a value to be able to identify this session in subsequent calls to the
 * TA. In this function you will normally do the global initialization for the
 * TA.
 */
TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
		TEE_Param __maybe_unused params[4],
		void __maybe_unused **sess_ctx)
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	/* Unused parameters */
	(void)&params;
	(void)&sess_ctx;

	TEE_Result res;
	if(get_policy_ta_session == TEE_HANDLE_NULL){
		res = TEE_OpenTASession(&get_policy_ta_uuid, TEE_TIMEOUT_INFINITE,
					0, NULL, &get_policy_ta_session, NULL);

		if (res != TEE_SUCCESS) {
			IMSG("TEE_OpenTASession to get policy TA failed.");
			return res;
		}
		IMSG("TA Open get policy TA session successfully.");
	}

	/* Prepare the TEE parameters */
	ta_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INOUT,
					  TEE_PARAM_TYPE_NONE,
					  TEE_PARAM_TYPE_NONE,
					  TEE_PARAM_TYPE_NONE);

	/* If return value != TEE_SUCCESS the session will not be created. */
	return TEE_SUCCESS;
}

/*
 * Called when a session is closed, sess_ctx hold the value that was
 * assigned by TA_OpenSessionEntryPoint().
 */
void TA_CloseSessionEntryPoint(void __maybe_unused *sess_ctx)
{
	(void)&sess_ctx; /* Unused parameter */
	IMSG("Goodbye!\n");
}

void set_default(void)
{
	// Set default whitelist
	whitelist = TEE_Malloc(sizeof(char *), TEE_MALLOC_FILL_ZERO);
	char *default_whitelist;
	default_whitelist = TEE_Malloc(sizeof(char) * (2 * MAX_CHECK_STRING_SIZE), TEE_MALLOC_FILL_ZERO);
	strcpy(default_whitelist, "take_picture blur_image");
	whitelist[0] = default_whitelist;
	list_cnt = 1;
	return;
}

static TEE_Result update_whitelist(void)
{
	TEE_Result res;
	RI = (Receive_Information *) TEE_Malloc(sizeof(Receive_Information), TEE_MALLOC_FILL_ZERO);
	if (!RI) {
		DMSG("Malloc failed");
		return TEE_ERROR_BAD_PARAMETERS;
	}
	memset(RI, 0, sizeof(Receive_Information));

	/* Set the TEE parameters */
	ta_params[0].memref.buffer = RI;
	ta_params[0].memref.size = sizeof(Receive_Information);

	/* Invoke the get policy TA */
	res = TEE_InvokeTACommand(get_policy_ta_session, TEE_TIMEOUT_INFINITE,
				  TA_GET_POLICY_CMD,
				  ta_param_types, ta_params, NULL);

	if (res != TEE_SUCCESS) {
		EMSG("TEE_InvokeTACommand failed with code 0x%x", res);
		return res;
	}

	IMSG("Servername : %s, policy: %s, digest: %s\n, list_cnt: %d, Receive time: %d\n",
			RI->servername, RI->policy[0], RI->digest, RI->list_cnt, RI->recv_time);

	list_cnt = RI->list_cnt;

	whitelist = TEE_Malloc(sizeof(char*) * list_cnt, TEE_MALLOC_FILL_ZERO);
	whitelist[0] = RI->policy[0];

	if(!whitelist)
		IMSG("NULL whitelist");
	else
		IMSG("whitelist update successfully");

	return TEE_SUCCESS;
}

static TEE_Result check_flow(uint32_t param_types,
	TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INOUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	Check_Information *CI = NULL;
	CI = (Check_Information *) params[0].memref.buffer;
	
	IMSG("source: %s, destination: %s\n", CI->source, CI->destination);
	
	char* check;
	check = TEE_Malloc(sizeof(char) * (2 * MAX_CHECK_STRING_SIZE), TEE_MALLOC_FILL_ZERO);
	sprintf(check, "%s %s", CI->source, CI->destination);

	TEE_Time before, after;
	TEE_GetREETime(&before);
	
	if(!whitelist){
		set_default();
		IMSG("Set default policy");
	}
	else
		update_whitelist();

	/* check */
	for(uint32_t i = 0; i < list_cnt; i++){
		if(strcmp(whitelist[i], check) == 0){
			CI->result = 1;
		}
	}
	
	TEE_GetREETime(&after);
	IMSG("Before: %d.%d\n", before.seconds, before.millis);
	IMSG("After: %d.%d\n", after.seconds, after.millis);
	IMSG("TA check result: %d", CI->result);

	return TEE_SUCCESS;
}

/*
 * Called when a TA is invoked. sess_ctx hold that value that was
 * assigned by TA_OpenSessionEntryPoint(). The rest of the paramters
 * comes from normal world.
 */
TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
			uint32_t cmd_id,
			uint32_t param_types, TEE_Param params[4])
{
	(void)&sess_ctx; /* Unused parameter */

	switch (cmd_id) {
	case TA_PROCESS_FLOWCTL_CMD:
		return check_flow(param_types, params);
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
