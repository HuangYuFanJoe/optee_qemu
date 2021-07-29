#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <tee_tcpsocket.h>
#include <tee_udpsocket.h>

#include <get_policy_ta.h>
#include <process_flowctl_ta.h>

static TEE_TASessionHandle process_flowctl_ta_session = TEE_HANDLE_NULL;
static const TEE_UUID process_flowctl_ta_uuid = TA_PROCESS_FLOWCTL_UUID;
static uint32_t ta_param_types;
static TEE_Param ta_params[TEE_NUM_PARAMS];

Receive_Information *RI = NULL;

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
	TEE_Result res;
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

static TEE_Result inc_value(uint32_t param_types,
	TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	IMSG("Got value: %u from NW", params[0].value.a);
	params[0].value.a++;
	IMSG("Increase value to: %u", params[0].value.a);

	return TEE_SUCCESS;
}

static TEE_Result dec_value(uint32_t param_types,
	TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	IMSG("Got value: %u from NW", params[0].value.a);
	params[0].value.a--;
	IMSG("Decrease value to: %u", params[0].value.a);

	return TEE_SUCCESS;
}

static TEE_Result tcp_connect(TEE_tcpSocket_Setup *setup,
			      TEE_iSocketHandle *ctx)
{
	TEE_Result res;
	uint32_t proto_error;

	setup->ipVersion   = TEE_IP_VERSION_DC;
	setup->server_addr = "140.115.52.122";
	setup->server_port = 8080U;

	res = TEE_tcpSocket->open(ctx, setup, &proto_error);
	if (res != TEE_SUCCESS) {
		EMSG("TCP_Socket open() failed. Return code: %u"
		     ", protocol error: %u", res, proto_error);
		return res;
	}

	return TEE_SUCCESS;
}

static TEE_Result udp_connect(TEE_udpSocket_Setup *setup,
			      TEE_iSocketHandle *ctx)
{
	TEE_Result res;
	uint32_t proto_error;

	setup->ipVersion   = TEE_IP_VERSION_DC;
	setup->server_addr = "140.115.52.122";
	setup->server_port = 8080U;

	res = TEE_udpSocket->open(ctx, setup, &proto_error);
	if (res != TEE_SUCCESS) {
		EMSG("UDP_Socket open() failed. Return code: %u"
		     ", protocol error: %u", res, proto_error);
		return res;
	}

	return TEE_SUCCESS;
}

static char *prepare_message(uint32_t param_types, TEE_Param params[4], unsigned int *message_size)
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INOUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;
    
	TEE_Time ree_time;
	TEE_GetREETime(&ree_time);

	*message_size = snprintf(NULL, 0, 
			"Requester=%s , time=%d", "Yufan's satellite", ree_time.seconds);
	IMSG("prepare message size = %d\n", *message_size);
	
	char *message_buffer = NULL;
	message_buffer = (char *)TEE_Malloc(*message_size + 1, TEE_MALLOC_FILL_ZERO);
	if (message_buffer == NULL)
		return message_buffer;

	snprintf(message_buffer, *message_size + 1,
		"Requester=%s , time=%d", "Yufan's satellite", ree_time.seconds);

	return message_buffer;
}

static TEE_Result socket_client(uint32_t param_types,
	TEE_Param params[4])
{
	TEE_Result res;
	
	TEE_iSocket *socket_Policy = NULL;
	TEE_iSocketHandle socketCtx_Policy;
	TEE_tcpSocket_Setup tcpSetup_Policy;
	TEE_udpSocket_Setup udpSetup_Policy;
	
	TEE_Time ree_time;
	TEE_GetREETime(&ree_time);
	
	char *message = NULL;
	unsigned int message_size = 0;
	
	message = prepare_message(param_types, params, &message_size);
	if (message == NULL) {
		return TEE_ERROR_OUT_OF_MEMORY;
	}
	
	IMSG("prepared message: %s\n", message);

	IMSG("Connect to Policy Server...\n");
	res = tcp_connect(&tcpSetup_Policy, &socketCtx_Policy);
	//res = udp_connect(&udpSetup_Policy, &socketCtx_Policy, s_args_Policy);
	
	if (res != TEE_SUCCESS) {
        IMSG("TCP/UDP connect failed!!\n");
		return TEE_ERROR_BAD_PARAMETERS;
	}
	
	IMSG("TCP/UDP connected\n");
	
	socket_Policy = TEE_tcpSocket;
	res = socket_Policy->send(socketCtx_Policy, message, &message_size, TEE_TIMEOUT_INFINITE);
	if (res != TEE_SUCCESS) {
		EMSG("Policy server socket send() failed. Error code: %#0" PRIX32, res);
		return res;
	}

	IMSG("Send request to Policy server successfully\n");

	RI = (Receive_Information *) params[0].memref.buffer;
	
	res = socket_Policy->recv(socketCtx_Policy, (void *)RI, &params[0].memref.size, 100);
	if (res != TEE_SUCCESS) {
		EMSG("Policy server socket recv() failed. Error code: %#0" PRIX32, res);
		return res;
	}
	
	RI->recv_time = ree_time.seconds;

	IMSG("Receive policy successfully\n");

	socket_Policy->close(socketCtx_Policy);

	return res;
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
		case TA_HELLO_WORLD_CMD_INC_VALUE:
			return inc_value(param_types, params);
		case TA_HELLO_WORLD_CMD_DEC_VALUE:
			return dec_value(param_types, params);
		case TA_GET_POLICY_CMD:
			return socket_client(param_types, params);
		default:
			return TEE_ERROR_BAD_PARAMETERS;
	}
}
