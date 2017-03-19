/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */

#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#include <syslog.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <signal.h>
#include "debug.h"
#include "config_file_ops.h"
#include "log.h"

#define COMMENT_CHARACTER			'#'
#define LINE_SIZE				(768)


static char config_file_path[MAX_PATH_LEN];
char (* ga_variables)[MAX_VAR_NAME_LEN + 1];
char (* ga_values)[MAX_VAR_VALUE_LEN + 1];
char (* ga_values_backup)[MAX_VAR_VALUE_LEN + 1];
int  g_var_num;

#define VAR_NAME_SIZE   (MAX_VAR_NUM)*(MAX_VAR_NAME_LEN + 1)
#define VAR_VALUE_SIZE  (MAX_VAR_NUM)*(MAX_VAR_VALUE_LEN + 1)

void free_config()
{
    g_var_num = 0;
    free(ga_variables); ga_variables = NULL;
    free(ga_values);    ga_values = NULL;
    if (ga_values_backup)
    {
        free(ga_values_backup);    ga_values_backup = NULL;
    }
}

void get_dir_path_of_file(char *file, char *dir_path)
{
	char *temp;
    strncpy(dir_path, file, MAX_PATH_LEN);
    temp = strrchr(dir_path, '/');
    temp[0] = '\0';

}

void remove_trailing_chars(char *path, char c)
{
	size_t len;

	len = strlen(path);
	while (len > 0 && path[len-1] == c)
		path[--len] = '\0';
}

int get_key(char **line, char **key, char **value)
{
	char *linepos;
	char *temp;

	linepos = *line;
	if (!linepos)
    {   
		return -1;
    }

	/* skip whitespace */
	while (isspace(linepos[0]))
		linepos++;

	if (linepos[0] == '\0')
    {   
		return -1;
    }

	/* get the key */
	*key = linepos;
	while (1) {
		linepos++;
		if (linepos[0] == '\0')
        {
			return -1;
        }
		if (isspace(linepos[0]))
			break;
		if (linepos[0] == '=')
			break;
	}

	/* terminate key */
	linepos[0] = '\0';

	while (1) {
		linepos++;
		if (linepos[0] == '\0')
        {
			return -1;
        }
		if (isspace(linepos[0]))
			continue;
		if (linepos[0] == '=')
			continue;
        break;
	}


	/* get the value*/
	if (linepos[0] == '"')
    {   
		linepos++;
    }
	else
    {
		return -1;
    }
	*value = linepos;

	temp = strchr(linepos, '"');
	if (!temp)
    {
		return -1;
    }
    
	temp[0] = '\0';
	return 0;
}

void value_copy(void *d, void *s, int var_num)
{
    memcpy(d, s, var_num*(MAX_VAR_VALUE_LEN + 1));
}

int parse_config_file(char *path_to_config_file, int need_update)
{
	char line[LINE_SIZE + 2];
	char *bufline;
	char *linepos;
	char *variable;
	char *value;

	size_t count;
	int lineno;
	int retval = 0;

    FILE *cfg_file;

    strcpy(config_file_path, path_to_config_file);
    cfg_file = fopen(config_file_path, "r");
	if (NULL == cfg_file) 
    {
		ErrSysLog("can't open '%s' as config file", path_to_config_file);
		goto EXIT;
	}
    
    ga_variables = malloc(VAR_NAME_SIZE);
    ga_values    = malloc(VAR_VALUE_SIZE);
    if (need_update)
        ga_values_backup = malloc(VAR_VALUE_SIZE);

    if (NULL==ga_variables || NULL==ga_values || (need_update && NULL==ga_values_backup))
    {
        ErrSysLog("malloc failed");
        goto EXIT;
    }

	/* loop through the whole file */
	lineno = 0;

	while (NULL != fgets(line, sizeof(line), cfg_file)) 
    {
		lineno++;
		bufline = line;
		count = strlen(line);

		if (count > LINE_SIZE) 
        {
			ErrSysLog("line too long, conf line skipped %s, line %d", path_to_config_file, lineno);
			continue;
		}

		/* eat the whitespace */
		while ((count > 0) && isspace(bufline[0])) 
        {
			bufline++;
			count--;
		}
		if (count == 0)
			continue;

		/* see if this is a comment */
		if (bufline[0] == COMMENT_CHARACTER)
			continue;

		memcpy(line, bufline, count);
		line[count] = '\0';

		linepos = line;
		retval = get_key(&linepos, &variable, &value);
		if (retval != 0) 
        {
			ErrSysLog("error parsing %s, line %d:%d", path_to_config_file, lineno, (int)(linepos-line));
			continue;
		}

        if (g_var_num >= MAX_VAR_NUM)
        {
			ErrSysLog("too many vars in  %s, line %d:%d", path_to_config_file, lineno, (int)(linepos-line));
			continue;
		}

        if (strlen(variable) > MAX_VAR_NAME_LEN)
        {
			ErrSysLog("var name to long %s, line %d:%d", path_to_config_file, lineno, (int)(linepos-line));
			continue;
		}

        if (strlen(value) > MAX_VAR_VALUE_LEN)
        {
			ErrSysLog("value to long %s, line %d:%d", path_to_config_file, lineno, (int)(linepos-line));
			continue;
		}

		strncpy(ga_variables[g_var_num], variable, sizeof(ga_variables[g_var_num]));
        remove_trailing_chars(value, '/');
		strncpy(ga_values[g_var_num], value, sizeof(ga_values[g_var_num]));
        g_var_num++;
        continue;

	}

    if (ga_values_backup) 
        value_copy(ga_values_backup, ga_values, g_var_num);
EXIT:
	fclose(cfg_file);
	return g_var_num;
}

char * get_config_var(char *var_name)
{
    int i;
	for(i = 0; i < g_var_num; i++)
    {
		if (strcasecmp(ga_variables[i], var_name) == 0) 
        {
			return ga_values[i];
		}

    }
    ErrSysLog("get %s failed", var_name);
	return NULL;
    
}

int write_to_config_file()
{
    char (* values)[MAX_VAR_VALUE_LEN + 1] = ga_values;
    char config_file_tmp[MAX_PATH_LEN];
    FILE *cfg_file;
    int  i;
	char line[LINE_SIZE + 2];
    int line_len;

    sprintf(config_file_tmp, "%s.tmp", config_file_path);
    cfg_file = fopen(config_file_tmp, "w");
	if (NULL == cfg_file) 
    {
		ErrSysLog("can't open '%s' for write", config_file_tmp);
		return -1;
	}

    if (ga_values_backup) values = ga_values_backup;

	for(i = 0; i < g_var_num; i++)
    {
		line_len = sprintf(line, "%s=\"%s\"\n", ga_variables[i], values[i]);
        if (line_len != fwrite(line, 1, line_len, cfg_file))
        {
    		ErrSysLog("write '%s' failed", config_file_tmp);
            fclose(cfg_file);
    		return -1;
    	}

    }
    
    fclose(cfg_file);
    return rename(config_file_tmp, config_file_path);
}

int set_config_var(char *var_name, char *value)
{
    int i;
    char value_old[(MAX_VAR_VALUE_LEN + 1)];
	for(i = 0; i < g_var_num; i++)
    {
		if (strcasecmp(ga_variables[i], var_name) == 0) 
        {
            break;
		}

    }

    if (i >= g_var_num)
    {
        ErrSysLog("get %s failed", var_name);
        return -1;
    }
    
    strcpy(value_old, ga_values[i]);
    
    if (NULL==ga_values_backup) 
    {
		strcpy(ga_values[i], value);
        return 0;
    }

    strcpy(ga_values_backup[i], value);
    if (write_to_config_file())
    {
        strcpy(ga_values_backup[i], value_old);
        return -1;
    }
    
    strcpy(ga_values[i], value);

	return 0;
    
}

int set_config_var2(char *var_name, char *value)
{
    int i;
	for(i = 0; i < g_var_num; i++)
    {
		if (strcasecmp(ga_variables[i], var_name) == 0) 
        {
            break;
		}

    }

    if (i >= g_var_num)
    {
        ErrSysLog("get %s failed", var_name);
        return -1;
    }
    
    
    strcpy(ga_values_backup[i], value);
	return 0;
    
}

void print_all_vars()
{
    int i;
    DBG_PRINT("g_var_num == %d", g_var_num);
	for(i = 0; i < g_var_num; i++)
    {
        printf("%s = %s\n", ga_variables[i], ga_values[i]);

    }
    
    
}
