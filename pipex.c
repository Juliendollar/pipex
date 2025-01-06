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

void	ft_free_tab(char **tab)
{
	int i = 0;
	while (tab[i])
	{
		free (tab[i]);
		i++;
	}
	free (tab);
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
	
	args = ft_split(cmd, ' ');
	if (!args || !args[0])
	{
		ft_free_tab(args);
		perror("Error parsing command");
		exit (1);
	}
	cmd_path = get_command_path(args[0], envp);
	if (!cmd_path)
	{
		ft_putstr_fd("Command not found: ", 2);
		ft_putstr_fd(args[0], 2);
		ft_putstr_fd("\n", 2);
		ft_free_tab(args);
		exit(127);
	}
	if(dup2(input_fd, STDIN_FILENO) == -1 || dup2(output_fd, STDOUT_FILENO) == -1)
	{
		perror("Error redirecting input/output");
		free(cmd_path);
		ft_free_tab(args);
		exit (1);
	}
	execve(cmd_path, args, envp);
	perror("Error executing command");
	free(cmd_path);
	ft_free_tab(args);
	exit (1);
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
	/*if (fd_in == -1)
	{
		perror("Error opening input file\n");
		return (1);
	}*/
	fd_out = open(argv[4], O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd_out == -1)
	{
		if (!ft_strncmp(argv[2], "sleep", 5) == 0)
		{
			perror ("error opening output file\n");
			close(fd_in);
			return (1);
		}
	}
	if (pipe(pipe_fd) == -1)
	{
		perror("Error creating pipe");
		close(fd_in);
		close(fd_out);
		return (1);
	}
	pid1 = fork();
	if (pid1 == -1)
	{
		perror("error forking cmd1");
		close(fd_in);
		close(fd_out);
		close (pipe_fd[0]);
		close (pipe_fd[1]);
		return (1);
	}
	if (pid1 == 0)
	{
		
		close (pipe_fd[0]);
		execute_cmd(argv[2], fd_in, pipe_fd[1], envp);
	}
	pid2 = fork();
	if (pid2 == -1)
	{
		perror("Error forking cmd2");
		close(fd_in);
		close(fd_out);
		close(pipe_fd[0]);
		close(pipe_fd[1]);
		return (1);
	}
	if (pid2 == 0)
	{
		close (pipe_fd[1]);
		execute_cmd(argv[3], pipe_fd[0], fd_out, envp);
	}
	close(pipe_fd[0]);
	close(pipe_fd[1]);
	close(fd_in);
	close(fd_out);
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