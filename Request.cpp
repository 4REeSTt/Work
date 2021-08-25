#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <list>
#include <vector>




//Global Vars

bool END_OF_PROGRAM = 0;
const int NumberOfThreads = 2;

class Request;
//Imitating incoming requests
std::list<Request*> incomming_list;

size_t current_request = 0;

//For correct output and request_list access
std::mutex request_list_access;
std::mutex output_access;





class Request {
public:
    Request(int value)
    :m_value(value)
    {}

    void Run() {
        output_access.lock();
        std::cout << "Run request" << m_value << std::endl;
        output_access.unlock();

        //imitating long calculation
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

private:
    int m_value;
};

Request* GetRequest() throw() {
    if (!(int)incomming_list.size())
        return nullptr;

    Request* current = incomming_list.back();
    incomming_list.pop_back();
    return current;
}


void ProcessRequest(Request* request) throw() {
    request->Run();
}


//Aditional threads
void WorkinkThread(std::vector<Request*> &request_list) {
    //get requests and run them using processrequest
    while (!END_OF_PROGRAM) {

        //Get request from list
        request_list_access.lock();

        if (request_list.size() <= current_request) {
            request_list_access.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }
        Request* request = request_list[current_request++];

        request_list_access.unlock();

        //processrequest(request);     
        ProcessRequest(request);
    }
}



//Feeling incomming_list with new requests
void init() {
    for (int req = 0; req < 30; req++)
        incomming_list.push_back(new Request(req));
}

int main()
{
    init();

    //1.Запустить несколько рабочих потоков
    std::thread* thread_list[NumberOfThreads];
    std::vector<Request*> request_list;

    for (int thread = 0; thread < NumberOfThreads; thread++)
        thread_list[thread] = new std::thread(WorkinkThread, std::ref(request_list));

    
    //2.Класть в очередь функции для выполнения пока GetRequest() не вернет nullptr
    //Можно установить другое условие завершения основного цикла если планируется пополнение incomming_requests(в условии не указано)
    while (true) {
        Request* current_request = GetRequest();
        if (current_request == nullptr) 
            break;

        request_list.push_back(current_request);
    }

    //Just for exaple(shows that threads're running and quitting correct)
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    END_OF_PROGRAM = 1;

    //3.Дождаться выполнения текущего ProcessRequest потоками. Остановить рабочие потоки/удалить.
    for (int thread = 0; thread < NumberOfThreads; thread++) {
        thread_list[thread]->join();
        delete thread_list[thread];
    }


    //4.Quit. Memory clearing
    for (auto request : request_list)
        delete request;

    std::cout << "All done\n";

    return 0;
}
