#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include "cmd.h"
#include "exec.h"
#include "monitor.h"
#include "update.h"
#include "config.h"

#define READ	0
#define WRITE	1

static pid_t pid = -1;
static int fdin[2], fdout[2], fderr[2];
static FILE *fin, *fout, *ferr;

int exec_status()
{
	return pid;
}

void exec_write_stdin(const char *prompt, const char *str, int echo)
{
	if (pid < 0)
		return;
	// Thread-safe?
	fprintf(fin, "%s\n", str);
	fflush(fin);
	if (echo)
		cmd_printf(CLR_ECHO, "%s: %s\n", prompt, str);
}

int exec_rfd(int err)
{
	if (pid < 0)
		return -1;
	return fileno(err ? ferr : fout);
}

static char *exec_read(int err)
{
	char *s = NULL;
	unsigned int len = 0;
	do {
		char buf[256];
		if (!fgets(buf, sizeof(buf), err ? ferr : fout))
			return s;
		unsigned int slen = strlen(buf);
		s = realloc(s, len + slen + 1);
		memcpy(s + len, buf, slen + 1);
		len += slen;
	} while (s[len - 1] != '\n');
	s[len - 1] = 0;
	return s;
}

int exec_process(int err)
{
	char *str = exec_read(err);
	if (!str)
		return EOF;
	cmd_printf(err ? CLR_STDERR : CLR_STDOUT, "%s\n", str);
	monitor_line(str);
	free(str);
	return 0;
}

int exec_server(const char *dir, const char *jar)
{
	if (pid >= 0)
		return EBUSY;
	cmd_printf(CLR_MESSAGE, "%s: Starting %s in %s\n", __func__, jar, dir);
	monitor_server_start();
	// Create input/output pipes
	pipe(fdin);
	pipe(fdout);
	pipe(fderr);

	// Create child process
	pid = fork();
	if (pid < 0) {
		monitor_server_stop();
		return pid;
	}

	if (pid) {
		// Parent process
		close(fdout[WRITE]);
		close(fderr[WRITE]);
		fin = fdopen(fdin[WRITE], "w");
		fout = fdopen(fdout[READ], "r");
		ferr = fdopen(fderr[READ], "r");
		setlinebuf(fin);
		setbuf(fout, NULL);
		setbuf(ferr, NULL);
		cmd_printf(CLR_MESSAGE, "%s: Process %u started\n",
				__func__, pid);
		return 0;
	}

	// Child process
	// Redirect stdio
	dup2(fdin[READ], STDIN_FILENO);
	dup2(fdout[WRITE], STDOUT_FILENO);
	dup2(fderr[WRITE], STDERR_FILENO);

	// Execute
	if (dir) {
		int err = chdir(dir);
		if (err) {
			cmd_printf(CLR_ERROR, "%s: Error changing directory: "
					"%s\n", __func__, strerror(errno));
			exit(0);
		}
	}
	execlp(EXEC_JAVA, EXEC_JAVA, EXEC_ARGS, (char *)NULL);
	return 0;
}

void exec_stop()
{
	if (monitor_server_status())
		exec_write_stdin(__func__, CMD_STOP, ECHO_CMD);
	else
		exec_kill();
}

void exec_kill()
{
	if (pid < 0)
		return;
	kill(pid, SIGTERM);
	exec_quit();
}

void exec_quit()
{
	if (pid < 0)
		return;
	waitpid(pid, NULL, 0);
	cmd_printf(CLR_MESSAGE, "%s: Process %u exited\n", __func__, pid);
	pid = -1;
	fclose(fin);
	close(fdin[READ]);
	fclose(fout);
	fclose(ferr);
	monitor_server_stop();
}

int exec_backup()
{
	cmd_printf(CLR_BACKUP, "%s: Executing backup commands\n", __func__);
	cmd_external(CLR_EXTERNAL);
	pid_t pid = fork();
	if (!pid)
		execl(EXEC_BACKUP, EXEC_BACKUP,
				SERVER_PATH, update_current(), (char *)NULL);
	int status = -1;
	waitpid(pid, &status, 0);
	return WEXITSTATUS(status);
}
