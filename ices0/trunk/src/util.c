/* util.c
 * - utility functions for ices
 * Copyright (c) 2000 Alexander Hav�ng
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include "definitions.h"

#include <string.h>
#include <thread.h>

extern shout_conn_t conn;
extern ices_config_t ices_config;

char **ices_argv;
int ices_argc;
const char cnull[7] = "(null)";

/* Public function definitions */

/* Wrapper function around strdup. Will always return
 * an allocated string, "(null)" if given NULL argument. */
char *
ices_util_strdup (const char *string)
{
	char *out;

	if (string) {
		out = strdup (string);
	} else {
		out = strdup ("(null)");
	}

	return out;
}

/* Wrapper function around CLI argv */
char **
ices_util_get_argv (void)
{
	return ices_argv;
}

/* Wrapper function around CLI argc */
int 
ices_util_get_argc (void)
{
	return ices_argc;
}

/* Mutator function for CLI args */
void
ices_util_set_args (int argc, char **argv)
{
	ices_argc = argc;
	ices_argv = argv;
}

/* Wrapper function around open */
int
ices_util_open_for_reading (const char *file) 
{
	if (file)
		return open (file, O_RDONLY);
	return -1;
}

/* Wrapper function around fopen */
FILE *
ices_util_fopen_for_reading (const char *file) 
{
	if (file)
		return fopen (file, "r");
	return NULL;
}

/* fOpen file for writing, in the base directory, unless
 * filename starts with '/' */
FILE *
ices_util_fopen_for_writing (const char *file)
{
	char namespace[2048];

	if (!file)
		return NULL;

	if (!ices_config.base_directory) {
		ices_log ("Ices Base Directory is NULL!");
		return NULL;
	}

	if (!ices_util_directory_exists (ices_config.base_directory)) {
		ices_log ("Ices base directory [%s] does not exist, trying to create", ices_config.base_directory);
		if (ices_util_directory_create (ices_config.base_directory)) {
			ices_log ("Could not create directory [%s]");
			return NULL;
		}
	}
	
	if (file[0] != '/' && file[0] != '\\') {
		sprintf (namespace, "%s/file", ices_config.base_directory);
	} else {
		strncpy (namespace, file, 2048);
	}
	
	return fopen (namespace, "w");
}

/* Wrapper function around fclose */
void
ices_util_fclose (FILE *fp) 
{
	if (fp) 
		fclose (fp);
}

/* Verify the validity of a file descriptor */
int
ices_util_valid_fd (int fd)
{
	return (fd >= 0);
}

/* Return a dynamically allocated string, the next line read
 * from FILE *fp */
char *
ices_util_read_line (FILE *fp)
{
	char temp[1024];

	if (!fp)
		return NULL;

	temp[0] = '\0';

	if (!fgets (temp, 1024, fp)) {

		if (!feof (fp)) {
			ices_log_error ("Got error while reading file, error: [%s]", ices_util_strerror (errno, temp, 1024));
			return NULL;
		}

		return ices_util_strdup (temp);
	}

	return ices_util_strdup (temp);
}

/* Create a box-unique filename of a certain type */
char *
ices_util_get_random_filename (char *namespace, char *type)
{
	if (!namespace || !type) {
		ices_log ("WARNING: ices_util_get_random_filename() called with NULL pointers.");
		return NULL;
	}

#ifdef _WIN32
	doooh();
#else
	sprintf (namespace, "/tmp/ices.%s.%d", type, (int)getpid ());
	return namespace;
#endif
}

/* Wrapper function around remove() */
int
ices_util_remove (const char *filename)
{
	if (filename)
		return remove (filename);
	return -1;
}

/* Function to get a box-unique integer */
int
ices_util_get_random (void) 
{
#ifdef _WIN32
	doooh();
#else
	return getpid();
#endif
}

/* Verify that the file descriptor is a regular file
 * return -1 on error
 * 1 when file is a regular file or link
 * -2 when fd is a directory
 * else 0 (fifo or pipe or something weird) */
int
ices_util_is_regular_file (int fd)
{
	struct stat st;
	
	if (fstat (fd, &st) == -1) {
		ices_log_error ("ERROR: Could not stat file");
		return -1;
	}

	if (S_ISLNK (st.st_mode) || S_ISREG (st.st_mode))
		return 1;

	if (S_ISDIR(st.st_mode)) {
		ices_log_error ("ERROR: Is directory!");
		return -2;
	}

	return 0;
}

/* Return the size of the file pointed to by fd */
int 
ices_util_fd_size (int fd)
{
	struct stat st;

	if (fstat (fd, &st) == -1) {
		ices_log_error ("ERROR: Could not stat file");
		return -1;
	}

	return st.st_size;
}

/* Make sure directory exists and is a directory */
int 
ices_util_directory_exists (const char *name)
{
	struct stat st;
	
	if (!name) {
		ices_log ("ices_util_directory_exists() called with NULL");
		return 0;
	}

	if (stat (name, &st) == -1)
		return 0;

	if (!S_ISDIR (st.st_mode))
		return 0;

	return 1;
}

/* Wrapper function around directory creation */
int
ices_util_directory_create (const char *name)
{
	if (name)
		return mkdir (name, 00755);
	return -1;
}

/* Verify that the string is not null, and if it is, return "(null)" */
const char *
ices_util_nullcheck (const char *string)
{
	if (!string)
		return &cnull[0];
	return string;
}

/* Wrapper function for percentage */
double
ices_util_percent (int this, int of_that)
{
	return (double)((double)this / (double) of_that) * 100.0;
}

/* Given bitrate and filesize, report the length of the given file
 * by nice formatting in string buf.
 * Note: This only works ok with CBR */
char *
ices_util_file_time (int bitrate, int filesize, char *buf)
{
	unsigned long int days, hours, minutes, nseconds, remains;
	unsigned long int seconds;

	 seconds = (double)((double)filesize / ((double)(bitrate * 1000) / 8.0));

	if (!buf)
		return NULL;

	days = seconds / 86400;
	remains = seconds % 86400;

	hours = remains / 3600;
	remains = remains % 3600;

	minutes = remains / 60;
	nseconds = remains % 60;

	sprintf (buf, "%lu:%lu:%lu:%lu", days, hours, minutes, nseconds);
	
	return buf;
}

/* Threadsafe version of strerror */
const char *
ices_util_strerror (int error, char *namespace, int maxsize)
{
	thread_library_lock ();
	strncpy (namespace, strerror (error), maxsize);
	thread_library_unlock ();
	return namespace;
}

/* Wrapper function around free */
void
ices_util_free (void *ptr)
{
	if (ptr)
		free (ptr);
}

/* Wrapper function around close */
void
ices_util_close (int fd)
{
	close (fd);
}

/* Verify that file exists, and is readable */
int
ices_util_verify_file (const char *filename)
{
	FILE *fp;

	if (!filename || !filename[0])
		return 0;

	if (!(fp = ices_util_fopen_for_reading (filename)))
		return 0;

	ices_util_fclose (fp);

	return 1;
}
