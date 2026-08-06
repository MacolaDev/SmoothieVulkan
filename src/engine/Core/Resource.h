#pragma once
#include "Common.h"

namespace Smoothie
{
    enum Resource_Loading_Status: int
    {
        Resource_Loading_Status_Success,
        Resource_Loading_Status_Fail
    };

    //Regular users shouldn't use this. Use pre-derived classes down below for loading instead.
    struct Resource_Base
    {
        //Engine will call this method whenever it needs to load something. If loading fails for whatever reason, just return Resource_Loading_Status_Fail.
        //No need to call destroy() method, worker thread will already do that.
        virtual Resource_Loading_Status load(const std::string& filepath) = 0;

        //If engine detects that file changed, it will call this function.
        //Job of this function is to update data that needs to be updated from a new file (if any) and returns if everything is okay or now.
        virtual Resource_Loading_Status reload(const std::string& filepath) = 0;

        //Self-explanatory. It destroys object and all of its users (or at least notifies them before destruction).
        virtual void destroy() = 0;

        //This numbers determines the ordering inside waiting queue for loading stuff.
        //Higher priority objects will wait for lower ones to load up first.
        //If object does not wait for any other objects to load, it's safe to put 0 as priority (for example texture file can be priority 0).
        //Likewise, if object is expected to wait only for objects with priority 0 to load (for example model waiting only for textures to load) then this can be set to 1.
        //If object waits for model to load then it can be set to 2 and so on, building up the hierarchy of dependencies for loading stuff.
        //Screwing up this number may cause the deadlock to occur.
        virtual unsigned int priority() const = 0;

        virtual ~Resource_Base() = default;
    };

    //Use for read-only data like textures, shaders, geometry data etc.; data that does not have to wait for anything else to load.
    struct Resource_Basic : Resource_Base
    {
        unsigned int priority() const final { return 0;}
    };

    //If resource needs to wait only for other Resource_Basic to load (such when models wait for their textures to load) use this.
    struct Resource_Model : Resource_Base
    {
        unsigned int priority() const final { return 1;}
    };

    //This is (currently) the highest level for loading stuff. This resource will load up last after everything else is loaded. Use it for scenes or other global data.
    struct Resource_Scene : Resource_Base
    {
        unsigned int priority() const final { return 2;}
    };


}

namespace Smoothie::internal
{
    enum ResourceEntry_Status
    {
        ResourceEntry_Status_Default, //Freshly allocated object have this status.
        ResourceEntry_Status_Updating, //Thread is either loading or destroying underlying resource object.
        ResourceEntry_Status_Reading, //Some thread(s) are reading values.
        ResourceEntry_Status_Idle, //No reads or writes are currently happening to this object.
        ResourceEntry_Status_Error, //Error occurred with the object.
    };

    struct ResourceEntry
    {
        std::mutex mutex;
        std::condition_variable cv;
        std::atomic<ResourceEntry_Status> current_status{ResourceEntry_Status_Default};
        std::shared_ptr<Resource_Base> resource;
        unsigned int reader_count{0};
        std::filesystem::file_time_type write_time;
    };

    enum ResourceThread_JobType: int
    {
        ResourceThread_JobType_Load,
        ResourceThread_JobType_Reload,
        ResourceThread_JobType_Destroy,
    };

    enum ResourceThread_Status: int
    {
        ResourceThread_Status_Running,
        ResourceThread_Status_Idle,
    };

    struct ResourceThread
    {
        std::atomic<ResourceThread_Status> status{ResourceThread_Status_Idle};
        std::thread thread;
    };

    struct ResourceThread_Job
    {
        ResourceThread_JobType type;
        std::string filepath;
        std::shared_ptr<ResourceEntry> entry;
    };

    struct ResourceThread_Guard
    {
        explicit ResourceThread_Guard(std::atomic<ResourceThread_Status>& thread) : _status(thread){_status.store(ResourceThread_Status_Running);}
        ~ResourceThread_Guard() { _status.store(ResourceThread_Status_Idle); }
    private:
        std::atomic<ResourceThread_Status> &_status;
    };

    class ResourceThreadPool
    {
    public:
        explicit ResourceThreadPool(unsigned int thread_count) : m_Threads(thread_count), manager_running{true},
        m_ManagerThread(&ResourceThreadPool::manager_work, this){}

        void add_job(ResourceThread_JobType type, const std::string& filepath, std::shared_ptr<ResourceEntry> resource);
        ~ResourceThreadPool();

    private:
        std::vector<ResourceThread> m_Threads;
        std::thread m_ManagerThread;
        std::atomic<bool> manager_running;
        void manager_work();

        std::mutex m_Jobs_Mutex;
        std::queue<ResourceThread_Job> m_Jobs;
    };

}