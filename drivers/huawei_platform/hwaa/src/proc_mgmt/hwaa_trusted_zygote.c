
#include "inc/proc_mgmt/hwaa_trusted_zygote.h"
#include <linux/binfmts.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/init.h>
#include <securec.h>
#include "inc/base/hwaa_utils.h"

#define ZYGOTE_EXE_COUNT 3

#define ZYGOTE_EXE_PATH_1 "/system/bin/app_process"
#define ZYGOTE_EXE_PATH_LEN_1 (sizeof(ZYGOTE_EXE_PATH_1) - 1)
#define ZYGOTE_EXE_PATH_2 "/system/bin/app_process32"
#define ZYGOTE_EXE_PATH_LEN_2 (sizeof(ZYGOTE_EXE_PATH_2) - 1)
#define ZYGOTE_EXE_PATH_3 "/system/bin/app_process64"
#define ZYGOTE_EXE_PATH_LEN_3 (sizeof(ZYGOTE_EXE_PATH_3) - 1)

#define ZYGOTE_PARENT_PID 1
#define ZYGOTE_PARENT_PROGRAM_NAME "init"
#define ZYGOTE_PARENT_PROGRAM_NAME_LEN (sizeof(ZYGOTE_PARENT_PROGRAM_NAME) - 1)

#define ZYGOTE_MAX_ARG_STRLEN 256
#define EXE_MAX_ARG_STRLEN 256

EXPORT_SYMBOL(saved_command_line);

struct zygote_exec_arg_t {
	const s8 *value;
	const s32 len;
};

static const struct zygote_exec_arg_t ZYGOTE_EXECS[ZYGOTE_EXE_COUNT] = {
	{ ZYGOTE_EXE_PATH_1,
	  ZYGOTE_EXE_PATH_LEN_1 },
	{ ZYGOTE_EXE_PATH_2,
	  ZYGOTE_EXE_PATH_LEN_2 },
	{ ZYGOTE_EXE_PATH_3,
	  ZYGOTE_EXE_PATH_LEN_3 }
};

s32 hwaa_trusted_zygote_lookup(const s8 *exe)
{
	s32 i;

	if (!exe)
		return -EINVAL;
	for (i = 0; i < ZYGOTE_EXE_COUNT; i++) {
		if ((strlen(exe) == ZYGOTE_EXECS[i].len) &&
			strncasecmp(exe, ZYGOTE_EXECS[i].value,
				ZYGOTE_EXECS[i].len) == 0) {
			return i;
		}
	}
	return -EINVAL;
}

s32 hwaa_trusted_zygote_check_parent(pid_t ppid, const s8 *pcomm)
{
	s8 *parent_path;
	u32 parent_path_len;
	s32 ret = 0;
	if (!pcomm || (ppid != ZYGOTE_PARENT_PID) ||
		strncasecmp(pcomm, ZYGOTE_PARENT_PROGRAM_NAME,
		ZYGOTE_PARENT_PROGRAM_NAME_LEN)) {
		return -EINVAL;
	}
	parent_path = get_cmdline_value(saved_command_line);
	if (!parent_path)
		return -EINVAL;
	parent_path_len = strlen(parent_path);
	if (!hwaa_utils_exe_check(ppid, parent_path, parent_path_len))
		ret = -EINVAL;
	kfree(parent_path);
	return ret;
}

bool hwaa_trusted_zygote_has_exe_path(pid_t pid)
{
	s32 i;

	for (i = 0; i < ZYGOTE_EXE_COUNT; i++) {
		if (hwaa_utils_exe_check(pid, ZYGOTE_EXECS[i].value,
			ZYGOTE_EXECS[i].len))
			return true;
	}

	return false;
}
