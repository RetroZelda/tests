#include <functional>
#include <algorithm>
#include <chrono>
#include <vector>
#include <string>

#include <stdio.h>
#include <float.h>
#include <string.h>

unsigned long long g_counter = 0;

void Operation(int depth)
{
    ++g_counter; // ensures nothing gets optimized away
    if(depth > 0)
    {
        Operation(depth - 1);
    }
}

void RunOperation(int depth)
{
    Operation(depth);
}

struct SFunctorMember 
{
  SFunctorMember(int depth) : m_depth(depth) {}
  void operator()() const { RunOperation(m_depth);  }

private:
  int m_depth;
};

struct SFunctorParam
{
  SFunctorParam() {}
  void operator()(int depth) const { RunOperation(depth);  }

private:
};

void TestFunc(int depth)
{
    RunOperation(depth);
}

void RunTests(int run_depth, int time_to_run)
{
    // defines all our test types
    std::function<void(int)> lambda_in_function_test = [](int depth) { RunOperation(depth); };
    std::function<void(int)> bind_in_function_test = std::bind(&TestFunc, std::placeholders::_1);
    auto lambda_in_auto_test = [](int depth) { RunOperation(depth); };    
    auto bind_in_auto_test = std::bind(&TestFunc, std::placeholders::_1);
    void(*function_pointer_test)(int) = &TestFunc;
    SFunctorMember member_test(run_depth);
    SFunctorParam param_test;
        
    // result vars
    int run_counter;
    std::vector<std::pair<std::string, double>> test_results;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time, end_time;
    std::chrono::duration<double, std::milli> duration;
    
    // run a regular function call as a control
    run_counter = time_to_run;
    start_time = std::chrono::high_resolution_clock::now();
    while(--run_counter > 0)
    {
        TestFunc(run_depth);
    }
    end_time = std::chrono::high_resolution_clock::now();
    duration = end_time - start_time;
    test_results.push_back(std::make_pair("function call", duration.count()));
    
    // run the lambda held in std::function
    run_counter = time_to_run;
    start_time = std::chrono::high_resolution_clock::now();
    while(--run_counter > 0)
    {
        lambda_in_function_test(run_depth);
    }
    end_time = std::chrono::high_resolution_clock::now();
    duration = end_time - start_time;
    test_results.push_back(std::make_pair("std::function lambda", duration.count()));
    
    // run a std::bind held in std::function
    run_counter = time_to_run;
    start_time = std::chrono::high_resolution_clock::now();
    while(--run_counter > 0)
    {
        bind_in_function_test(run_depth);
    }
    end_time = std::chrono::high_resolution_clock::now();
    duration = end_time - start_time;
    test_results.push_back(std::make_pair("std::function std::bind", duration.count()));
    
    // run the lambda held in auto
    run_counter = time_to_run;
    start_time = std::chrono::high_resolution_clock::now();
    while(--run_counter > 0)
    {
        lambda_in_auto_test(run_depth);
    }
    end_time = std::chrono::high_resolution_clock::now();
    duration = end_time - start_time;
    test_results.push_back(std::make_pair( "auto lambda", duration.count()));
    
    // run the std::bind held in auto
    run_counter = time_to_run;
    start_time = std::chrono::high_resolution_clock::now();
    while(--run_counter > 0)
    {
        bind_in_auto_test(run_depth);
    }
    end_time = std::chrono::high_resolution_clock::now();
    duration = end_time - start_time;
    test_results.push_back(std::make_pair("auto std::bind", duration.count()));
    
    // run a raw function pointer 
    run_counter = time_to_run;
    start_time = std::chrono::high_resolution_clock::now();
    while(--run_counter > 0)
    {
        function_pointer_test(run_depth);
    }
    end_time = std::chrono::high_resolution_clock::now();
    duration = end_time - start_time;
    test_results.push_back(std::make_pair("function pointer", duration.count()));
    
    // run a functor that utilizes a member variable
    run_counter = time_to_run;
    start_time = std::chrono::high_resolution_clock::now();
    while(--run_counter > 0)
    {
        member_test();
    }
    end_time = std::chrono::high_resolution_clock::now();
    duration = end_time - start_time;
    test_results.push_back(std::make_pair("functor member", duration.count()));
    
    // run a functor that accepts a param
    run_counter = time_to_run;
    start_time = std::chrono::high_resolution_clock::now();
    while(--run_counter > 0)
    {
        param_test(run_depth);
    }
    end_time = std::chrono::high_resolution_clock::now();
    duration = end_time - start_time;
    test_results.push_back(std::make_pair("functor param", duration.count()));

    // sort resutls quickest -> slowest
    std::sort(test_results.begin(), test_results.end(), [](const std::pair<std::string, double>& left, const std::pair<std::string, double>& right)
    {
        return left.second < right.second;
    });
    
    // determin longest sting for formatting(slow.  oh well)
    unsigned int longest = 0;
    for(const std::pair<std::string, double>& result_pair : test_results)
    {
        if(result_pair.first.size() > longest)
        {
            longest = result_pair.first.size();
        }
    }

    // display results
    int counter = 0;
    char buffer[128];
    unsigned int header_length;
    sprintf(buffer, "Running %d Deep %d Times%n", run_depth, time_to_run, &header_length);
    if(header_length > longest) longest = header_length;
    printf("-----------------------------------------------------------------------------------\n");
    printf("#| %-*s | Total\t\t\t| Average\n", longest, buffer);
    printf("-----------------------------------------------------------------------------------\n");
    for(const std::pair<std::string, double>& result_pair : test_results)
    {
        printf("%d| %-*s | %.*f ms\t| %.*f ms\n", counter++, longest, result_pair.first.c_str(), DBL_DIG, result_pair.second, DBL_DIG, result_pair.second / (double)time_to_run);
    }
    printf("-----------------------------------------------------------------------------------\n");
}


int main(int argc, char* argv[])
{
    if(argc < 3)
    {
        printf("Usage: %s [depth] [iterations]\n\texample: %s 100 1000\n", argv[0], argv[0]);
        return 1;
    }
    
    int depth = atoi(argv[1]);
    int iterations = atoi(argv[2]);
    RunTests(depth, iterations);
    
    //RunTests(100, 10);
    //RunTests(100, 100);
    //RunTests(100, 1000);
    //RunTests(100, 10000);
    //RunTests(100, 100000);
    //RunTests(100, 1000000);
    //RunTests(100, 10000000);
    //RunTests(100, 100000000);    
    // printf("Counter Value: %llu\n", g_counter);
    return 0;
}
