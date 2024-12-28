#include<iostream>
#include<filesystem>
#include<vector>
#include<string>
#include<thread>
#include<Windows.h>

using namespace std;
namespace fs = std::filesystem;

class Solution
{
public:
	Solution(string self_name)
	{
		string current_dir = fs::current_path().string();
		for (const auto& file_path : fs::recursive_directory_iterator(current_dir))
		{
			if (!(file_path.is_directory()) and (file_path.path().string() != self_name))
			{
				m_files.push_back(file_path.path().string());
			}
		}
		m_filesize = m_files.size();
	}

	void convert1()
	{
		string temp1;
		for (string file : m_files)
		{
			temp1 = file + ".txt";
			rename(file.c_str(), temp1.c_str());
			++m_pos1;
			printf("%s convert 1 %d\n", file.c_str(), m_pos1);
		}
	}

	void convert2()
	{
		Sleep(500);
		string file;
		string temp1;
		string temp2;
		while (true)
		{
			if (m_pos2 < m_pos1)
			{
				file = m_files[m_pos2];
				temp1 = file + ".txt";
				temp2 = file + ".temp";
				m_cmd = "move \"" + temp1 + "\" \"" + temp2 + "\" >nul";
				system(m_cmd.c_str());
				++m_pos2;
				printf("%s convert 2 %d\n", temp1.c_str(), m_pos2);
			}
			else if (m_pos2 == m_filesize)
			{
				break;
			}
		}
	}

	void convert3()
	{
		Sleep(1000);
		string file;
		string temp2;
		while (true)
		{
			if (m_pos3 < m_pos2)
			{
				file = m_files[m_pos3];
				temp2 = file + ".temp";
				rename(temp2.c_str(), file.c_str());
				++m_pos3;
				printf("%s convert 3 %d\n", temp2.c_str(), m_pos3);
			}
			else if (m_pos3 == m_filesize)
			{
				break;
			}
		}
	}

private:
	vector<string> m_files;
	string m_cmd;
	int m_filesize;
	int m_pos1 = 0;
	int m_pos2 = 0;
	int m_pos3 = 0;
};

int main(int argc, char* argv[])
{
	string self_name = fs::path(argv[0]).string();
	Solution s1(self_name);
	thread convertthread1(&Solution::convert1, &s1);
	thread convertthread2(&Solution::convert2, &s1);
	thread convertthread3(&Solution::convert3, &s1);
	convertthread1.join();
	convertthread2.join();
	convertthread3.join();
}
