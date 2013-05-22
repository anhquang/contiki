/*
 * jsmn-utility.h
 *
 *  Created on: May 21, 2013
 *      Author: nqdinh
 */

#ifndef JSMN_UTILITY_H_
#define JSMN_UTILITY_H_

/* Terminate current test with error */
#define fail()	return __LINE__
/* Successfull end of the test case */
#define done() return 0
/* Check single condition */
#define check(cond) do { if (!(cond)) fail(); } while (0)

#define TOKEN_EQ(t, tok_start, tok_end, tok_type) \
	((t).start == tok_start \
	 && (t).end == tok_end  \
	 && (t).type == (tok_type))
#define TOKEN_STRING(js, t, s) \
	(strncmp(js+(t).start, s, (t).end - (t).start) == 0 \
	 && strlen(s) == (t).end - (t).start)
#define TOKEN_PRINT(t) \
	PRINTF("start: %d, end: %d, type: %d, size: %d\n", \
			(t).start, (t).end, (t).type, (t).size)

#endif /* JSMN_UTILITY_H_ */
