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
              << "   --iterations N,    Number of iterations (default: 100)\n"
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
        unsigned long iterations = 100;
        unsigned long threads = 4;

        /* Argument parsing (ugly code) */
        std::vector<std::string> args(argv + 1, argv + argc);
        for (size_t i = 0; i < args.size(); i++) {
                if (args[i] == "--alloc-rnd" || args[i] == "--alloc-seq" ||
                    args[i] == "--free-rnd" || args[i] == "--free-seq" ||
                    args[i] == "--latency" || args[i] == "--stress") {
                        benchmark = args[i].substr(2);
                } else if (args[i] == "--multi") {
                        is_multi = true;
                } else if (args[i] == "--iterations") {
                         if (i + 1 < args.size()) {
                                iterations = std::stoul(args[++i]);
                        } else {
                                std::cerr << "Error: --iterations requires a number" << std::endl;
                                return 1;
                        }
                } else if (args[i] == "--threads") {
                         if (i + 1 < args.size()) {
                                threads = std::stoul(args[++i]);
                        } else {
                                std::cerr << "Error: --threads requires a number" << std::endl;
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

        /* Run the corressponding benchmark */
        int res = 0;
        std::ofstream out_file(output, std::ios::out | std::ios::binary);

        if (benchmark == "alloc-rnd") {
                res = is_multi ? alloc_rnd_multi(out_file, iterations, threads):
                        alloc_rnd_single(out_file, iterations);
        } else if (benchmark == "alloc-seq") {
                res = is_multi ? alloc_seq_multi(out_file, iterations, threads):
                        alloc_seq_single(out_file, iterations);
        } else if (benchmark == "free-rnd") {
                res = is_multi ? free_rnd_multi(out_file, iterations, threads):
                        free_rnd_single(out_file, iterations);
        } else if (benchmark == "free-seq") {
                res = is_multi ? free_seq_multi(out_file, iterations, threads):
                        free_seq_single(out_file, iterations);
        } else if (benchmark == "stress") {
                res = is_multi ? stress_multi(out_file, iterations, threads):
                        stress_single(out_file, iterations);
        } else {
                std::cerr << "Unknown benchmark: " << benchmark << std::endl;
                res = 1;
        }
        
        if (!res) {
                std::cout << "Success: " << benchmark;
                if (is_multi) {
                        std::cout << ": multi-threaded /w "
                                  << threads << " threads";
                } else {
                        std::cout << ": single-threaded";
                }
                std::cout << " /w " << iterations << " iterations" << std::endl;

                /* Write to file */
                out_file.flush();
                out_file.close();

                std::cout << "Results are saved to '" << output << "'" << std::endl;
        }

        return res;
}