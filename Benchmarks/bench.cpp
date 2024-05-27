#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "bench.hpp"

void show_help() {
    std::cout << "Usage: ./bench [benchmark] [options]\n"
              << "Benchmarks:\n"
              << "   --alloc-rnd,       Run random allocation benchmark\n"
              << "   --alloc-seq,       Run sequential allocation benchmark\n"
              << "   --free-rnd,        Run random free benchmark\n"
              << "   --free-seq,        Run sequential free benchmark\n"
              << "   --stress,          Run stress test\n"
              << "\n"
              << "Options:\n"
              << "   --multi,           Multi-threaded\n"
              << "   --threads N,       Thread count (default: 4)\n"
              << "   --duration S,      Duration for the benchmark (default: 30)\n"
              << "   --output FILE,     Output file (default: results.txt)\n"
              << "   --help,            Show this help message\n"
              << std::endl;
}

int main(int argc, char *argv[])
{
        if (argc < 2) {
                show_help();
                return 1;
        }

        /* Default values */
        std::string benchmark = "latency";
        std::string output = "results.txt";

        bool is_multi = false;

        unsigned tc = 4;
        unsigned dur = 30;

        /* Argument parsing (ugly code) */
        std::vector<std::string> args(argv + 1, argv + argc);

        for (size_t i = 0; i < args.size(); i++) {
                if (args[i] == "--alloc-rnd" || args[i] == "--alloc-seq" ||
                    args[i] == "--free-rnd" || args[i] == "--free-seq" ||
                    args[i] == "--latency" || args[i] == "--stress") {
                        benchmark = args[i].substr(2);
                } else if (args[i] == "--multi") {
                        is_multi = true;
                } else if (args[i] == "--threads") {
                         if (i + 1 < args.size()) {
                                tc = std::stoul(args[++i]);
                        } else {
                                std::cerr << "Error: --threads requires a number" << std::endl;
                                return 1;
                        }
                } else if (args[i] == "--duration") {
                         if (i + 1 < args.size()) {
                                dur = std::stoul(args[++i]);
                        } else {
                                std::cerr << "Error: --duration requires a number" << std::endl;
                                return 1;
                        }
                } else if (args[i] == "--output") {
                        if (i + 1 < args.size()) {
                                output = args[++i];
                        } else {
                                std::cerr << "Error: --output requires a file name" << std::endl;
                                return 1;
                        }
                } else if (args[i] == "--help") {
                        show_help();
                        return 0;
                } else {
                        std::cerr << "Unknown argument: " << args[i] << std::endl;
                        show_help();
                        return 1;
                }
        }

        /* Verbose */
        std::cout << "Running '" << benchmark << "' with options:\n"
                  << "\tMulti-threaded: " << is_multi << "\n";
        if (is_multi) {
                std::cout << "\tThread: " << tc << "\n";
        }
        std::cout << "\tDuration: " << dur << "s\n"
                  << "\tOutput: " << output << std::endl;

        /* Run the corressponding benchmark */
        std::ofstream ofs(output, std::ios::out | std::ios::binary);
        int res = 0;

        if (benchmark == "alloc-rnd") {
                res = is_multi ? alloc_rnd_multi(ofs, dur, tc):
                        alloc_rnd_single(ofs, dur);
        } else if (benchmark == "alloc-seq") {
                res = is_multi ? alloc_seq_multi(ofs, dur, tc):
                        alloc_seq_single(ofs, dur);
        } else if (benchmark == "free-rnd") {
                res = is_multi ? free_rnd_multi(ofs, tc):
                        free_rnd_single(ofs);
        } else if (benchmark == "free-seq") {
                res = is_multi ? free_seq_multi(ofs, tc):
                        free_seq_single(ofs);
        } else if (benchmark == "stress") {
                res = is_multi ? stress_multi(ofs, dur, tc):
                        stress_single(ofs, dur);
        } else {
                std::cerr << "Unknown benchmark: " << benchmark << std::endl;
                res = 1;
        }
        
        if (!res) {
                /* Write to file */
                ofs.flush();
                std::cout << "Saving results to: " << output << std::endl;
        } else {
                ofs.clear();
                std::cout << "Something went wrong: " << res << std::endl;
        }

        ofs.close();

        return res;
}