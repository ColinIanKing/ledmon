/*
 * AMD LED control
 * Copyright (C) 2019, Advanced Micro Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/sysinfo.h>
#include <sys/file.h>

#if _HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include "config.h"
#include "ibpi.h"
#include "list.h"
#include "utils.h"
#include "amd.h"
#include "amd_sgpio.h"

int _find_file_path(const char *start_path, const char *filename,
		    char *path, size_t path_len)
{
	int rc, found;
	struct stat sbuf;
	struct list dir;
	char *dir_name;
	const char *dir_path;

	rc = scan_dir(start_path, &dir);
	if (rc) {
		log_info("Failed to scan %s", start_path);
		return 0;
	}

	found = 0;
	list_for_each(&dir, dir_path) {
		dir_name = strrchr(dir_path, '/');
		if (!dir_name)
			continue;

		/* skip past the leading '/' */
		dir_name++;

		if (strncmp(dir_name, filename, strlen(filename)) == 0) {
			char tmp[PATH_MAX + 1];

			strncpy(tmp, dir_path, path_len);
			snprintf(path, path_len, "%s", dirname(tmp));

			found = 1;
			break;
		}

		if (lstat(dir_path, &sbuf) == -1)
			continue;

		if (S_ISDIR(sbuf.st_mode)) {
			found = _find_file_path(dir_path, filename,
						path, path_len);
			if (found)
				break;
		}
	}

	list_erase(&dir);
	return found;
}

int amd_em_enabled(const char *path)
{
	return _amd_sgpio_em_enabled(path);
}

int amd_write(struct block_device *device, enum ibpi_pattern ibpi)
{
	/* write only if state has changed */
	if (ibpi == device->ibpi_prev)
		return 1;

	return _amd_sgpio_write(device, ibpi);
}

char *amd_get_path(const char *cntrl_path)
{
	return _amd_sgpio_get_path(cntrl_path);
}