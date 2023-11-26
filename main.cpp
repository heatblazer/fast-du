#include <string>
#include <iostream>
#include <filesystem>
#include <vector>
#include <thread>
#include <unistd.h>
#include <cstdio>

namespace fs = std::filesystem;

//du -sh /folder

void filter_result_dush(const std::string_view& res)
{

}

struct ProcessOpen
{
    std::string cmd;
    std::string where;
    std::string result;

    ProcessOpen(const std::string_view& v, const std::string_view& where) :
        cmd {v}, where{where}, cmdPipe{nullptr} { }

    ~ProcessOpen() noexcept = default;

    bool open()
    {
#define MAX_PATH 1024

        cmdPipe = popen((cmd + where).c_str(), "r");
        if (!cmdPipe) return false;

        char path[MAX_PATH] = {0}, *pChar;
        pChar = path;
        while (fgets(path, MAX_PATH, cmdPipe) != NULL)
            result+=(std::string{pChar});

        if (pclose(cmdPipe) < 0) return false;
        cmdPipe = nullptr;

#undef MAX_PATH
        return true;

    }
private:
    FILE *cmdPipe;
};


int main()
{
    std::string path = "/home/ilian";
    std::vector<std::string> folders;
    std::vector<ProcessOpen> cmd_exec;
    std::vector<std::thread> workers;
    auto start = std::chrono::steady_clock::now();


    for (const auto & entry : fs::directory_iterator(path)) {
        if (entry.is_directory()) {
            cmd_exec.push_back(ProcessOpen{"du -sh ", entry.path().c_str()});
        }
    }

    for(auto& cmd : cmd_exec) {
        workers.push_back(std::thread{
                                          [&]()
            {
                if (cmd.open()) std::cout << "Ok started the worker\r\n";
                else std::cout << "Failed starting the worker\r\n";
            }
        });
   }

    for(auto& w : workers)  w.join();

    for (auto& cmd : cmd_exec) {
           std::cout << cmd.result;
    }
    auto finish = std::chrono::steady_clock::now();
    double elapsed_seconds = std::chrono::duration_cast<
                                 std::chrono::duration<double>>(finish - start).count();

    std::cout << "Elapsed time: " << elapsed_seconds << std::endl;
    return 0;
}
