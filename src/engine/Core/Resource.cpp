//
// Created by macola on 7/28/26.
//

#include "Resource.h"
#include "SmoothieCore.h"

using namespace Smoothie;


using namespace Smoothie::internal;
static void f_load(std::string filepath, std::atomic<ResourceThread_Status>& thread_status, std::shared_ptr<ResourceEntry> rss)
{
    //1. RAII thread guard.
    auto _thread_guard = ResourceThread_Guard(thread_status);
    if (rss == nullptr) return;
    try
    {
        //2. Make sure that object is in updating status.
        assert(rss->current_status.load() == ResourceEntry_Status_Updating);

        //3. Start loading resource.
        if (rss->resource->load(filepath) == Resource_Loading_Status_Fail)
        {
            SmoothieCore::logWarning("File: " + filepath + " didn't loaded properly.");

            //If error shows up, set error status and destroy resource, notify every waiting thread.
            rss->current_status.store(ResourceEntry_Status_Error);
            rss->cv.notify_all();
            rss->resource->destroy();
            return;
        }

        //4.Loading completed, resource is ready for reading, notify waiting all threads.
        rss->current_status.store(ResourceEntry_Status_Idle);
        rss->cv.notify_all();
        SmoothieCore::logVerbose_Formated(R"(File "{}" loaded.)", filepath);

    }
    catch (std::exception& e)
    {
        SmoothieCore::logError(e.what());
    }
}

void ResourceThreadPool::manager_work()
{
    while (manager_running.load())
    {
        std::unique_lock<std::mutex> lock(m_Jobs_Mutex);

        //1. Check if there is any pending work to be done.
        if (m_Jobs.empty() == true)
        {
            if (lock.owns_lock()) lock.unlock();

            std::this_thread::sleep_for(std::chrono::milliseconds(50)); //Give manager some rest before checking again.
            continue;
        }

        auto& _job = m_Jobs.front();

        //2. Find jobless thread and make it do some work.
        bool thread_search = true;
        std::size_t thread_index = 0;
        while (thread_search)
        {
            auto& _thread = m_Threads[thread_index];

            if (_thread.status.load() == ResourceThread_Status_Idle)
            {
                if (_thread.thread.joinable()) _thread.thread.join();
                switch (_job.type)
                {
                    case ResourceThread_JobType_Load:
                    {
                        auto _t = std::thread(f_load, std::move(_job.filepath), std::ref(_thread.status), _job.entry);
                        _thread.thread = std::move(_t);
                        _thread.status.store(ResourceThread_Status_Running);
                    }break;

                    case ResourceThread_JobType_Reload:
                    {

                    }break;


                    case ResourceThread_JobType_Destroy:
                    {

                    }break;
                }
                thread_search = false;
            }

            if (thread_index == m_Threads.size() - 1)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); //All threads are busy, wait a bit then try again.
            }

            thread_index = (thread_index + 1) % m_Threads.size();
        }

        //3. This one is finished, move to the next job.
        m_Jobs.pop();
    }
}

void ResourceThreadPool::add_job(ResourceThread_JobType type, const std::string& filepath, std::shared_ptr<ResourceEntry> resource)
{
    std::lock_guard<std::mutex> lock(m_Jobs_Mutex);
    m_Jobs.push({type, filepath, resource});
}

ResourceThreadPool::~ResourceThreadPool()
{
    while (m_Jobs.size() != 0)
    {
        //Other threads are still running, wait for them to finish.
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    for (auto& _thread : m_Threads)
    {
        if (_thread.thread.joinable()) _thread.thread.join();
    }

    manager_running.store(false);
    if (m_ManagerThread.joinable())
    {
        m_ManagerThread.join();
    }
}