#pragma once
#include "Common.h"

#include "Tools/Tool_Base.h"
#include "Tools/Tool_Camera.h"

namespace Smoothie::Editor
{
    class Core
    {
    public:
        virtual int create();
        virtual int resize_callback();

        //Does UI work.
        virtual void ui();
        virtual void compute_work(VkCommandBuffer command_buffer);
        virtual void draw_ui(VkCommandBuffer command_buffer);

        void set_ResultImage(VkImage image) {m_Result_Image = image;}
        void set_ResultImageView(VkImageView view) {m_Result_ImageView = view;}
        void set_DepthImage(VkImage image) {m_ResultDepth_Image = image;}
        void set_DepthImageView(VkImageView view) {m_ResultDepth_ImageView = view;}

        virtual void destroy();

        virtual ~Core() = default;
    protected:
        VkSampler m_Sampler = nullptr;

        VkImage m_Result_Image = nullptr;
        VkImageView m_Result_ImageView = nullptr;

        VkImage m_ResultDepth_Image = nullptr;
        VkImageView m_ResultDepth_ImageView = nullptr;

        VkDescriptorSet m_ResultImage_Set = nullptr;

        //**************************** Tools ****************************//
        std::shared_ptr<Tools_Data> m_Tools_Data = std::make_shared<Tools_Data>();


        std::unique_ptr<Tool_Camera> m_Tool_Camera = std::make_unique<Tool_Camera>(nullptr);

        //ImGui UI of tools. Override if you want to add your own tool.
        virtual void tools_work_ui();

    };
}