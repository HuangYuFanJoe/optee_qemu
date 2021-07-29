#ifndef TA_PROCESS_FLOWCTL_H
#define TA_PROCESS_FLOWCTL_H

#define MAX_CHECK_STRING_SIZE 50

typedef struct {
	bool result;
	char source[MAX_CHECK_STRING_SIZE];
	char destination[MAX_CHECK_STRING_SIZE];
} Check_Information;


/*
 * This UUID is generated with uuidgen
 * the ITU-T UUID generator at http://www.itu.int/ITU-T/asn1/uuid.html
 */
#define TA_PROCESS_FLOWCTL_UUID \
	{ 0x0ed43bf1, 0x45ee, 0x47aa, \
		{ 0x8d, 0x45, 0xf4, 0xc4, 0x0a, 0x32, 0x37, 0xf2} }

/* The function IDs implemented in this TA */
#define TA_PROCESS_FLOWCTL_CMD		0
#define TA_UPDATE_WHITELIST_CMD		1

#endif /*TA_PROCESS_FLOWCTL_H*/
