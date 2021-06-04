// make_csv.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//
#define _CRT_SECURE_NO_WARNINGS	

#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include <regex>
#include <vector>

#include <stdio.h>
#include <string.h>

#define	MAX_ROW_COUNT	10000000000
#define	MAX_COL_COUNT	512

using namespace std;

char drive[512];
char dir[512];
char prog_name[512];
char ext[512];
int	 err_id;
int  parameter_line;
char parameter_name[2050];
char buff[2048];

char file_name[2055];
char file_dir[512];

int64_t max_row_count = 0;
int64_t row_count = 0;
int  max_col_count = 0;
int  col_count = 0;
int64_t  col_size[512];

struct col_define
{
	int		col_type;
	int64_t	col_size;
	char	col_name[512];
} col_define_area[513];

bool output_csv = false;
char output_separator = '\t';


// Used message out
void used_mesg()
{
	cerr << "Used : " << prog_name << " <parameter file name>\n";
}
void error_mesg(int err_id)
{
	string err_mesg;

	switch (err_id)
	{
	case 1:
		err_mesg = "parameter nothing error";
		break;
	case 2:
		err_mesg = "parameter file multiple error";
		break;
	case 3:
		err_mesg = "parameter file read error";
		break;
	case 4:
		err_mesg = "parameter file define error";
		break;
	case 5:
		err_mesg = "parameter file define colmun type error";
		break;
	case 6:
		err_mesg = "parameter file define colmun size error";
		break;
	case 7:
		err_mesg = "parameter file define colmun name error";
		break;
	case 8:
		err_mesg = "parameter file define colmun count over error";
		break;
	case 9:
		err_mesg = "parameter file write error";
		break;
	default:
		err_mesg = "non error id";
		break;
	}
	cerr << err_mesg.c_str() << "\n";
}

// split tab code token
vector<string> token_split(char* buff)
{
	vector<string> parameter = {};
	string s = buff;   // 分割対象の文字列
	regex separator{ "\t" }; // 分割用のセパレータ
	auto ite = sregex_token_iterator(s.begin(), s.end(), separator, -1);
	auto end = sregex_token_iterator();
	while (ite != end) {
		parameter.push_back(*ite++);     // 分割文字列を格納
	}
	return parameter;
}

// cut cr code & replace space to tab code buffer
// cut continue tab code to one
void cut_buff(char *buff)
{
	char *pt = buff;
	char c;
	while ((c = *buff))
	{
		switch(c)
		{
		case ' ':
			*buff = '\t';
			break;
		case '\n':
			*buff = NULL;
			break;
		case '\f':
			*buff = NULL;
		}
		*buff++;
	}
	while ((c = *pt))
	{
		if (c == '\t' && pt[1] == '\t')
		{
			strcpy(pt, pt + 1);
		}
		else
		{
			pt++;
		}
	}
}

// command data set
int command_set(char *buff)
{
	vector<string> command = token_split(buff);
	if (command.size() > 0)
	{
		string cmd = command[0];
		transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
		if (cmd == "//CSV")
		{
			output_csv = true;
			output_separator = ',';
			return 0;
		}
		else if (cmd == "//TAB")
		{
			output_csv = false;
			output_separator = '\t';
			return 0;
		}
		else if (cmd == "//MAX_ROW_COUNT")
		{
			if (all_of(command[1].cbegin(), command[1].cend(), isdigit))
			{
				int64_t size = stoll(command[1]);
				if (size <= 0)
				{
					// error
					return -1;
				}
				else if (size > MAX_ROW_COUNT)
				{
					// error
					return -1;
				}
				else
				{
					max_row_count = size;
					return 0;
				}
			}
			else
			{
				// error
				return -1;
			}
		}
		return 1;
	}
	return 1;
}

void comment_cut(char *buff)
{
	for (int64_t i = strlen(buff) - 1; i > 0; i--)
	{
		if (buff[i] == '/' && buff[i - 1] == '/') {
			i--;
			buff[i] = NULL;
		}
		if (i <= 0L)
		{
			break;
		}
	}
}

bool col_type_check(string data, int *type)
{
	if (data == "I")
	{
		*type = 1;
		return false;
	}
	if (data == "F")
	{
		*type = 2;
		return false;
	}
	if (data == "A")
	{
		*type = 7;
		return false;
	}
	return true;
}

bool col_size_check(string data, int64_t *size)
{
	if (all_of(data.cbegin(), data.cend(), isdigit))
	{
		int64_t ll = stoll(data);
		if (ll <= 0)
		{
			// error
			return true;
		}
		else if (ll > MAX_ROW_COUNT)
		{
			// error
			return true;
		}
		else
		{
			*size = ll;
			return false;
		}
	}
	// error
	return true;
}

bool col_name_check(string data, char *name)
{
	if (data.length() > 255)
	{
		return true;
	}
	strcpy(name, data.c_str());
	return false;
}

bool parameter_setting(char *parameter_buff)
{
	char p_drive[512];
	char p_dir[512];
	char file_name[512];
	char p_ext[512];

	_splitpath(parameter_buff, p_drive, p_dir, file_name, p_ext);

	// Drive
	if (p_drive[0] == NULL)
	{
		strcpy(p_drive, drive);
	}
	// Directory
	if (p_dir[0] == NULL)
	{
		strcpy(p_dir, dir);
	}
	// File name
	if (file_name[0] == NULL)
	{
		return false;
	}
	strcpy(parameter_name, p_drive);
	strcat(parameter_name, p_dir);
	strcat(parameter_name, file_name);
	strcat(parameter_name, p_ext);

	FILE *fp;
	if ((fp = fopen(parameter_name, "r")) == NULL)
	{
		// エラー発生
		return false;
	}
	parameter_line = 0;
	col_count = 0;
	while (fgets(buff, 1024, fp))
	{
		parameter_line++;

		cut_buff(buff);
		int ret;
		if ((ret = command_set(buff)) <= 0)
		{
			if (ret < 0)
			{
				// error
				err_id = 4;
				error_mesg(err_id);
				cerr << "parameter file error line : " << parameter_line << endl;
			}
			continue;
		}
		comment_cut(buff);
		if (buff[0] == '\t' || buff[0] == NULL)
		{
			continue;
		}

		vector<string> parameter = token_split(buff);
		if (parameter.size() > 3)
		{
			// error
			err_id = 4;
			error_mesg(err_id);
			cerr << "parameter file error line : " << parameter_line << endl;
			continue;
		}
		if (col_type_check(parameter[0], &col_define_area[col_count].col_type))
		{
			// error
			err_id = 5;
			error_mesg(err_id);
			cerr << "parameter file error line : " << parameter_line << endl;
			continue;
		}
		if (col_size_check(parameter[1], &col_define_area[col_count].col_size))
		{
			// error
			err_id = 6;
			error_mesg(err_id);
			cerr << "parameter file error line : " << parameter_line << endl;
			continue;
		}
		if (col_name_check(parameter[2], &col_define_area[col_count].col_name[0]))
		{
			// error
			err_id = 7;
			error_mesg(err_id);
			cerr << "parameter file error line : " << parameter_line << endl;
			continue;
		}
		if (++col_count > MAX_COL_COUNT)
		{
			// error
			err_id = 8;
			error_mesg(err_id);
			cerr << "parameter file error line : " << parameter_line << endl;
			col_count = 512;
			break;
		}
	}
	fclose(fp);
	max_col_count = col_count;
	return true;
}

int output_data()
{
	strcpy(file_name, parameter_name);
	if (output_csv)
	{
		strcat(file_name, ".csv");
	}
	else
	{
		strcat(file_name, ".txt");
	}
	FILE *fp;
	if ((fp = fopen(file_name, "w")) == NULL)
	{
		// エラー発生
		err_id = 9;
		used_mesg();
		error_mesg(err_id);
		return(err_id);
	}
	memset(col_size, 0, sizeof(col_size));
	for (int64_t line = 0; line < max_row_count; line++)
	{
		for (col_count = 0; col_count < max_col_count; col_count++)
		{
			if (col_count > 0)
			{
				fputc(output_separator, fp);
			}

			int64_t max_size = col_define_area[col_count].col_size;
			int64_t data = col_size[col_count];

			switch (col_define_area[col_count].col_type)
			{
			case 1:
				fprintf(fp, "%lld", data);
				break;
			case 2:
				fprintf(fp, "%.3lf", ((double)data) / 1000);
				break;
			default:
				fprintf(fp, "\"%010lld\"", data + 1);
				break;
			}

			data++;
			if (data >= max_size)
			{
				data = 0;
			}
			col_size[col_count] = data;

		}
		fputc('\n', fp);
	}
	fclose(fp);
	return(0);
}

int main(int argc, char **argv)
{
	_splitpath(argv[0], drive, dir, prog_name, ext);
	if (argc <= 1)
	{
		// parameter nothing error
		err_id = 1;
		used_mesg();
		error_mesg(err_id);
		return(err_id);
	}
	if (argc > 2)
	{
		// parameter multiple error
		err_id = 2;
		used_mesg();
		error_mesg(err_id);
		return(err_id);
	}
	// parameter setting
	if (!parameter_setting(argv[1]))
	{
		// parameter file error
		err_id = 3;
		used_mesg();
		error_mesg(err_id);
		return(err_id);
	}

	//for (int i = 0; i < max_col_count; i++)
	//{
	//	cout << i << ":" << col_define_area[i].col_name << " " << col_define_area[i].col_type << " (" << col_define_area[i].col_size << ")\n";
	//}
	int ret = output_data();
	return ret;
}
