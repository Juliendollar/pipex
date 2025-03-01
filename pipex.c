/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jd-halle <jd-halle@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/30 17:29:48 by jd-halle          #+#    #+#             */
/*   Updated: 2025/01/06 18:40:44 by jd-halle         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pipex.h"

void	close_all(int fd_in, int fd_out, int pipe_fd[2])
{
	if (fd_in != -1)
	{	
		ft_printf("close fd_in\n");
		close(fd_in);	
	}
	if (fd_out != -1)
	{
		ft_printf("close fd_out\n");
		close(fd_out);
	}
	if (pipe_fd[0] != -1)
	{
		ft_printf("close pipe_fd[0]\n");
		close(pipe_fd[0]);
	}
	if (pipe_fd[1] != -1)
	{
		ft_printf("close pipe_fd[1]\n");
		close(pipe_fd[1]);
	}
}


void	ft_free_tab(char **tab)
{
	int i;
	
	i = 0;
	if (!tab)
		return;
	while (tab[i])
	{
		free (tab[i]);
		tab[i] = NULL;
		i++;
	}
	free (tab);
	tab = NULL;
}

void	error(char *message, char **args, char *cmd, int exit_code)
{
	if (exit_code != 0)
	{
		if (message)
		{
			ft_putstr_fd("Error: ", 2);
			if (ft_strcmp(message, "Command not found: ") == 0 && cmd)
			{
				ft_putstr_fd("Command not found: ", 2);
				ft_putstr_fd(cmd, 2);
				ft_putstr_fd("\n", 2);
			}
			else if (ft_strcmp(message, "Redirecting input\n") == 0 || ft_strcmp(message, "Redirecting output\n") == 0)
				free(cmd);
			else
				ft_putstr_fd(message, 2);
		}
		if (args)
			ft_free_tab(args);
	}
	if (exit_code != 0)
		exit(exit_code);
}


char	*join_path_cmd(char *path, char *cmd)
{
	char	*path_with_slash;
	char	*full_path;
	
	path_with_slash = ft_strjoin(path, "/");
	if (!path_with_slash)
		return (NULL);
	full_path = ft_strjoin(path_with_slash, cmd);
	free(path_with_slash);
	if (!full_path)
		return (NULL);
	return (full_path);
}

char *get_command_path(char *cmd, char **envp)
{
	char *path_var;
	char **paths;
	char *full_path;
	int i;

	i = 0;
	path_var = NULL;

	if (!ft_strncmp(cmd, "/", 1) || !ft_strncmp(cmd, "./", 2) || !ft_strncmp(cmd, "../", 3))
	{
		return (ft_strdup(cmd));
	}
	while (envp[i])
	{
		if (ft_strncmp ("PATH=", envp[i], 5) == 0)
		{
			path_var = envp[i] + 5;
			break;
		}
		i++;
	}
	if (!path_var)
		return (NULL);
	paths = ft_split(path_var, ':');
	if (!paths)
		return (NULL);
	i= 0;
	while (paths[i])
	{
		full_path = join_path_cmd(paths[i], cmd);
		if (!full_path)
		{
			ft_free_tab(paths);
			return (NULL);
		}
		if (access(full_path, X_OK) == 0)
		{
			ft_free_tab(paths);
			return (full_path);
		}
		free (full_path);
		i++;
	}
	ft_free_tab(paths);
	return (NULL);
}

void execute_cmd(char *cmd, int input_fd, int output_fd, char **envp)
{
	char **args;
	char *cmd_path;
	
	if (!cmd || cmd[0] == 0)
		error("Command is empty\n", NULL, NULL, 1);
	args = ft_split(cmd, ' ');
	if (!args || !args[0])
		error("Parsing command\n", NULL, NULL, 1);
	cmd_path = get_command_path(args[0], envp);
	if (!cmd_path)
		error("Command not found: ", args, cmd, 127);
	if(dup2(input_fd, STDIN_FILENO) == -1)
		error("Redirecting input\n", args, cmd_path, 1);
	if (dup2(output_fd, STDOUT_FILENO) == -1)
		error("Redirecting output\n", args, cmd_path, 1);
	execve(cmd_path, args, envp);
	free(cmd_path);
	error("Executing command\n", args, NULL, 1);
}

int main(int argc, char **argv, char **envp)
{
	int	fd_in;
	int	fd_out;
	int	pipe_fd[2];
	pid_t pid1;
	pid_t pid2;
	int status1;
	int status2;
	
	if (argc != 5)
	{
		write (2, "./pipex file1 cmd1 cmd2 file2\n", 29);
		return (1);
	}
	
	fd_in = open(argv[1], O_RDONLY);
	if (fd_in == -1)
		error("Opening input file\n", NULL, NULL, 0);	
	if (pipe(pipe_fd) == -1)
	{
		close(fd_in);
		error("Creating pipe\n", NULL, NULL, 1);
	}
	//pid1 = -1;
	pid1 = fork();
	if (pid1 == -1)
	{
		/*close(fd_in);
		close (pipe_fd[0]);
		close (pipe_fd[1]);*/
		close_all(fd_in, -1, pipe_fd);
		error("Forking cmd1\n", NULL, NULL, 1);
	}
	if (pid1 == 0)
	{
		
		close (pipe_fd[0]);
		execute_cmd(argv[2], fd_in, pipe_fd[1], envp);
	}
	close(fd_in);
	fd_out = open(argv[4], O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd_out == -1)
	{
		close_all(-1, -1, pipe_fd);
		waitpid(pid1, NULL, 0);
		error ("Opening output file\n", NULL, NULL, 1);
	}
	pid2 = fork();
	if (pid2 == -1)
	{
		close_all(-1, fd_out, pipe_fd);
		error("Forking cmd2\n", NULL, NULL, 1);
	}
	if (pid2 == 0)
	{
		close (pipe_fd[1]);
		execute_cmd(argv[3], pipe_fd[0], fd_out, envp);
	}
	close_all(-1, fd_out, pipe_fd);
	waitpid(pid1, &status1, 0);
	waitpid(pid2, &status2, 0);
	int exit_status2 = (status2 >> 8) & 0xFF;
	if (exit_status2 == 127)
		return (127);
	if ( exit_status2 == 0)
	{
		return 0;
	}
	return 1;
}

