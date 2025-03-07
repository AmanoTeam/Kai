#if !defined(__HAIKU__)

#if defined(_WIN32)
	#include <windows.h>
	#include <securitybaseapi.h>
#endif

#if !defined(_WIN32)
	#include <unistd.h>
#endif

#include "os/privileges.h"

int is_administrator(void) {
	/*
	Returns whether the caller's process is a member of the Administrators local
	group (on Windows) or a root (on POSIX), via "geteuid() == 0".
	
	Returns (1) on true, (0) on false, (-1) on error.
	*/
	
	#if defined(_WIN32)
		SID_IDENTIFIER_AUTHORITY authority = {SECURITY_NT_AUTHORITY};
		PSID group = {0};
		BOOL is_member = FALSE;
		BOOL status = FALSE;
		
		status = AllocateAndInitializeSid(
			&authority,
			2,
			SECURITY_BUILTIN_DOMAIN_RID,
			DOMAIN_ALIAS_RID_ADMINS,
			0,
			0,
			0,
			0,
			0,
			0,
			&group
		);
		
		if (!status) {
			return -1;
		}
		
		status = CheckTokenMembership(0, group, &is_member);
		
		FreeSid(group);
		
		if (!status) {
			return -1;
		}
		
		return (int) is_member;
	#else
		return geteuid() == 0;
	#endif
	
}

#endif
